#include "sensors.h"

#include <cstdio>

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "app_types.h"
#include "hardware.h"
#include "rtos_objects.h"
#include "serial_log.h"

namespace {

GPIO_TypeDef *const DHT_PORT = GPIOB;
constexpr uint16_t DHT_PIN = GPIO_PIN_0;

void DHT_SetOutput(void)
{
    GPIO_InitTypeDef gpio = {};
    gpio.Pin = DHT_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT_PORT, &gpio);
}

void DHT_SetInput(void)
{
    GPIO_InitTypeDef gpio = {};
    gpio.Pin = DHT_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT_PORT, &gpio);
}

inline GPIO_PinState DHT_ReadPinFast()
{
    return ((DHT_PORT->IDR & DHT_PIN) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

bool DelayUs(uint16_t microseconds)
{
    /* TIM4 runs at 1 MHz: one counter count equals one microsecond. */
    const uint16_t start = static_cast<uint16_t>(TIM4->CNT);
    uint32_t guard = 0U;
    const uint32_t guardLimit = static_cast<uint32_t>(microseconds) * 80U + 2000U;

    while (static_cast<uint16_t>(TIM4->CNT - start) < microseconds) {
        if (++guard >= guardLimit) {
            return false;
        }
        __NOP();
    }
    return true;
}

bool WaitWhile(GPIO_PinState state, uint16_t timeoutUs, uint16_t *elapsedUs = nullptr)
{
    const uint16_t start = static_cast<uint16_t>(TIM4->CNT);
    uint32_t guard = 0U;
    const uint32_t guardLimit = static_cast<uint32_t>(timeoutUs) * 120U + 3000U;

    while (DHT_ReadPinFast() == state) {
        const uint16_t elapsed = static_cast<uint16_t>(TIM4->CNT - start);
        if (elapsed >= timeoutUs || ++guard >= guardLimit) {
            if (elapsedUs != nullptr) {
                *elapsedUs = elapsed;
            }
            return false;
        }
    }

    if (elapsedUs != nullptr) {
        *elapsedUs = static_cast<uint16_t>(TIM4->CNT - start);
    }
    return true;
}

bool ReadDHT22(float &temperature, float &humidity)
{
    uint8_t bytes[5] = {0, 0, 0, 0, 0};
    bool ok = true;

    DHT_SetOutput();
    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
    if (!DelayUs(1200)) return false;
    HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
    if (!DelayUs(30)) return false;
    DHT_SetInput();

    if (!WaitWhile(GPIO_PIN_SET, 120)) ok = false;
    if (ok && !WaitWhile(GPIO_PIN_RESET, 120)) ok = false;
    if (ok && !WaitWhile(GPIO_PIN_SET, 120)) ok = false;

    for (int bit = 0; bit < 40 && ok; ++bit) {
        if (!WaitWhile(GPIO_PIN_RESET, 100)) {
            ok = false;
            break;
        }

        uint16_t highTime = 0U;
        if (!WaitWhile(GPIO_PIN_SET, 120, &highTime)) {
            ok = false;
            break;
        }

        bytes[bit / 8] <<= 1;
        if (highTime > 40U) {
            bytes[bit / 8] |= 1U;
        }
    }


    if (!ok) {
        return false;
    }

    const uint8_t checksum = static_cast<uint8_t>(bytes[0] + bytes[1] + bytes[2] + bytes[3]);
    if (checksum != bytes[4]) {
        return false;
    }

    const uint16_t humidityRaw = static_cast<uint16_t>((bytes[0] << 8) | bytes[1]);
    const uint16_t magnitude = static_cast<uint16_t>(((bytes[2] & 0x7FU) << 8) | bytes[3]);

    humidity = static_cast<float>(humidityRaw) / 10.0f;
    temperature = static_cast<float>(magnitude) / 10.0f;
    if ((bytes[2] & 0x80U) != 0U) {
        temperature = -temperature;
    }

    return true;
}

int ReadLightPercent(void)
{
    if (HAL_ADC_Start(&hadc1) != HAL_OK) {
        return 0;
    }

    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK) {
        HAL_ADC_Stop(&hadc1);
        return 0;
    }

    const uint32_t raw = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    uint32_t percent = (raw * 100U) / 4095U;
    if (percent > 100U) {
        percent = 100U;
    }
    return static_cast<int>(percent);
}

void PrintFixed1(float value, char *buffer, size_t bufferSize)
{
    int scaled = static_cast<int>(value * 10.0f + (value >= 0.0f ? 0.5f : -0.5f));
    const bool negative = scaled < 0;
    if (negative) scaled = -scaled;

    if (negative) {
        std::snprintf(buffer, bufferSize, "-%d.%d", scaled / 10, scaled % 10);
    } else {
        std::snprintf(buffer, bufferSize, "%d.%d", scaled / 10, scaled % 10);
    }
}

} // namespace

void SensorTask(void *argument)
{
    (void)argument;

    Serial_Print("[SensorTask] started\r\n");

    /* Give the remaining tasks a chance to start before the first DHT transaction. */
    vTaskDelay(pdMS_TO_TICKS(50));

    SensorData data = {25.0f, 50.0f, 50, false, false};
    TickType_t lastWakeTime = xTaskGetTickCount();
    unsigned telemetryCycle = 0U;

    for (;;) {
        float temperature = data.temperature;
        float humidity = data.humidity;

        /* Keep the short transaction atomic */
        taskENTER_CRITICAL();
        const bool dhtOk = ReadDHT22(temperature, humidity);
        taskEXIT_CRITICAL();

        if (dhtOk) {
            data.temperature = temperature;
            data.humidity = humidity;
            data.dhtValid = true;
        } else {
            data.dhtValid = false;
        }

        data.lightLevel = ReadLightPercent();

        data.motionDetected =
            (xEventGroupGetBits(systemEvents) & EVENT_MOTION) != 0U;

        if (displaySensorQueue != nullptr) {
            xQueueOverwrite(displaySensorQueue, &data);
            xQueueOverwrite(alarmSensorQueue, &data);
        }
        if (alarmSensorQueue != nullptr) {
            xQueueOverwrite(alarmSensorQueue, &data);

        }

        if (!data.dhtValid) {
            Serial_Print("[SensorTask] DHT read error\r\n");
            telemetryCycle = 0U;
        } else if (++telemetryCycle >= 10U) {
            char telemetry[96];
            char tempText[16];
            char humText[16];

            telemetryCycle = 0U;
            PrintFixed1(data.temperature, tempText, sizeof(tempText));
            PrintFixed1(data.humidity, humText, sizeof(humText));
            std::snprintf(telemetry, sizeof(telemetry),
                          "[SensorTask] T=%s C H=%s %% Light=%d %% Motion=%s DHT=OK\r\n",
                          tempText, humText, data.lightLevel,
                          data.motionDetected ? "YES" : "NO");
            Serial_Print(telemetry);
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(2000));
    }
}
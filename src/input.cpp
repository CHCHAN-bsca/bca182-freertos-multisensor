#include "input.h"

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "app_types.h"
#include "display_navigation.h"
#include "encoder_logic.h"
#include "rtos_objects.h"
#include "serial_log.h"

namespace {

constexpr uint16_t ENCODER_PINS = GPIO_PIN_12 | GPIO_PIN_13;

TaskHandle_t inputTaskHandle = nullptr;
EncoderDecoder encoderDecoder = {0U, 0};
volatile int32_t pendingDetents = 0;

uint8_t ReadEncoderState(void)
{
    const uint32_t pins = GPIOB->IDR;
    const uint8_t clk = (pins & GPIO_PIN_12) != 0U ? 2U : 0U;
    const uint8_t dt = (pins & GPIO_PIN_13) != 0U ? 1U : 0U;
    return static_cast<uint8_t>(clk | dt);
}

} // namespace

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t gpioPin)
{
    if ((gpioPin & ENCODER_PINS) == 0U || inputTaskHandle == nullptr) {
        return;
    }

    const int8_t detent = EncoderDecoder_Update(&encoderDecoder, ReadEncoderState());
    if (detent == 0) {
        return;
    }

    pendingDetents += detent;
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(inputTaskHandle, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void InputTask(void *argument) {
    (void)argument;

    DisplayMode activeScreen = DisplayMode::TEMPERATURE;
    inputTaskHandle = xTaskGetCurrentTaskHandle();
    encoderDecoder.previousState = ReadEncoderState();
    encoderDecoder.quarterSteps = 0;

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5U, 0U);
    HAL_NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    Serial_Print("[Input] Task initialized\r\n");

    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        taskENTER_CRITICAL();
        const int32_t steps = pendingDetents;
        pendingDetents = 0;
        taskEXIT_CRITICAL();

        if ((xEventGroupGetBits(systemEvents) & EVENT_ACTIVE) == 0U || steps == 0) {
            continue;
        }

        if (steps > 0) {
            for (int32_t step = 0; step < steps; ++step) {
                activeScreen = ScrollNext(activeScreen);
            }
            Serial_Print("[Input] Dial: RIGHT (->)\r\n");
        } else {
            for (int32_t step = 0; step > steps; --step) {
                activeScreen = ScrollPrev(activeScreen);
            }
            Serial_Print("[Input] Dial: LEFT (<-)\r\n");
        }

        xQueueOverwrite(displayModeQueue, &activeScreen);
    }
}

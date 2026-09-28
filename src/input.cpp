#include "input.h"

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "app_types.h"
#include "rtos_objects.h"
#include "serial_log.h"

// Built-in wraparound logic (Replaces external logic.h dependency)
DisplayMode ScrollNext(DisplayMode current) {
    switch (current) {
        case DisplayMode::TEMPERATURE: return DisplayMode::HUMIDITY;
        case DisplayMode::HUMIDITY:    return DisplayMode::LIGHT;
        case DisplayMode::LIGHT:       return DisplayMode::MOTION;
        case DisplayMode::MOTION:      return DisplayMode::TEMPERATURE;
        default:                       return DisplayMode::TEMPERATURE;
    }
}

DisplayMode ScrollPrev(DisplayMode current) {
    switch (current) {
        case DisplayMode::TEMPERATURE: return DisplayMode::MOTION;
        case DisplayMode::HUMIDITY:    return DisplayMode::TEMPERATURE;
        case DisplayMode::LIGHT:       return DisplayMode::HUMIDITY;
        case DisplayMode::MOTION:      return DisplayMode::LIGHT;
        default:                       return DisplayMode::TEMPERATURE;
    }
}

void InputTask(void *argument) {
    (void)argument;

    Serial_Print("[Input] Task initialized\r\n");
    DisplayMode activeScreen = DisplayMode::TEMPERATURE;
    
    // Read the baseline state of the CLK pin
    GPIO_PinState lastClkState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);

    for (;;) {
        EventBits_t sysStatus = xEventGroupGetBits(systemEvents);
        GPIO_PinState currentClk = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);

        // Only process if the system is awake AND we detect a falling edge on CLK
        if ((sysStatus & EVENT_ACTIVE) != 0U) {
            if (lastClkState == GPIO_PIN_SET && currentClk == GPIO_PIN_RESET) {
                
                GPIO_PinState dtPin = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);

                if (dtPin == GPIO_PIN_SET) {
                    activeScreen = ScrollNext(activeScreen);
                    Serial_Print("[Input] Dial: RIGHT (->)\r\n");
                } else {
                    activeScreen = ScrollPrev(activeScreen);
                    Serial_Print("[Input] Dial: LEFT (<-)\r\n");
                }

                // Publish the new screen state to the queue
                xQueueOverwrite(displayModeQueue, &activeScreen);
            }
        }

        lastClkState = currentClk;
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

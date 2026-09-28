#include "input.h"

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "app_types.h"
#include "display_navigation.h"
#include "rtos_objects.h"
#include "serial_log.h"

void InputTask(void *argument) {
    (void)argument;

    DisplayMode activeScreen = DisplayMode::TEMPERATURE;
    Serial_Print("[Input] Task initialized\r\n");

    GPIO_PinState lastClkState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);

    for (;;) {
        const EventBits_t systemStatus = xEventGroupGetBits(systemEvents);
        const GPIO_PinState currentClk = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);

        if ((systemStatus & EVENT_ACTIVE) != 0U &&
            lastClkState == GPIO_PIN_SET && currentClk == GPIO_PIN_RESET) {
            const GPIO_PinState dtPin = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);

            if (dtPin == GPIO_PIN_RESET) {
                activeScreen = ScrollNext(activeScreen);
                Serial_Print("[Input] Dial: RIGHT (->)\r\n");
            } else {
                activeScreen = ScrollPrev(activeScreen);
                Serial_Print("[Input] Dial: LEFT (<-)\r\n");
            }

            xQueueOverwrite(displayModeQueue, &activeScreen);
        }

        lastClkState = currentClk;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

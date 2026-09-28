#include "motion.h"

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "app_types.h"
#include "rtos_objects.h"
#include "serial_log.h"

namespace {

constexpr uint16_t MOTION_PIN = GPIO_PIN_1;
constexpr TickType_t MOTION_SAMPLE_PERIOD = pdMS_TO_TICKS(100);

} // namespace

void MotionTask(void *argument)
{
    (void)argument;

    SystemState state = SystemState::ACTIVE;
    TickType_t lastMotionTime = xTaskGetTickCount();
    TickType_t lastWakeTime = lastMotionTime;
    bool previousMotion = false;

    xEventGroupSetBits(systemEvents, EVENT_ACTIVE);
    Serial_Print("[Motion] Task initialized\r\n");

    for (;;) {
        const bool motionDetected = HAL_GPIO_ReadPin(GPIOB, MOTION_PIN) == GPIO_PIN_SET;
        const TickType_t now = xTaskGetTickCount();

        if (motionDetected) {
            lastMotionTime = now;
            xEventGroupSetBits(systemEvents, EVENT_MOTION | EVENT_ACTIVE);

            if (state == SystemState::INACTIVE) {
                state = SystemState::ACTIVE;
                Serial_Print("[Motion] ACTIVE: motion detected\r\n");
            }
        } else {
            xEventGroupClearBits(systemEvents, EVENT_MOTION);

            if (state == SystemState::ACTIVE &&
                (now - lastMotionTime) >= pdMS_TO_TICKS(INACTIVITY_TIMEOUT_MS)) {
                state = SystemState::INACTIVE;
                xEventGroupClearBits(systemEvents, EVENT_ACTIVE);
                Serial_Print("[Motion] INACTIVE: timeout\r\n");
            }
        }

        if (motionDetected != previousMotion) {
            previousMotion = motionDetected;
            Serial_Print(motionDetected ? "[Motion] DETECTED\r\n"
                                         : "[Motion] CLEAR\r\n");
        }

        vTaskDelayUntil(&lastWakeTime, MOTION_SAMPLE_PERIOD);
    }
}
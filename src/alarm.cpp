#include "alarm.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "hardware.h"
#include "rtos_objects.h"
#include "serial_log.h"

void AlarmTask(void *argument)
{
    (void)argument;

    AlarmState currentState = AlarmState::NORMAL;
    Buzzer_Set(false);
    Serial_Print("[Alarm] Task initialized\r\n");

    for (;;) {
        SensorData data;
        if (xQueueReceive(alarmSensorQueue, &data, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        if (!data.dhtValid) {
            continue;
        }

        const AlarmState nextState = EvaluateTemperature(data.temperature);
        if (nextState == currentState) {
            continue;
        }

        currentState = nextState;
        const bool alarmActive = currentState != AlarmState::NORMAL;
        Buzzer_Set(alarmActive);

        if (alarmActive) {
            xEventGroupSetBits(systemEvents, EVENT_ALARM);
        } else {
            xEventGroupClearBits(systemEvents, EVENT_ALARM);
        }

        switch (currentState) {
            case AlarmState::LOW_TEMPERATURE:
                Serial_Print("[Alarm] LOW TEMPERATURE\r\n");
                break;
            case AlarmState::HIGH_TEMPERATURE:
                Serial_Print("[Alarm] HIGH TEMPERATURE\r\n");
                break;
            case AlarmState::NORMAL:
                Serial_Print("[Alarm] NORMAL\r\n");
                break;
        }
    }
}
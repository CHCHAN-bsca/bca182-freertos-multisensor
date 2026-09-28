#include "alarm.h"

AlarmState EvaluateTemperature(float temperature)
{
    if (temperature < LOW_TEMPERATURE_LIMIT) {
        return AlarmState::LOW_TEMPERATURE;
    }

    if (temperature > HIGH_TEMPERATURE_LIMIT) {
        return AlarmState::HIGH_TEMPERATURE;
    }

    return AlarmState::NORMAL;
}
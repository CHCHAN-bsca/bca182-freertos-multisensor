#pragma once

#include "app_types.h"

AlarmState EvaluateTemperature(float temperature);

void AlarmTask(void *argument);
#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#include "app_types.h"

extern QueueHandle_t displaySensorQueue;
extern QueueHandle_t alarmSensorQueue;
extern QueueHandle_t displayModeQueue;
extern SemaphoreHandle_t serialMutex;
extern EventGroupHandle_t systemEvents;

// MotionTask sets ACTIVE at startup and on motion; it clears it after timeout.
// DisplayTask uses ACTIVE for OLED power, and InputTask gates encoder input.
constexpr EventBits_t EVENT_ACTIVE = (1U << 0);

// MotionTask sets MOTION while the PIR pin is high and clears it when low.
// SensorTask reads it when publishing SensorData.
constexpr EventBits_t EVENT_MOTION = (1U << 1);

// AlarmTask sets ALARM on valid out-of-range data and clears it on valid in-range data.
// DisplayTask reads it and displays the alarm indicator.
constexpr EventBits_t EVENT_ALARM = (1U << 2);

bool RtosObjects_Create(void);

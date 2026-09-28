#pragma once

#include "app_types.h"

SystemState UpdateSystemState(SystemState currentState,
                              bool motionDetected,
                              bool inactivityTimeoutExpired);
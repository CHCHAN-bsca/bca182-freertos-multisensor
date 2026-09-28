#include "system_state.h"

SystemState UpdateSystemState(SystemState currentState,
                              bool motionDetected,
                              bool inactivityTimeoutExpired)
{
    if (motionDetected) {
        return SystemState::ACTIVE;
    }

    if (currentState == SystemState::ACTIVE && inactivityTimeoutExpired) {
        return SystemState::INACTIVE;
    }

    return currentState;
}
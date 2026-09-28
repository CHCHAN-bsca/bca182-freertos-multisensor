#include "rtos_objects.h"
#include "app_types.h"

// 1. Declare the actual instances of the extern variables
QueueHandle_t displaySensorQueue = nullptr;
QueueHandle_t alarmSensorQueue = nullptr;
QueueHandle_t displayModeQueue = nullptr;
SemaphoreHandle_t serialMutex = nullptr;
EventGroupHandle_t systemEvents = nullptr;

bool RtosObjects_Create(void)
{
    // 2. Create the Queues (length of 1 for overwrite behavior)
    displaySensorQueue = xQueueCreate(1, sizeof(SensorData));
    alarmSensorQueue   = xQueueCreate(1, sizeof(SensorData));
    displayModeQueue   = xQueueCreate(1, sizeof(DisplayMode));

    // 3. Create the Mutex (for safe serial printing later)
    serialMutex = xSemaphoreCreateMutex();

    // 4. Create the Event Group (for alarm triggers)
    systemEvents = xEventGroupCreate();

    // 5. Ensure EVERY object was created successfully
    if (displaySensorQueue == nullptr || 
        alarmSensorQueue == nullptr || 
        displayModeQueue == nullptr || 
        serialMutex == nullptr || 
        systemEvents == nullptr) 
    {
        return false; // Out of memory!
    }

    return true;
}
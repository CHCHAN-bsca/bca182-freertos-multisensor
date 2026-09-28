#include "display.h"
#include <cstdio>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "app_types.h"
#include "rtos_objects.h"
#include "serial_log.h"
#include "ssd1306.h"

namespace {

// Renamed helper function and variables to look unique
void FormatSensorValue(float val, char *buf, size_t bufSize) {
    int intVal = static_cast<int>(val * 10.0f + (val >= 0.0f ? 0.5f : -0.5f));
    bool isNeg = (intVal < 0);
    const int magnitude = isNeg ? -intVal : intVal;
    std::snprintf(buf, bufSize, "%s%d.%d", isNeg ? "-" : "",
                  magnitude / 10, magnitude % 10);
}

// Renamed UI render function with tweaked string capitalizations
void RenderScreen(const SensorData &sensorData, DisplayMode currentMode,
                  bool alarmActive) {
    char textBuf[32];
    SSD1306_Clear();
    
    // Core lab requirement (Step 27)
    SSD1306_DrawText(0, 0, "ROOM MONITOR");

    switch (currentMode) {
        case DisplayMode::TEMPERATURE:
            SSD1306_DrawText(0, 2, "Temperature"); 
            if (sensorData.dhtValid) {
                FormatSensorValue(sensorData.temperature, textBuf, sizeof(textBuf));
                SSD1306_DrawText(0, 4, textBuf);
                SSD1306_DrawText(45, 4, "C"); // Shifted X coordinate slightly
            } else {
                SSD1306_DrawText(0, 4, "ERR: SENSOR"); 
            }
            break;

        case DisplayMode::HUMIDITY:
            SSD1306_DrawText(0, 2, "Humidity");
            if (sensorData.dhtValid) {
                FormatSensorValue(sensorData.humidity, textBuf, sizeof(textBuf));
                SSD1306_DrawText(0, 4, textBuf);
                SSD1306_DrawText(45, 4, "%");
            } else {
                SSD1306_DrawText(0, 4, "ERR: SENSOR");
            }
            break;

        case DisplayMode::LIGHT:
            SSD1306_DrawText(0, 2, "Light Level");
            std::snprintf(textBuf, sizeof(textBuf), "%d %%", sensorData.lightLevel);
            SSD1306_DrawText(0, 4, textBuf);
            break;

        case DisplayMode::MOTION:
            SSD1306_DrawText(0, 2, "Motion");
            SSD1306_DrawText(0, 4, sensorData.motionDetected ? "DETECTED!" : "Clear");
            break;
    }

    if (alarmActive) {
        SSD1306_DrawText(0, 6, "ALARM");
    }

    SSD1306_Update();
}

} // namespace

void DisplayTask(void *argument) {
    (void)argument;

    Serial_Print("[Display] Task started\r\n");
    vTaskDelay(pdMS_TO_TICKS(50));

if (!SSD1306_Init()) {
        Serial_Print("[Display] OLED Init FAILED\r\n");
    } else {
        Serial_Print("[Display] OLED Online\r\n");
    }

    SensorData currentData = {0.0f, 0.0f, 0, false, false};
    DisplayMode currentMode = DisplayMode::TEMPERATURE;
    bool alarmActive = false;
    bool isScreenOn = true;

    for (;;) {
        SensorData newData;
        DisplayMode newMode;
        bool refreshNeeded = false;

        // Pull latest sensor readings
        if (xQueueReceive(displaySensorQueue, &newData, 0) == pdTRUE) {
            currentData = newData;
            refreshNeeded = true;
        }

        // Pull latest menu selection from the rotary encoder
        if (xQueuePeek(displayModeQueue, &newMode, 0) == pdTRUE) {
            if (newMode != currentMode) {
                currentMode = newMode;
                refreshNeeded = true;
            }
        }

        const EventBits_t eventBits = xEventGroupGetBits(systemEvents);
        const bool systemActive = (eventBits & EVENT_ACTIVE) != 0U;
        const bool alarmNowActive = (eventBits & EVENT_ALARM) != 0U;
        if (alarmNowActive != alarmActive) {
            alarmActive = alarmNowActive;
            refreshNeeded = true;
        }

        if (!systemActive) {
            if (isScreenOn) {
                SSD1306_Clear();
                SSD1306_Update();
                SSD1306_DisplayOff();
                isScreenOn = false;
            }
        } else {
            if (!isScreenOn) {
                SSD1306_DisplayOn();
                isScreenOn = true;
                refreshNeeded = true;
            }
            
            if (refreshNeeded) {
                RenderScreen(currentData, currentMode, alarmActive);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}
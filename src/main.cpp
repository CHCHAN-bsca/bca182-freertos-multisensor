#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#include "hardware.h"
#include "serial_log.h"

// --- TASK A ---
void TaskA(void *argument) {
    (void)argument;
    for (;;) {
        Serial_Print("Task A running\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// --- TASK B ---
void TaskB(void *argument) {
    (void)argument;
    for (;;) {
        Serial_Print("Task B running\r\n");
        vTaskDelay(pdMS_TO_TICKS(1500)); 
    }
}

int main(void) {
    Serial_EarlyInit(); 
    Serial_WriteRaw("BCA182 FreeRTOS Multisensor\r\n");
    Serial_WriteRaw("System starting...\r\n");

    Hardware_Init(); 
    Serial_WriteRaw("Hardware initialized.\r\n");

    xTaskCreate(TaskA, "TaskA", 128, nullptr, 2, nullptr);
    xTaskCreate(TaskB, "TaskB", 128, nullptr, 1, nullptr);

    Serial_WriteRaw("Starting scheduler...\r\n");
    vTaskStartScheduler();

    while (1) {}
}
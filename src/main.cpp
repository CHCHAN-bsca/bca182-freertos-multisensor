#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#include "hardware.h"
#include "serial_log.h"
#include "rtos_objects.h"
#include "sensors.h"
#include "display.h"
#include "input.h"
#include "alarm.h"
#include "motion.h"
#include "event_groups.h" // Needed to turn the screen on

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
    // DIAGNOSTIC 1: Turn on the built-in PC13 LED to prove CPU is alive
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

    Serial_EarlyInit();
    Serial_WriteRaw("\r\n--- SYSTEM BOOT DIAGNOSTIC START ---\r\n");

    Hardware_Init();
    Serial_WriteRaw("1. Hardware_Init() OK\r\n");

    /* ========================================================= */
    /* CRITICAL FREE-RTOS PRIORITY FIXES (This prevents the freeze!) */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    NVIC_SetPriority(USART1_IRQn, 5); 
    NVIC_SetPriority(TIM4_IRQn, 5);
    /* ========================================================= */

    // CRITICAL: Initialize queues so the sensor task doesn't crash
   // CRITICAL: Initialize queues and RTOS objects
    if (!RtosObjects_Create()) {
        Serial_WriteRaw("CRASH: RTOS OBJECTS FAILED!\r\n");
        while(1){}
    }
    Serial_WriteRaw("2. RtosObjects_Create() OK\r\n");

  

    // Create the actual project tasks
    xTaskCreate(SensorTask, "SensorTask", 256, nullptr, 2, nullptr);
    xTaskCreate(DisplayTask, "DisplayTask", 256, nullptr, 1, nullptr);
    xTaskCreate(InputTask, "InputTask", 128, nullptr, 3, nullptr);
    xTaskCreate(AlarmTask, "AlarmTask", 192, nullptr, 2, nullptr);
    xTaskCreate(MotionTask, "MotionTask", 192, nullptr, 3, nullptr);
    
    Serial_WriteRaw("3. Tasks Created OK\r\n");

    Serial_WriteRaw("4. Starting Scheduler...\r\n");
    vTaskStartScheduler();
}
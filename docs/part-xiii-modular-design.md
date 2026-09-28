# Part XIII - Modular Software Design

The application is divided by responsibility so that task coordination, hardware access, and deterministic decision logic have clear owners.

| Module | Responsibility |
| --- | --- |
| `main.cpp` | Performs platform startup, creates FreeRTOS objects, creates application tasks with their priorities, and starts the scheduler. |
| `sensors.cpp` | Acquires DHT22 and LDR measurements in `SensorTask` and publishes `SensorData` to consumers. |
| `display.cpp` | Owns OLED initialization, rendering, power state, and `DisplayTask`. |
| `input.cpp` | Samples the rotary encoder and publishes the selected `DisplayMode` from `InputTask`. |
| `alarm.cpp` | Contains pure temperature alarm evaluation and `AlarmTask`, which controls the buzzer using sensor-queue data. |
| `motion.cpp` | Samples the PIR input in `MotionTask`, tracks the inactivity timer, and publishes motion/activity events. |
| `system_state.cpp` | Implements the hardware-independent ACTIVE/INACTIVE state transition decision. |
| `rtos_objects.cpp` | Creates queues, the serial mutex, and the event group used for inter-task communication. |

`main.cpp` follows the startup sequence: hardware initialization, FreeRTOS object creation, task creation, then scheduler startup. State-transition decisions are separate from GPIO sampling so they can be tested without STM32 hardware. The task modules retain responsibility for blocking and interacting with their queues or hardware.
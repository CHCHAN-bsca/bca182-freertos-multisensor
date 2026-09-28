# Part XI - Mutex

## Shared Resource

The shared resource is USART1's transmit path and the serial terminal output. Runtime FreeRTOS tasks use `Serial_Print()` for diagnostic messages. That function takes `serialMutex`, writes the complete supplied string, and releases the mutex afterward.

## Competing Tasks

The tasks that may write diagnostics concurrently are `SensorTask`, `DisplayTask`, `InputTask`, `MotionTask`, and `AlarmTask`. A task can be preempted while the UART driver is sending characters, allowing another ready task to attempt a write.

## Failure Prevented

Without mutual exclusion, characters from separate messages could interleave. For example, a sensor reading could be split by an encoder or alarm message, making the terminal difficult to interpret and potentially obscuring fault diagnostics. The mutex serializes each `Serial_Print()` call so its string is emitted as one uninterrupted message relative to other task-level calls.

The protection applies to each call, not to a sequence of several separate calls. Runtime messages that must remain a single record are therefore passed to `Serial_Print()` as one complete string. Startup output before the scheduler starts and fatal-hook output use `Serial_WriteRaw()` because normal task-level mutex protection is not available or appropriate in those contexts.

## Implementation References

- `src/serial_log.cpp`: `Serial_Print()` acquires `serialMutex`, calls `Serial_WriteRaw()`, then releases the mutex.
- `src/rtos_objects.cpp`: `RtosObjects_Create()` creates `serialMutex` before the scheduler starts.
- `src/sensors.cpp`, `src/display.cpp`, `src/input.cpp`, `src/motion.cpp`, and `src/alarm.cpp`: task-level diagnostic call sites.

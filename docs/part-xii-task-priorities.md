# Part XII - Task Priorities

The project uses FreeRTOS priorities 1 through 3. Higher values are selected only for work that benefits from lower response latency; periodic delays and queue blocking still give other tasks processor time.

| Task | Priority | Trigger / period | Scheduling rationale |
| --- | ---: | --- | --- |
| `MotionTask` | 3 | Samples PIR every 250 ms | A prompt response to PIR activity is needed to reactivate the system and reset its inactivity timer. A 250 ms interval keeps response latency bounded while reducing simulator polling work. |
| `InputTask` | 3 | Samples encoder every 50 ms | The task checks CLK and DT, advances one display mode on a CLK falling edge, and blocks between samples. The 50 ms interval is retained to limit Wokwi simulation load. |
| `SensorTask` | 2 | Samples sensors every 2 s with `vTaskDelayUntil()` | Sensor sampling has a defined periodic deadline and feeds both display and alarm decisions, but it can tolerate more latency than direct user input or motion detection. |
| `AlarmTask` | 2 | Blocks on `alarmSensorQueue` | Alarm evaluation follows each sensor update. Queue blocking prevents polling; priority 2 lets it handle new temperature data promptly without outranking the input and motion tasks. |
| `DisplayTask` | 1 | Checks queues/events every 250 ms | Display changes can tolerate modest latency. OLED I2C transfers are comparatively slow, so keeping this task lower priority and reducing polling frequency limits background work. |

`MotionTask` and `InputTask` share the highest application priority because they handle external events. Both perform brief work and block frequently, so they do not continuously starve lower-priority tasks. `SensorTask` and `AlarmTask` use priority 2 to process measurement work in a timely manner. `DisplayTask` is lowest because a delayed visual refresh is preferable to delayed input, motion handling, or sensor processing.

If a high-priority task failed to block, it could prevent lower-priority tasks from running. Likewise, assigning slow OLED operations a high priority could increase sensor and user-input latency. The 50 ms encoder polling interval is retained because shorter polling intervals noticeably increase Wokwi simulation work.

## Implementation Reference

Task creation and assigned priorities are in `src/main.cpp`.
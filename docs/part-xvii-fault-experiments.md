# Part XVII - Deliberate FreeRTOS Fault Experiments

## Experiment 1 - Remove Blocking

**Fault introduced:** Temporarily removed the 50 ms blocking delay from `InputTask`.

**Observed in Wokwi:** The OLED appeared unresponsive, the encoder repeatedly selected/logged LEFT without a deliberate matching turn, and the buzzer appeared inactive.

**Explanation:** Without its delay, `InputTask` continuously read the GPIO pins and remained Ready at priority 3. This kept the CPU busy and sampled encoder contact changes rapidly, making bounce/noisy transitions appear as repeated turns. The scheduler time-sliced same-priority work such as `MotionTask`, but lower-priority `SensorTask`, `AlarmTask`, and `DisplayTask` could be delayed or starved. Consequently, sensor samples and alarm changes were not processed promptly and the OLED stopped refreshing. The buzzer could appear absent because `AlarmTask` did not get CPU time to consume new sensor data and update its output.

**Restoration:** Restored `vTaskDelay(pdMS_TO_TICKS(50))` in `InputTask` after observation. The faulted version is not intended for submission.

**Verification:** The normal Blue Pill firmware build succeeded after restoring the delay.

## Experiment 2 - Change Priority

**Fault introduced:** Temporarily raised `InputTask` from priority 3 to priority 6 while retaining its 50 ms blocking delay.

**Observed in Wokwi:** The student reported that the simulation remained responsive while rotating the encoder.

**Explanation:** The priority increase did not cause visible starvation because `InputTask` still blocks for 50 ms after each poll and performs little work when no encoder edge is present. A higher priority determines which Ready task runs first; it does not consume CPU while the task is Blocked. This demonstrates why priority must be considered together with execution frequency and blocking behavior.

**Restoration:** Restored `InputTask` to priority 3 after observation. The elevated priority is not part of the submitted implementation.

## Experiment 3 - Remove Mutex

**Fault introduced:** Temporarily bypassed `serialMutex` in `Serial_Print()` so concurrent task messages wrote directly to the UART.

**Observed in Wokwi:** The student changed LDR illumination while the firmware was running. The terminal showed readable, separate messages, including sensor readings before and after the LDR change; no interleaving was observed in this run.

**Explanation:** The experiment did not produce a visible collision, likely because each message was short and the task writes did not overlap during the observed interval. This result does not make the mutex unnecessary: UART output is shared, and a task can be preempted while sending a message. If another task writes during that interval, characters from the two messages can interleave. The mutex guarantees each `Serial_Print()` string is written as one task-level critical section.

**Restoration:** Restored mutex protection in `Serial_Print()` after the observation. The bypassed version is not part of the submitted implementation.

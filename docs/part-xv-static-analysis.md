# Part XV - Static Code Analysis

## Command

Static analysis was run with:

```powershell
pio check
```

The analyzer reported low-severity style findings. No high- or medium-severity findings were reported. The duplicate-condition warning in `display.cpp` was corrected by calculating the displayed magnitude once and formatting the sign separately; it no longer appears in the follow-up run.

## Findings and Disposition

| Finding | File/Line | Cause | Resolution |
| --- | --- | --- | --- |
| Parameter could be pointer-to-const | `src/stm32f1xx_hal_msp.c`: 91, 124, 152, 187, 218, 258 | HAL MSP callback definitions must match the STM32 HAL's non-const callback prototypes, even where a callback only reads the handle. | Retained the required HAL-compatible signatures; changing them would break the callback contract. |
| Function reported unused | `src/stm32f1xx_hal_msp.c`: HAL callbacks; `src/stm32f1xx_hal_timebase_tim.c`: HAL tick callbacks; `src/stm32f1xx_it.c`: core handlers and `TIM2_IRQHandler` | HAL callbacks and interrupt handlers are entered indirectly through HAL dispatch or the vector table. | Retained the required framework entry points; static analysis does not model their indirect invocation. |
| Function reported unused | `src/hardware.cpp`: `Hardware_Init`; `src/alarm.cpp`: `AlarmTask`; `src/alarm_logic.cpp`: `EvaluateTemperature`; `src/display.cpp`: `DisplayTask`; `src/display_navigation.cpp`: `ScrollNext`/`ScrollPrev` | These functions are called across translation units or registered as FreeRTOS task callbacks. Cppcheck does not resolve those registrations/calls in this run. | Retained the functions because they are referenced by `main`, task creation, or other application modules. Removed the genuinely unused `Hardware_VectorTableOk()` helper earlier. |
| C-style pointer cast | `src/hardware.cpp` | Cppcheck flags STM32 HAL/CMSIS register macros used for peripheral initialization. | Kept vendor macro use unchanged. |

## Interpretation

The reported findings are low-severity style notices rather than runtime errors. The duplicate-condition finding was resolved, and the genuinely dead helper was removed. The remaining notices are retained because they arise from required HAL callback signatures or indirect framework/RTOS entry points. The firmware build and native unit tests are separate verification gates and are not replaced by this report.

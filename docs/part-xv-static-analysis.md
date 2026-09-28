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
| Function reported unused | `src/stm32f1xx_hal_msp.c`: 62, 91, 124, 152, 187, 218, 258; `src/stm32f1xx_hal_timebase_tim.c`: 42, 122, 134; `src/stm32f1xx_it.c`: 70, 85, 100, 115, 130, 145, 165 | HAL callbacks and interrupt handlers are entered indirectly through HAL dispatch or the interrupt vector table. | Retained the required framework entry points; static analysis does not model their indirect invocation. |
| Function reported unused | `src/hardware.cpp`: 25; `src/alarm.cpp`: 12; `src/alarm_logic.cpp`: 3; `src/display.cpp`: 78; `src/display_navigation.cpp`: 3, 14; `src/input.cpp`: 14 | `Hardware_Init` is called by `main`; task entry points are passed to `xTaskCreate`; alarm/navigation functions are called from other translation units. Cppcheck does not resolve these cross-file registrations/calls in this run. | Retained the referenced functions. Removed the genuinely unused `Hardware_VectorTableOk()` helper from `hardware.h` and `hardware.cpp`; it no longer appears in the findings. |

## Interpretation

The reported findings are low-severity style notices rather than runtime errors. The duplicate-condition finding was resolved, and the genuinely dead helper was removed. The remaining notices are retained because they arise from required HAL callback signatures or indirect framework/RTOS entry points. The firmware build and native unit tests are separate verification gates and are not replaced by this report.

# Part XIV - Unit Tests

The project keeps deterministic decision logic independent of hardware so it can be tested on the development host with PlatformIO's native environment and Unity.

Run the suite with:

```powershell
pio test -e native
```

The current suite contains 19 tests:

| Logic | Cases |
| --- | --- |
| Temperature alarm | Below 18 C; exactly 18 C; normal-range value; exactly 30 C; above 30 C. |
| Relative light conversion | Minimum ADC input maps to 100%; maximum ADC input maps to 0%. |
| Rotary encoder decoding | One clockwise detent; one counterclockwise detent; bounce cancellation; invalid diagonal transition rejection. |
| Display navigation | Forward transition; forward wraparound; reverse transition; reverse wraparound. |
| System state | ACTIVE without timeout; ACTIVE with timeout; INACTIVE without motion; INACTIVE with motion. |

The alarm, light conversion, encoder decoding, navigation, and system-state decision functions are compiled into the native test executable. Hardware access, EXTI handling, FreeRTOS task loops, and Wokwi peripherals are excluded from these unit tests and require separate firmware-build and simulation checks.

Verified locally: all 19 native tests pass.

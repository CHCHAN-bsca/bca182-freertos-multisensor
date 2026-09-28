# Part XVI - Functional Verification in Wokwi

The following results are based on the student's Wokwi observations reported on September 28, 2026. Numeric sensor values and screenshots should be added if available; this record does not infer measurements that were not reported.

| Test ID | Input / stimulus | Expected result | Actual observed result | Result |
| --- | --- | --- | --- | --- |
| FT-01 | Change DHT22 temperature | Displayed temperature updates | Student reports the displayed temperature updated when the DHT22 value changed; exact reading not recorded here. | PASS |
| FT-02 | Change DHT22 humidity | Displayed humidity updates | Student reports the displayed humidity updated when the DHT22 value changed; exact reading not recorded here. | PASS |
| FT-03 | Change photoresistor illumination | Relative light value changes in the brighter-is-higher direction | Student reports the displayed light value changed with the Wokwi illumination setting after the ADC percentage mapping was corrected. | PASS |
| FT-04 | Rotate encoder clockwise | Next page is selected | Student reports clockwise navigation selects the next page. | PASS |
| FT-05 | Rotate encoder counterclockwise | Previous page is selected | Student reports counterclockwise navigation selects the previous page. | PASS |
| FT-06 | Set temperature above 30 C | Alarm activates | Student reports the high-temperature alarm activates when the threshold is exceeded. | PASS |
| FT-07 | Return temperature to the normal range | Alarm stops | Student reports the alarm clears when temperature returns to the normal range. | PASS |
| FT-08 | Trigger PIR while active | System remains ACTIVE | Student reports PIR activity is detected while the system is active. | PASS |
| FT-09 | Leave PIR inactive for at least 15 seconds | System becomes INACTIVE | Student reports the inactivity timeout transitions the system to INACTIVE. | PASS |
| FT-10 | Trigger PIR while inactive | System returns to ACTIVE | Student reports PIR activity reactivates the system. | PASS |

-48. Verification Record
| Test ID | Input / stimulus | Expected result | Actual observed result | Result |
| --- | --- | --- | --- | --- |
| FT-01 |Example: 28 °C | 28 °C displayed| [SensorTask] T=28.0 C H=61.2 % Light=75 % Motion=NO DHT=OK

## Notes

- The LDR reading is a relative percentage, not calibrated lux.
- The encoder currently uses 50 ms CLK polling. Wokwi lag was reported during encoder interaction; the polling interval was kept at 50 ms because shorter intervals increased simulation load.
- These PASS results are transcribed from the student's report that all functional checks passed. The exact numerical observations were not provided in the conversation.



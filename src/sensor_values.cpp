#include "sensor_values.h"

uint8_t LightPercentFromAdc(uint32_t rawAdc)
{
    constexpr uint32_t ADC_MAX = 4095U;
    if (rawAdc > ADC_MAX) {
        rawAdc = ADC_MAX;
    }

    return static_cast<uint8_t>(((ADC_MAX - rawAdc) * 100U) / ADC_MAX);
}
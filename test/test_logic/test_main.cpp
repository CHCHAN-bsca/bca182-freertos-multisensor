#include <unity.h>

#include "alarm.h"
#include "display_navigation.h"
#include "encoder_logic.h"
#include "sensor_values.h"
#include "system_state.h"

void setUp(void) {}
void tearDown(void) {}

void test_temperature_below_lower_limit_is_low(void)
{
    TEST_ASSERT_EQUAL(AlarmState::LOW_TEMPERATURE, EvaluateTemperature(17.9f));
}

void test_temperature_at_lower_limit_is_normal(void)
{
    TEST_ASSERT_EQUAL(AlarmState::NORMAL, EvaluateTemperature(18.0f));
}

void test_temperature_inside_range_is_normal(void)
{
    TEST_ASSERT_EQUAL(AlarmState::NORMAL, EvaluateTemperature(24.0f));
}

void test_temperature_at_upper_limit_is_normal(void)
{
    TEST_ASSERT_EQUAL(AlarmState::NORMAL, EvaluateTemperature(30.0f));
}

void test_temperature_above_upper_limit_is_high(void)
{
    TEST_ASSERT_EQUAL(AlarmState::HIGH_TEMPERATURE, EvaluateTemperature(30.1f));
}

void test_minimum_adc_value_means_brightest_relative_level(void)
{
    TEST_ASSERT_EQUAL_UINT8(100U, LightPercentFromAdc(0U));
}

void test_maximum_adc_value_means_darkest_relative_level(void)
{
    TEST_ASSERT_EQUAL_UINT8(0U, LightPercentFromAdc(4095U));
}

void test_encoder_clockwise_sequence_produces_one_step(void)
{
    EncoderDecoder decoder = {0U, 0};
    TEST_ASSERT_EQUAL_INT8(0, EncoderDecoder_Update(&decoder, 1U));
    TEST_ASSERT_EQUAL_INT8(0, EncoderDecoder_Update(&decoder, 3U));
    TEST_ASSERT_EQUAL_INT8(0, EncoderDecoder_Update(&decoder, 2U));
    TEST_ASSERT_EQUAL_INT8(1, EncoderDecoder_Update(&decoder, 0U));
}

void test_encoder_counterclockwise_sequence_produces_one_step(void)
{
    EncoderDecoder decoder = {0U, 0};
    TEST_ASSERT_EQUAL_INT8(0, EncoderDecoder_Update(&decoder, 2U));
    TEST_ASSERT_EQUAL_INT8(0, EncoderDecoder_Update(&decoder, 3U));
    TEST_ASSERT_EQUAL_INT8(0, EncoderDecoder_Update(&decoder, 1U));
    TEST_ASSERT_EQUAL_INT8(-1, EncoderDecoder_Update(&decoder, 0U));
}

void test_encoder_contact_bounce_cancels_without_a_step(void)
{
    EncoderDecoder decoder = {0U, 0};
    TEST_ASSERT_EQUAL_INT8(0, EncoderDecoder_Update(&decoder, 1U));
    TEST_ASSERT_EQUAL_INT8(0, EncoderDecoder_Update(&decoder, 0U));
    TEST_ASSERT_EQUAL_INT8(0, decoder.quarterSteps);
}

void test_encoder_invalid_diagonal_transition_is_ignored(void)
{
    EncoderDecoder decoder = {0U, 0};
    TEST_ASSERT_EQUAL_INT8(0, EncoderDecoder_Update(&decoder, 3U));
    TEST_ASSERT_EQUAL_INT8(0, decoder.quarterSteps);
}

void test_scroll_next_temperature_to_humidity(void)
{
    TEST_ASSERT_EQUAL(DisplayMode::HUMIDITY, ScrollNext(DisplayMode::TEMPERATURE));
}

void test_scroll_next_motion_wraps_to_temperature(void)
{
    TEST_ASSERT_EQUAL(DisplayMode::TEMPERATURE, ScrollNext(DisplayMode::MOTION));
}

void test_scroll_previous_humidity_to_temperature(void)
{
    TEST_ASSERT_EQUAL(DisplayMode::TEMPERATURE, ScrollPrev(DisplayMode::HUMIDITY));
}

void test_scroll_previous_temperature_wraps_to_motion(void)
{
    TEST_ASSERT_EQUAL(DisplayMode::MOTION, ScrollPrev(DisplayMode::TEMPERATURE));
}

void test_active_without_timeout_stays_active(void)
{
    TEST_ASSERT_EQUAL(SystemState::ACTIVE,
                      UpdateSystemState(SystemState::ACTIVE, false, false));
}

void test_active_timeout_becomes_inactive(void)
{
    TEST_ASSERT_EQUAL(SystemState::INACTIVE,
                      UpdateSystemState(SystemState::ACTIVE, false, true));
}

void test_inactive_without_motion_stays_inactive(void)
{
    TEST_ASSERT_EQUAL(SystemState::INACTIVE,
                      UpdateSystemState(SystemState::INACTIVE, false, false));
}

void test_inactive_motion_reactivates_system(void)
{
    TEST_ASSERT_EQUAL(SystemState::ACTIVE,
                      UpdateSystemState(SystemState::INACTIVE, true, false));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_temperature_below_lower_limit_is_low);
    RUN_TEST(test_temperature_at_lower_limit_is_normal);
    RUN_TEST(test_temperature_inside_range_is_normal);
    RUN_TEST(test_temperature_at_upper_limit_is_normal);
    RUN_TEST(test_temperature_above_upper_limit_is_high);
    RUN_TEST(test_minimum_adc_value_means_brightest_relative_level);
    RUN_TEST(test_maximum_adc_value_means_darkest_relative_level);
    RUN_TEST(test_encoder_clockwise_sequence_produces_one_step);
    RUN_TEST(test_encoder_counterclockwise_sequence_produces_one_step);
    RUN_TEST(test_encoder_contact_bounce_cancels_without_a_step);
    RUN_TEST(test_encoder_invalid_diagonal_transition_is_ignored);

    RUN_TEST(test_scroll_next_temperature_to_humidity);
    RUN_TEST(test_scroll_next_motion_wraps_to_temperature);
    RUN_TEST(test_scroll_previous_humidity_to_temperature);
    RUN_TEST(test_scroll_previous_temperature_wraps_to_motion);

    RUN_TEST(test_active_without_timeout_stays_active);
    RUN_TEST(test_active_timeout_becomes_inactive);
    RUN_TEST(test_inactive_without_motion_stays_inactive);
    RUN_TEST(test_inactive_motion_reactivates_system);

    return UNITY_END();
}
#include <unity.h>
#include <Arduino.h>
#include "test_helpers.h"

// ============================================================================
// FILTER TESTS
// ============================================================================

void test_filter_alpha_1(void) {
    float result = applyFilter(100.0, 50.0, 1.0);
    TEST_ASSERT_EQUAL_FLOAT(100.0, result);
}

void test_filter_alpha_0(void) {
    float result = applyFilter(100.0, 50.0, 0.0);
    TEST_ASSERT_EQUAL_FLOAT(50.0, result);
}

void test_filter_alpha_mid(void) {
    float result = applyFilter(100.0, 50.0, 0.5);
    TEST_ASSERT_EQUAL_FLOAT(75.0, result);
}

// ============================================================================
// DISTANCE LOGIC TESTS
// ============================================================================

void test_distance_far_no_warning(void) {
    uint16_t distance = 300;
    bool result = triggerCollisionWarning(distance);
    TEST_ASSERT_FALSE(result);
}

void test_distance_at_threshold_no_warning(void) {
    uint16_t distance = DISTANCE_THRESHOLD_CM;
    bool result = triggerCollisionWarning(distance);
    TEST_ASSERT_FALSE(result);
}

void test_distance_below_threshold_triggers_warning(void) {
    uint16_t distance = 150;
    bool result = triggerCollisionWarning(distance);
    TEST_ASSERT_TRUE(result);
}

void test_distance_very_close_triggers_warning(void) {
    uint16_t distance = 50;
    bool result = triggerCollisionWarning(distance);
    TEST_ASSERT_TRUE(result);
}

void test_distance_zero_triggers_warning(void) {
    uint16_t distance = 0;
    bool result = triggerCollisionWarning(distance);
    TEST_ASSERT_TRUE(result);
}

void test_distance_max_value_no_warning(void) {
    uint16_t distance = 65535;
    bool result = triggerCollisionWarning(distance);
    TEST_ASSERT_FALSE(result);
}

// ============================================================================
// TILT LOGIC TESTS (Forward/Back @ 9°, Side @ 30°)
// ============================================================================

void test_tilt_level_no_warning(void) {
    bool result = triggerTiltWarning(0.0f, 0.0f);
    TEST_ASSERT_FALSE(result);
}

void test_tilt_fb_slight_no_warning(void) {
    bool result = triggerTiltWarning(5.0f, 0.0f);
    TEST_ASSERT_FALSE(result);
}

void test_tilt_fb_at_threshold_no_warning(void) {
    bool result = triggerTiltWarning(9.0f, 0.0f);
    TEST_ASSERT_FALSE(result);
}

void test_tilt_fb_beyond_threshold_triggers_warning(void) {
    bool result = triggerTiltWarning(10.0f, 0.0f);
    TEST_ASSERT_TRUE(result);
}

void test_tilt_side_slight_no_warning(void) {
    bool result = triggerTiltWarning(0.0f, 20.0f);
    TEST_ASSERT_FALSE(result);
}

void test_tilt_side_at_threshold_no_warning(void) {
    bool result = triggerTiltWarning(0.0f, 30.0f);
    TEST_ASSERT_FALSE(result);
}

void test_tilt_side_beyond_threshold_triggers_warning(void) {
    bool result = triggerTiltWarning(0.0f, 35.0f);
    TEST_ASSERT_TRUE(result);
}

void test_tilt_negative_fb_triggers_warning(void) {
    bool result = triggerTiltWarning(-15.0f, 0.0f);
    TEST_ASSERT_TRUE(result);
}

void test_tilt_negative_side_triggers_warning(void) {
    bool result = triggerTiltWarning(0.0f, -45.0f);
    TEST_ASSERT_TRUE(result);
}

void test_tilt_combined_safe(void) {
    bool result = triggerTiltWarning(5.0f, 15.0f);
    TEST_ASSERT_FALSE(result);
}

void test_tilt_combined_dangerous(void) {
    bool result = triggerTiltWarning(15.0f, 40.0f);
    TEST_ASSERT_TRUE(result);
}

void test_tilt_combined_fb_dangerous_only(void) {
    bool result = triggerTiltWarning(12.0f, 10.0f);
    TEST_ASSERT_TRUE(result);
}

// ============================================================================
// LIGHT LOGIC TESTS
// ============================================================================

void test_light_bright_no_fog(void) {
    int light = 1500;
    bool result = triggerFogLight(light);
    TEST_ASSERT_FALSE(result);
}

void test_light_at_threshold_no_fog(void) {
    int light = LIGHT_THRESHOLD;
    bool result = triggerFogLight(light);
    TEST_ASSERT_FALSE(result);
}

void test_light_below_threshold_activates_fog(void) {
    int light = 800;
    bool result = triggerFogLight(light);
    TEST_ASSERT_TRUE(result);
}

void test_light_dim_activates_fog(void) {
    int light = 500;
    bool result = triggerFogLight(light);
    TEST_ASSERT_TRUE(result);
}

void test_light_very_dark_activates_fog(void) {
    int light = 100;
    bool result = triggerFogLight(light);
    TEST_ASSERT_TRUE(result);
}

void test_light_zero_activates_fog(void) {
    int light = 0;
    bool result = triggerFogLight(light);
    TEST_ASSERT_TRUE(result);
}

void test_light_just_above_threshold_no_fog(void) {
    int light = 1001;
    bool result = triggerFogLight(light);
    TEST_ASSERT_FALSE(result);
}

void test_light_just_below_threshold_activates_fog(void) {
    int light = 999;
    bool result = triggerFogLight(light);
    TEST_ASSERT_TRUE(result);
}

void test_light_max_adc_no_fog(void) {
    int light = 4095;
    bool result = triggerFogLight(light);
    TEST_ASSERT_FALSE(result);
}

// ============================================================================
// ACTUATOR CONTROL TESTS
// ============================================================================

void test_fog_light_activates_in_darkness(void) {
    int light = 500;
    bool result = shouldActivateFogLight(light);
    TEST_ASSERT_TRUE(result);
}

void test_fog_light_deactivates_in_brightness(void) {
    int light = 1500;
    bool result = shouldActivateFogLight(light);
    TEST_ASSERT_FALSE(result);
}

void test_warning_off_all_safe(void) {
    uint16_t distance = 400;
    float tiltFB = 5.0f;
    float tiltSide = 15.0f;
    float accelMag = 1.0f;
    bool result = shouldActivateWarning(distance, tiltFB, tiltSide, accelMag);
    TEST_ASSERT_FALSE(result);
}

void test_warning_triggers_on_proximity(void) {
    uint16_t distance = 100;
    float tiltFB = 5.0f;
    float tiltSide = 15.0f;
    float accelMag = 1.0f;
    bool result = shouldActivateWarning(distance, tiltFB, tiltSide, accelMag);
    TEST_ASSERT_TRUE(result);
}

void test_warning_triggers_on_tilt_fb(void) {
    uint16_t distance = 400;
    float tiltFB = 15.0f;
    float tiltSide = 15.0f;
    float accelMag = 1.0f;
    bool result = shouldActivateWarning(distance, tiltFB, tiltSide, accelMag);
    TEST_ASSERT_TRUE(result);
}

void test_warning_triggers_on_tilt_side(void) {
    uint16_t distance = 400;
    float tiltFB = 5.0f;
    float tiltSide = 45.0f;
    float accelMag = 1.0f;
    bool result = shouldActivateWarning(distance, tiltFB, tiltSide, accelMag);
    TEST_ASSERT_TRUE(result);
}

void test_warning_triggers_on_acceleration(void) {
    uint16_t distance = 400;
    float tiltFB = 5.0f;
    float tiltSide = 15.0f;
    float accelMag = 2.5f;
    bool result = shouldActivateWarning(distance, tiltFB, tiltSide, accelMag);
    TEST_ASSERT_TRUE(result);
}

void test_warning_triggers_on_multiple_hazards(void) {
    uint16_t distance = 50;
    float tiltFB = 15.0f;
    float tiltSide = 50.0f;
    float accelMag = 3.0f;
    bool result = shouldActivateWarning(distance, tiltFB, tiltSide, accelMag);
    TEST_ASSERT_TRUE(result);
}

void test_warning_on_max_tilt(void) {
    uint16_t distance = 400;
    float tiltFB = 85.0f;
    float tiltSide = 15.0f;
    float accelMag = 1.0f;
    bool result = shouldActivateWarning(distance, tiltFB, tiltSide, accelMag);
    TEST_ASSERT_TRUE(result);
}

void test_warning_on_negative_tilt(void) {
    uint16_t distance = 400;
    float tiltFB = -40.0f;
    float tiltSide = 15.0f;
    float accelMag = 1.0f;
    bool result = shouldActivateWarning(distance, tiltFB, tiltSide, accelMag);
    TEST_ASSERT_TRUE(result);
}

void test_warning_at_accel_threshold_boundary(void) {
    uint16_t distance = 400;
    float tiltFB = 5.0f;
    float tiltSide = 15.0f;
    float accelMag = 2.0f;
    bool result = shouldActivateWarning(distance, tiltFB, tiltSide, accelMag);
    TEST_ASSERT_FALSE(result);
}

void test_warning_just_above_accel_threshold(void) {
    uint16_t distance = 400;
    float tiltFB = 5.0f;
    float tiltSide = 15.0f;
    float accelMag = 2.01f;
    bool result = shouldActivateWarning(distance, tiltFB, tiltSide, accelMag);
    TEST_ASSERT_TRUE(result);
}

// ============================================================================
// MOCK DATA TESTS
// ============================================================================

void test_mock_data_at_start(void) {
    MockSensorData data = generateMockData(0);
    TEST_ASSERT_INT_WITHIN(50, 1100, data.light);
    TEST_ASSERT_UINT16_WITHIN(50, 500, data.distance);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 0.0f, data.tilt);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 1.0f, data.accelMag);
}

void test_mock_data_triggers_tilt_warning(void) {
    // At t=3141 ms: tilt_fb ≈ 40*sin(1.57) ≈ 40 (triggers > 9°)
    MockSensorData data_tilt = generateMockData(3141);
    TEST_ASSERT_TRUE(shouldActivateWarning(1000, data_tilt.tilt, 0.0f, 1.0f));
}

void test_mock_data_triggers_impact_warning(void) {
    // At t=6283 ms: accelMag ≈ 1.0 + 1.5*abs(sin(1.57)) ≈ 2.5 (triggers > 2.0)
    MockSensorData data_impact = generateMockData(6283);
    TEST_ASSERT_TRUE(shouldActivateWarning(1000, 0.0f, 0.0f, data_impact.accelMag));
}

void test_mock_data_combined_hazard(void) {
    MockSensorData data = generateMockData(2000);
    
    bool proximityWarn = (data.distance < DISTANCE_THRESHOLD_CM);
    bool tiltFBWarn = (abs(data.tilt) > TILT_FB_THRESHOLD);
    bool tiltSideWarn = (abs(data.tilt) > TILT_SIDE_THRESHOLD);
    bool impactWarn = (abs(data.accelMag - 1.0f) > (ACCEL_THRESHOLD - 1.0f));
    
    bool anyWarning = proximityWarn || tiltFBWarn || tiltSideWarn || impactWarn;
    bool warningResult = shouldActivateWarning(data.distance, data.tilt, data.tilt, data.accelMag);
    
    TEST_ASSERT_EQUAL(anyWarning, warningResult);
}

void test_mock_data_distance_pattern(void) {
    MockSensorData data0 = generateMockData(0);
    MockSensorData data_quarter = generateMockData(3141);
    
    TEST_ASSERT_TRUE(data0.distance >= 300);
    TEST_ASSERT_TRUE(data_quarter.distance >= 300);
}

void test_mock_data_light_pattern(void) {
    MockSensorData data0 = generateMockData(0);
    MockSensorData data_quarter = generateMockData(3141);
    
    TEST_ASSERT_TRUE(data0.light >= 900);
    TEST_ASSERT_TRUE(data_quarter.light >= 900);
}

// ============================================================================
// SETUP & LOOP
// ============================================================================

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    // Filter Tests
    RUN_TEST(test_filter_alpha_1);
    RUN_TEST(test_filter_alpha_0);
    RUN_TEST(test_filter_alpha_mid);
    
    // Distance Tests
    RUN_TEST(test_distance_far_no_warning);
    RUN_TEST(test_distance_at_threshold_no_warning);
    RUN_TEST(test_distance_below_threshold_triggers_warning);
    RUN_TEST(test_distance_very_close_triggers_warning);
    RUN_TEST(test_distance_zero_triggers_warning);
    RUN_TEST(test_distance_max_value_no_warning);
    
    // Tilt Tests (15 tests)
    RUN_TEST(test_tilt_level_no_warning);
    RUN_TEST(test_tilt_fb_slight_no_warning);
    RUN_TEST(test_tilt_fb_at_threshold_no_warning);
    RUN_TEST(test_tilt_fb_beyond_threshold_triggers_warning);
    RUN_TEST(test_tilt_side_slight_no_warning);
    RUN_TEST(test_tilt_side_at_threshold_no_warning);
    RUN_TEST(test_tilt_side_beyond_threshold_triggers_warning);
    RUN_TEST(test_tilt_negative_fb_triggers_warning);
    RUN_TEST(test_tilt_negative_side_triggers_warning);
    RUN_TEST(test_tilt_combined_safe);
    RUN_TEST(test_tilt_combined_dangerous);
    RUN_TEST(test_tilt_combined_fb_dangerous_only);
    
    // Light Tests (9 tests)
    RUN_TEST(test_light_bright_no_fog);
    RUN_TEST(test_light_at_threshold_no_fog);
    RUN_TEST(test_light_below_threshold_activates_fog);
    RUN_TEST(test_light_dim_activates_fog);
    RUN_TEST(test_light_very_dark_activates_fog);
    RUN_TEST(test_light_zero_activates_fog);
    RUN_TEST(test_light_just_above_threshold_no_fog);
    RUN_TEST(test_light_just_below_threshold_activates_fog);
    RUN_TEST(test_light_max_adc_no_fog);
    
    // Actuator Tests (14 tests)
    RUN_TEST(test_fog_light_activates_in_darkness);
    RUN_TEST(test_fog_light_deactivates_in_brightness);
    RUN_TEST(test_warning_off_all_safe);
    RUN_TEST(test_warning_triggers_on_proximity);
    RUN_TEST(test_warning_triggers_on_tilt_fb);
    RUN_TEST(test_warning_triggers_on_tilt_side);
    RUN_TEST(test_warning_triggers_on_acceleration);
    RUN_TEST(test_warning_triggers_on_multiple_hazards);
    RUN_TEST(test_warning_on_max_tilt);
    RUN_TEST(test_warning_on_negative_tilt);
    RUN_TEST(test_warning_at_accel_threshold_boundary);
    RUN_TEST(test_warning_just_above_accel_threshold);
    
    // Mock Data Tests (6 tests)
    RUN_TEST(test_mock_data_at_start);
    RUN_TEST(test_mock_data_triggers_tilt_warning);
    RUN_TEST(test_mock_data_triggers_impact_warning);
    RUN_TEST(test_mock_data_combined_hazard);
    RUN_TEST(test_mock_data_distance_pattern);
    RUN_TEST(test_mock_data_light_pattern);
    
    UNITY_END();
}

void loop() {}

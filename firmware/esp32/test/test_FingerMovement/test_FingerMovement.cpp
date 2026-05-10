// Unit tests for FingerMovement – runs on native platform via Unity
#include <unity.h>
#include "FingerMovement.h"

// Provide the definition required by the Arduino.h mock
uint32_t mock_millis_value = 0;

void setUp()    {}
void tearDown() {}

// ── Constructor: one argument ─────────────────────────────────────────────

void test_one_arg_stores_target_position() {
    FingerMovement fm(5);
    TEST_ASSERT_EQUAL_INT(5, fm.normalizedTargetPosition);
}

void test_one_arg_default_delay_is_zero() {
    FingerMovement fm(5);
    TEST_ASSERT_EQUAL_UINT32(0, fm.startDelay);
}

void test_one_arg_default_step_equals_default() {
    FingerMovement fm(5);
    TEST_ASSERT_EQUAL_FLOAT(DEFAULT_STEP, fm.step);
}

// ── Constructor: two arguments ────────────────────────────────────────────

void test_two_arg_stores_target_position() {
    FingerMovement fm(7, 500);
    TEST_ASSERT_EQUAL_INT(7, fm.normalizedTargetPosition);
}

void test_two_arg_stores_delay() {
    FingerMovement fm(7, 500);
    TEST_ASSERT_EQUAL_UINT32(500, fm.startDelay);
}

void test_two_arg_default_step_equals_default() {
    FingerMovement fm(7, 500);
    TEST_ASSERT_EQUAL_FLOAT(DEFAULT_STEP, fm.step);
}

// ── Constructor: three arguments ──────────────────────────────────────────

void test_three_arg_stores_target_position() {
    FingerMovement fm(3, 200, 2.5f);
    TEST_ASSERT_EQUAL_INT(3, fm.normalizedTargetPosition);
}

void test_three_arg_stores_delay() {
    FingerMovement fm(3, 200, 2.5f);
    TEST_ASSERT_EQUAL_UINT32(200, fm.startDelay);
}

void test_three_arg_stores_custom_step() {
    FingerMovement fm(3, 200, 2.5f);
    TEST_ASSERT_EQUAL_FLOAT(2.5f, fm.step);
}

// ── DEFAULT_STEP constant ─────────────────────────────────────────────────

void test_default_step_is_one() {
    TEST_ASSERT_EQUAL_FLOAT(1.0f, DEFAULT_STEP);
}

// ── Boundary values ───────────────────────────────────────────────────────

void test_fully_open_position() {
    FingerMovement fm(0);
    TEST_ASSERT_EQUAL_INT(0, fm.normalizedTargetPosition);
}

void test_fully_closed_position() {
    FingerMovement fm(100);
    TEST_ASSERT_EQUAL_INT(100, fm.normalizedTargetPosition);
}

void test_zero_delay() {
    FingerMovement fm(50, 0);
    TEST_ASSERT_EQUAL_UINT32(0, fm.startDelay);
}

// ─────────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_one_arg_stores_target_position);
    RUN_TEST(test_one_arg_default_delay_is_zero);
    RUN_TEST(test_one_arg_default_step_equals_default);

    RUN_TEST(test_two_arg_stores_target_position);
    RUN_TEST(test_two_arg_stores_delay);
    RUN_TEST(test_two_arg_default_step_equals_default);

    RUN_TEST(test_three_arg_stores_target_position);
    RUN_TEST(test_three_arg_stores_delay);
    RUN_TEST(test_three_arg_stores_custom_step);

    RUN_TEST(test_default_step_is_one);
    RUN_TEST(test_fully_open_position);
    RUN_TEST(test_fully_closed_position);
    RUN_TEST(test_zero_delay);

    return UNITY_END();
}

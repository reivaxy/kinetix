// Unit tests for Hand – runs on native platform via Unity
#include <unity.h>
#include "Hand.h"

// Provide definitions required by the Arduino.h mock
uint32_t mock_millis_value = 0;
int mock_analog_read_value = 0;

// Right-hand defaults (no LEFT_HAND macro defined)
// THUMB:   maxOpen=30,  maxClosed=180, direction=1
// FINGER1: maxOpen=10,  maxClosed=180, direction=1
// FINGER2: maxOpen=180, maxClosed=0,   direction=-1
// FINGER3: maxOpen=0,   maxClosed=180, direction=1
// FINGER4: maxOpen=180, maxClosed=35,  direction=-1

// ── Fixtures ──────────────────────────────────────────────────────────────

static Settings* g_settings = nullptr;
static Hand*     g_hand     = nullptr;

void setUp() {
    mock_millis_value      = 0;
    mock_analog_read_value = 0;
    g_settings = new Settings();
    g_hand     = new Hand(g_settings);
}

void tearDown() {
    delete g_hand;     g_hand     = nullptr;
    delete g_settings; g_settings = nullptr;
}

// ── Construction ──────────────────────────────────────────────────────────

void test_hand_creates_five_fingers() {
    for (int i = 0; i < FINGER_COUNT; i++) {
        TEST_ASSERT_NOT_NULL(g_hand->fingers[i]);
    }
}

void test_thumb_max_open_is_default() {
    TEST_ASSERT_EQUAL_INT(THUMB_MAX_OPEN, g_hand->fingers[0]->maxOpen);
}

void test_thumb_max_closed_is_default() {
    TEST_ASSERT_EQUAL_INT(THUMB_MAX_CLOSED, g_hand->fingers[0]->maxClosed);
}

void test_finger1_max_open_is_default() {
    TEST_ASSERT_EQUAL_INT(FINGER1_MAX_OPEN, g_hand->fingers[1]->maxOpen);
}

void test_finger2_direction_is_inverted() {
    TEST_ASSERT_EQUAL_INT(-1, g_hand->fingers[2]->direction);
}

void test_finger3_direction_is_normal() {
    TEST_ASSERT_EQUAL_INT(1, g_hand->fingers[3]->direction);
}

void test_finger4_direction_is_inverted() {
    TEST_ASSERT_EQUAL_INT(-1, g_hand->fingers[4]->direction);
}

// ── open() / close() – all fingers ───────────────────────────────────────

void test_close_all_sets_thumb_target_to_max_closed() {
    g_hand->close();
    TEST_ASSERT_EQUAL_INT(THUMB_MAX_CLOSED, g_hand->fingers[0]->target);
}

void test_close_all_sets_finger1_target_to_max_closed() {
    g_hand->close();
    TEST_ASSERT_EQUAL_INT(FINGER1_MAX_CLOSED, g_hand->fingers[1]->target);
}

void test_close_all_sets_all_fingers() {
    g_hand->close();
    int expectedClosed[] = {
        THUMB_MAX_CLOSED, FINGER1_MAX_CLOSED, FINGER2_MAX_CLOSED,
        FINGER3_MAX_CLOSED, FINGER4_MAX_CLOSED
    };
    for (int i = 0; i < FINGER_COUNT; i++) {
        TEST_ASSERT_EQUAL_INT_MESSAGE(expectedClosed[i], g_hand->fingers[i]->target, "close(): wrong target");
    }
}

void test_open_all_sets_thumb_target_to_max_open() {
    g_hand->close(); // move to closed first
    g_hand->open();
    TEST_ASSERT_EQUAL_INT(THUMB_MAX_OPEN, g_hand->fingers[0]->target);
}

void test_open_all_sets_all_fingers() {
    g_hand->close();
    g_hand->open();
    int expectedOpen[] = {
        THUMB_MAX_OPEN, FINGER1_MAX_OPEN, FINGER2_MAX_OPEN,
        FINGER3_MAX_OPEN, FINGER4_MAX_OPEN
    };
    for (int i = 0; i < FINGER_COUNT; i++) {
        TEST_ASSERT_EQUAL_INT_MESSAGE(expectedOpen[i], g_hand->fingers[i]->target, "open(): wrong target");
    }
}

// ── open(finger) / close(finger) – single finger ─────────────────────────

void test_close_single_finger_does_not_affect_others() {
    g_hand->open(); // open all
    g_hand->close(0); // close only thumb
    TEST_ASSERT_EQUAL_INT(THUMB_MAX_CLOSED, g_hand->fingers[0]->target);
    TEST_ASSERT_EQUAL_INT(FINGER1_MAX_OPEN,  g_hand->fingers[1]->target);
}

void test_open_single_finger_does_not_affect_others() {
    g_hand->close(); // close all
    g_hand->open(1); // open only finger 1
    TEST_ASSERT_EQUAL_INT(THUMB_MAX_CLOSED,  g_hand->fingers[0]->target);
    TEST_ASSERT_EQUAL_INT(FINGER1_MAX_OPEN,  g_hand->fingers[1]->target);
}

void test_close_each_finger_individually() {
    int expectedClosed[] = {
        THUMB_MAX_CLOSED, FINGER1_MAX_CLOSED, FINGER2_MAX_CLOSED,
        FINGER3_MAX_CLOSED, FINGER4_MAX_CLOSED
    };
    for (int i = 0; i < FINGER_COUNT; i++) {
        g_hand->open();
        g_hand->close(i);
        TEST_ASSERT_EQUAL_INT_MESSAGE(expectedClosed[i], g_hand->fingers[i]->target, "close(i)");
    }
}

// ── moveRelative() ────────────────────────────────────────────────────────

void test_moveRelative_fully_open_sets_thumb_to_max_open() {
    // computeTarget(0) for direction=1: target = map(0, 0,100, maxOpen, maxClosed) = maxOpen
    g_hand->moveRelative(0);
    TEST_ASSERT_EQUAL_INT(THUMB_MAX_OPEN, g_hand->fingers[0]->target);
}

void test_moveRelative_fully_closed_sets_thumb_to_max_closed() {
    // computeTarget(100) for direction=1: target = map(100, 0,100, maxOpen, maxClosed) = maxClosed
    g_hand->moveRelative(100);
    TEST_ASSERT_EQUAL_INT(THUMB_MAX_CLOSED, g_hand->fingers[0]->target);
}

void test_moveRelative_fully_open_sets_all_fingers_to_open() {
    g_hand->moveRelative(0);
    int expectedOpen[] = {
        THUMB_MAX_OPEN, FINGER1_MAX_OPEN, FINGER2_MAX_OPEN,
        FINGER3_MAX_OPEN, FINGER4_MAX_OPEN
    };
    for (int i = 0; i < FINGER_COUNT; i++) {
        TEST_ASSERT_EQUAL_INT_MESSAGE(expectedOpen[i], g_hand->fingers[i]->target, "moveRelative(0)");
    }
}

void test_moveRelative_fully_closed_sets_all_fingers_to_closed() {
    g_hand->moveRelative(100);
    int expectedClosed[] = {
        THUMB_MAX_CLOSED, FINGER1_MAX_CLOSED, FINGER2_MAX_CLOSED,
        FINGER3_MAX_CLOSED, FINGER4_MAX_CLOSED
    };
    for (int i = 0; i < FINGER_COUNT; i++) {
        TEST_ASSERT_EQUAL_INT_MESSAGE(expectedClosed[i], g_hand->fingers[i]->target, "moveRelative(100)");
    }
}

void test_moveRelative_midpoint_thumb() {
    // direction=1: target = map(50, 0, 100, 30, 180) = 105
    g_hand->moveRelative(50);
    int expected = map(50, 0, 100, THUMB_MAX_OPEN, THUMB_MAX_CLOSED);
    TEST_ASSERT_EQUAL_INT(expected, g_hand->fingers[0]->target);
}

void test_moveRelative_updates_normalized_position() {
    g_hand->moveRelative(75);
    TEST_ASSERT_EQUAL_INT(75, g_hand->fingers[0]->currentNormalizedPosition);
}

// ── setCalibration() ──────────────────────────────────────────────────────

void test_setCalibration_true_sets_thumb_max_open_to_calibration() {
    g_hand->setCalibration(true);
    TEST_ASSERT_EQUAL_INT(SERVO_CALIBRATION_OPEN, g_hand->fingers[0]->maxOpen);
}

void test_setCalibration_true_sets_thumb_max_closed_to_calibration() {
    g_hand->setCalibration(true);
    TEST_ASSERT_EQUAL_INT(SERVO_CALIBRATION_CLOSED, g_hand->fingers[0]->maxClosed);
}

void test_setCalibration_true_inverted_finger_uses_swapped_values() {
    // fingers[2] is mounted inverted → setMax(CLOSED, OPEN)
    g_hand->setCalibration(true);
    TEST_ASSERT_EQUAL_INT(SERVO_CALIBRATION_CLOSED, g_hand->fingers[2]->maxOpen);
    TEST_ASSERT_EQUAL_INT(SERVO_CALIBRATION_OPEN,   g_hand->fingers[2]->maxClosed);
}

void test_setCalibration_false_restores_factory_thumb() {
    g_hand->setCalibration(true);
    g_hand->setCalibration(false);
    TEST_ASSERT_EQUAL_INT(THUMB_MAX_OPEN,   g_hand->fingers[0]->maxOpen);
    TEST_ASSERT_EQUAL_INT(THUMB_MAX_CLOSED, g_hand->fingers[0]->maxClosed);
}

void test_setCalibration_false_restores_all_factory_defaults() {
    g_hand->setCalibration(true);
    g_hand->setCalibration(false);
    int expectedOpen[]   = {THUMB_MAX_OPEN,   FINGER1_MAX_OPEN,   FINGER2_MAX_OPEN,   FINGER3_MAX_OPEN,   FINGER4_MAX_OPEN};
    int expectedClosed[] = {THUMB_MAX_CLOSED, FINGER1_MAX_CLOSED, FINGER2_MAX_CLOSED, FINGER3_MAX_CLOSED, FINGER4_MAX_CLOSED};
    for (int i = 0; i < FINGER_COUNT; i++) {
        TEST_ASSERT_EQUAL_INT_MESSAGE(expectedOpen[i],   g_hand->fingers[i]->maxOpen,   "maxOpen after restore");
        TEST_ASSERT_EQUAL_INT_MESSAGE(expectedClosed[i], g_hand->fingers[i]->maxClosed, "maxClosed after restore");
    }
}

// ── updateMaxPositionsFromSettings() ─────────────────────────────────────

void test_updateMaxPositions_restores_factory_defaults_when_no_stored_settings() {
    // Force calibration values, then restore via updateMaxPositionsFromSettings
    g_hand->setCalibration(true);
    g_hand->updateMaxPositionsFromSettings();
    TEST_ASSERT_EQUAL_INT(THUMB_MAX_OPEN,   g_hand->fingers[0]->maxOpen);
    TEST_ASSERT_EQUAL_INT(THUMB_MAX_CLOSED, g_hand->fingers[0]->maxClosed);
}

// ── run() ─────────────────────────────────────────────────────────────────

void test_run_does_not_crash() {
    g_hand->close();
    g_hand->run();
    TEST_PASS();
}

void test_run_single_finger_does_not_crash() {
    g_hand->close();
    g_hand->run(2);
    TEST_PASS();
}

void test_run_advances_finger_toward_target() {
    g_hand->fingers[0]->target          = THUMB_MAX_CLOSED;
    g_hand->fingers[0]->currentPosition = THUMB_MAX_OPEN;
    mock_millis_value = 1000; // well past any delay
    g_hand->run(0);
    TEST_ASSERT_GREATER_THAN(THUMB_MAX_OPEN, (int)g_hand->fingers[0]->currentPosition);
}

// ── stop(uint finger) ─────────────────────────────────────────────────────

void test_stop_single_finger_sets_target_to_current_position() {
    // Move finger partway, then stop it – target must equal currentPosition
    g_hand->fingers[1]->currentPosition = 50.0f;
    g_hand->fingers[1]->target          = 150;
    g_hand->stop(1);
    TEST_ASSERT_EQUAL_INT(50, g_hand->fingers[1]->target);
}

void test_stop_single_finger_does_not_affect_others() {
    g_hand->fingers[0]->target = THUMB_MAX_CLOSED;
    g_hand->fingers[1]->currentPosition = 50.0f;
    g_hand->fingers[1]->target          = 150;
    g_hand->stop(1);
    // Finger 0 target must be unchanged
    TEST_ASSERT_EQUAL_INT(THUMB_MAX_CLOSED, g_hand->fingers[0]->target);
}

// ── setStep(float) / setStep(int, float) ─────────────────────────────────

void test_setStep_all_changes_every_finger_step() {
    g_hand->setStep(3.5f);
    for (int i = 0; i < FINGER_COUNT; i++) {
        TEST_ASSERT_EQUAL_FLOAT(3.5f, g_hand->fingers[i]->step);
    }
}

void test_setStep_single_finger_changes_only_that_finger() {
    g_hand->setStep(0, 2.0f);
    TEST_ASSERT_EQUAL_FLOAT(2.0f, g_hand->fingers[0]->step);
    // Other fingers must still have the default step
    TEST_ASSERT_EQUAL_FLOAT(DEFAULT_STEP, g_hand->fingers[1]->step);
}

void test_setStep_single_finger_all_indices() {
    for (int i = 0; i < FINGER_COUNT; i++) {
        float s = 1.0f + i * 0.5f;
        g_hand->setStep(i, s);
        TEST_ASSERT_EQUAL_FLOAT(s, g_hand->fingers[i]->step);
    }
}

// ── isStill() ────────────────────────────────────────────────────────────

void test_isStill_true_when_all_fingers_at_target() {
    // Align every finger's target with its currentPosition so all are still
    for (int i = 0; i < FINGER_COUNT; i++) {
        g_hand->fingers[i]->target = (int)g_hand->fingers[i]->currentPosition;
    }
    TEST_ASSERT_TRUE(g_hand->isStill());
}

void test_isStill_false_when_one_finger_not_at_target() {
    // Push finger 2 target far from currentPosition
    g_hand->fingers[2]->target = (int)g_hand->fingers[2]->currentPosition + 50;
    TEST_ASSERT_FALSE(g_hand->isStill());
}

void test_isStill_single_finger_true_at_target() {
    // Align finger 0's target with its currentPosition
    g_hand->fingers[0]->target = (int)g_hand->fingers[0]->currentPosition;
    TEST_ASSERT_TRUE(g_hand->isStill(0));
}

void test_isStill_single_finger_false_away_from_target() {
    g_hand->fingers[3]->target = (int)g_hand->fingers[3]->currentPosition + 50;
    TEST_ASSERT_FALSE(g_hand->isStill(3));
}

// ── setFinger() with null FingerMovement ──────────────────────────────────

void test_setFinger_null_is_noop() {
    // Preserve current target, then call setFinger with nullptr – must not crash or change target
    int originalTarget = g_hand->fingers[0]->target;
    g_hand->setFinger(0, nullptr);
    TEST_ASSERT_EQUAL_INT(originalTarget, g_hand->fingers[0]->target);
}

// ── Finger::setMaxOpen / setMaxClosed / setStep ───────────────────────────

void test_finger_setMaxOpen_updates_value() {
    g_hand->fingers[0]->setMaxOpen(55);
    TEST_ASSERT_EQUAL_INT(55, g_hand->fingers[0]->maxOpen);
}

void test_finger_setMaxClosed_updates_value() {
    g_hand->fingers[0]->setMaxClosed(170);
    TEST_ASSERT_EQUAL_INT(170, g_hand->fingers[0]->maxClosed);
}

void test_finger_setStep_updates_value() {
    g_hand->fingers[1]->setStep(4.0f);
    TEST_ASSERT_EQUAL_FLOAT(4.0f, g_hand->fingers[1]->step);
}

// ── Finger::run() delay early-return ─────────────────────────────────────

void test_finger_run_respects_start_delay() {
    // Give finger 0 a movement with a 500 ms start delay
    FingerMovement fm(100, 500); // target=100%, delay=500ms
    g_hand->fingers[0]->setMovement(&fm);

    // Record position before run
    float positionBefore = g_hand->fingers[0]->currentPosition;

    // Run at t=0 – within the delay → early return, no position change
    mock_millis_value = 0;
    g_hand->fingers[0]->run();
    TEST_ASSERT_EQUAL_FLOAT(positionBefore, g_hand->fingers[0]->currentPosition);
}

void test_finger_run_moves_after_delay_elapsed() {
    FingerMovement fm(100, 500);
    g_hand->fingers[0]->setMovement(&fm);
    float positionBefore = g_hand->fingers[0]->currentPosition;

    // Run after delay expires
    mock_millis_value = 501;
    g_hand->fingers[0]->run();
    // currentPosition should have stepped toward target
    TEST_ASSERT_NOT_EQUAL((int)positionBefore, (int)g_hand->fingers[0]->currentPosition);
}

// ── Finger::setMovement with null (guard branch) ──────────────────────────

void test_finger_setMovement_null_is_noop() {
    int originalTarget = g_hand->fingers[0]->target;
    g_hand->fingers[0]->setMovement(nullptr);
    TEST_ASSERT_EQUAL_INT(originalTarget, g_hand->fingers[0]->target);
}

// ─────────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_hand_creates_five_fingers);
    RUN_TEST(test_thumb_max_open_is_default);
    RUN_TEST(test_thumb_max_closed_is_default);
    RUN_TEST(test_finger1_max_open_is_default);
    RUN_TEST(test_finger2_direction_is_inverted);
    RUN_TEST(test_finger3_direction_is_normal);
    RUN_TEST(test_finger4_direction_is_inverted);

    RUN_TEST(test_close_all_sets_thumb_target_to_max_closed);
    RUN_TEST(test_close_all_sets_finger1_target_to_max_closed);
    RUN_TEST(test_close_all_sets_all_fingers);
    RUN_TEST(test_open_all_sets_thumb_target_to_max_open);
    RUN_TEST(test_open_all_sets_all_fingers);

    RUN_TEST(test_close_single_finger_does_not_affect_others);
    RUN_TEST(test_open_single_finger_does_not_affect_others);
    RUN_TEST(test_close_each_finger_individually);

    RUN_TEST(test_moveRelative_fully_open_sets_thumb_to_max_open);
    RUN_TEST(test_moveRelative_fully_closed_sets_thumb_to_max_closed);
    RUN_TEST(test_moveRelative_fully_open_sets_all_fingers_to_open);
    RUN_TEST(test_moveRelative_fully_closed_sets_all_fingers_to_closed);
    RUN_TEST(test_moveRelative_midpoint_thumb);
    RUN_TEST(test_moveRelative_updates_normalized_position);

    RUN_TEST(test_setCalibration_true_sets_thumb_max_open_to_calibration);
    RUN_TEST(test_setCalibration_true_sets_thumb_max_closed_to_calibration);
    RUN_TEST(test_setCalibration_true_inverted_finger_uses_swapped_values);
    RUN_TEST(test_setCalibration_false_restores_factory_thumb);
    RUN_TEST(test_setCalibration_false_restores_all_factory_defaults);

    RUN_TEST(test_updateMaxPositions_restores_factory_defaults_when_no_stored_settings);

    RUN_TEST(test_run_does_not_crash);
    RUN_TEST(test_run_single_finger_does_not_crash);
    RUN_TEST(test_run_advances_finger_toward_target);

    RUN_TEST(test_stop_single_finger_sets_target_to_current_position);
    RUN_TEST(test_stop_single_finger_does_not_affect_others);

    RUN_TEST(test_setStep_all_changes_every_finger_step);
    RUN_TEST(test_setStep_single_finger_changes_only_that_finger);
    RUN_TEST(test_setStep_single_finger_all_indices);

    RUN_TEST(test_isStill_true_when_all_fingers_at_target);
    RUN_TEST(test_isStill_false_when_one_finger_not_at_target);
    RUN_TEST(test_isStill_single_finger_true_at_target);
    RUN_TEST(test_isStill_single_finger_false_away_from_target);

    RUN_TEST(test_setFinger_null_is_noop);

    RUN_TEST(test_finger_setMaxOpen_updates_value);
    RUN_TEST(test_finger_setMaxClosed_updates_value);
    RUN_TEST(test_finger_setStep_updates_value);

    RUN_TEST(test_finger_run_respects_start_delay);
    RUN_TEST(test_finger_run_moves_after_delay_elapsed);

    RUN_TEST(test_finger_setMovement_null_is_noop);

    return UNITY_END();
}

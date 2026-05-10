// Unit tests for Sequence – runs on native platform via Unity
#include <unity.h>
#include "Sequence.h"
#include "Settings.h"
#include "Hand.h"

// Provide the definition required by the Arduino.h mock
uint32_t mock_millis_value = 0;

// Shared objects – re-created each test via setUp/tearDown
static Settings* g_settings = nullptr;
static Hand*     g_hand     = nullptr;

void setUp() {
    mock_millis_value = 0;
    g_settings = new Settings();
    g_hand     = new Hand(g_settings);
}

void tearDown() {
    // Fingers are owned by Hand but Hand has no destructor; leak is acceptable in tests.
    delete g_hand;     g_hand     = nullptr;
    delete g_settings; g_settings = nullptr;
}

// Helper: allocate a HandMovement owned by the Sequence (Sequence will delete it)
static HandMovement* makeHM() {
    return new HandMovement(g_hand, "test");
}

// ── Initial state ─────────────────────────────────────────────────────────

void test_sequence_not_running_initially() {
    Sequence seq;
    TEST_ASSERT_FALSE(seq.isRunning());
}

void test_sequence_zero_movements_initially() {
    Sequence seq;
    TEST_ASSERT_EQUAL_UINT8(0, seq.movementCount);
}

// ── Repeat-count constructor ──────────────────────────────────────────────

void test_sequence_default_repeat_count_is_one() {
    Sequence seq;
    TEST_ASSERT_EQUAL_UINT8(1, seq.repeatCount);
}

void test_sequence_custom_repeat_count_stored() {
    Sequence seq(3);
    TEST_ASSERT_EQUAL_UINT8(3, seq.repeatCount);
}

void test_sequence_zero_repeat_count_means_forever() {
    Sequence seq(0);
    TEST_ASSERT_EQUAL_UINT8(0, seq.repeatCount);
}

// ── addMovement ───────────────────────────────────────────────────────────

void test_add_one_movement_increments_count() {
    Sequence seq;
    seq.addMovement(makeHM(), 1000);
    TEST_ASSERT_EQUAL_UINT8(1, seq.movementCount);
}

void test_add_multiple_movements_increments_count() {
    Sequence seq;
    seq.addMovement(makeHM(), 1000);
    seq.addMovement(makeHM(), 2000);
    seq.addMovement(makeHM(), 500);
    TEST_ASSERT_EQUAL_UINT8(3, seq.movementCount);
}

void test_add_movement_stores_duration() {
    Sequence seq;
    seq.addMovement(makeHM(), 3000);
    TEST_ASSERT_EQUAL_UINT32(3000, seq.durations[0]);
}

void test_add_movement_default_duration() {
    Sequence seq;
    seq.addMovement(makeHM()); // default duration = 2000
    TEST_ASSERT_EQUAL_UINT32(2000, seq.durations[0]);
}

void test_add_movement_stores_pointer() {
    Sequence seq;
    HandMovement* hm = makeHM();
    seq.addMovement(hm, 1000);
    TEST_ASSERT_EQUAL_PTR(hm, seq.movements[0]);
}

// ── Capacity cap ──────────────────────────────────────────────────────────

void test_add_beyond_max_is_ignored() {
    Sequence seq;
    for (int i = 0; i <= MAX_MOVEMENTS; i++) {
        seq.addMovement(makeHM(), 100);
    }
    // The (MAX_MOVEMENTS + 1)-th call must be silently ignored
    TEST_ASSERT_EQUAL_UINT8(MAX_MOVEMENTS, seq.movementCount);
}

// ─────────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_sequence_not_running_initially);
    RUN_TEST(test_sequence_zero_movements_initially);

    RUN_TEST(test_sequence_default_repeat_count_is_one);
    RUN_TEST(test_sequence_custom_repeat_count_stored);
    RUN_TEST(test_sequence_zero_repeat_count_means_forever);

    RUN_TEST(test_add_one_movement_increments_count);
    RUN_TEST(test_add_multiple_movements_increments_count);
    RUN_TEST(test_add_movement_stores_duration);
    RUN_TEST(test_add_movement_default_duration);
    RUN_TEST(test_add_movement_stores_pointer);

    RUN_TEST(test_add_beyond_max_is_ignored);

    return UNITY_END();
}

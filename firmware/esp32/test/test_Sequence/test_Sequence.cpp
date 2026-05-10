// Unit tests for Sequence – runs on native platform via Unity
#include <unity.h>
#include "Sequence.h"
#include "Settings.h"
#include "Hand.h"

// Provide the definitions required by the Arduino.h mock
uint32_t mock_millis_value = 0;
int mock_analog_read_value = 0;

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

// ── start() / stop() / isRunning() ───────────────────────────────────────

void test_start_sets_running_true() {
    Sequence seq;
    seq.addMovement(makeHM(), 1000);
    mock_millis_value = 0;
    seq.start();
    TEST_ASSERT_TRUE(seq.isRunning());
}

void test_stop_sets_running_false() {
    Sequence seq;
    seq.addMovement(makeHM(), 1000);
    mock_millis_value = 0;
    seq.start();
    seq.stop();
    TEST_ASSERT_FALSE(seq.isRunning());
}

void test_start_sets_current_to_zero() {
    Sequence seq;
    seq.addMovement(makeHM(), 1000);
    seq.addMovement(makeHM(), 1000);
    mock_millis_value = 0;
    seq.start();
    TEST_ASSERT_EQUAL_UINT8(0, seq.current);
}

// ── run() – early-return branches ────────────────────────────────────────

void test_run_when_not_running_is_noop() {
    Sequence seq;
    seq.addMovement(makeHM(), 1000);
    // Do NOT start – running == false → early return
    seq.run();
    TEST_ASSERT_FALSE(seq.isRunning());
}

// ── run() – timing: does not advance before duration expires ─────────────

void test_run_does_not_advance_before_duration() {
    Sequence seq;
    seq.addMovement(makeHM(), 1000);
    seq.addMovement(makeHM(), 1000);
    mock_millis_value = 0;
    seq.start();
    mock_millis_value = 500; // < duration
    seq.run();
    TEST_ASSERT_EQUAL_UINT8(0, seq.current); // still at first movement
}

// ── run() – timing: advances to next movement after duration ─────────────

void test_run_advances_to_next_movement_after_duration() {
    Sequence seq;
    seq.addMovement(makeHM(), 1000);
    seq.addMovement(makeHM(), 1000);
    mock_millis_value = 0;
    seq.start();
    mock_millis_value = 1001; // > duration[0]
    seq.run();
    TEST_ASSERT_EQUAL_UINT8(1, seq.current);
    TEST_ASSERT_TRUE(seq.isRunning());
}

void test_run_advances_through_two_movements() {
    Sequence seq(1);
    seq.addMovement(makeHM(), 1000);
    seq.addMovement(makeHM(), 500);
    mock_millis_value = 0;
    seq.start();
    // Advance past first movement
    mock_millis_value = 1001;
    seq.run();
    TEST_ASSERT_EQUAL_UINT8(1, seq.current);
    TEST_ASSERT_TRUE(seq.isRunning());
    // Advance past second movement → one full loop → repeatCount(1) → stop
    mock_millis_value = 1001 + 501;
    seq.run();
    TEST_ASSERT_FALSE(seq.isRunning());
}

// ── run() – finite repeat count ───────────────────────────────────────────

void test_run_stops_finite_sequence_after_repeat_count() {
    // repeatCount=1, one movement of 1000 ms
    Sequence seq(1);
    seq.addMovement(makeHM(), 1000);
    mock_millis_value = 0;
    seq.start();
    mock_millis_value = 1001; // exceeds duration → completes loop 1 → stop
    seq.run();
    TEST_ASSERT_FALSE(seq.isRunning());
}

void test_run_loopCount_increments_on_each_loop() {
    Sequence seq(2);
    seq.addMovement(makeHM(), 1000);
    mock_millis_value = 0;
    seq.start();
    // First loop completes
    mock_millis_value = 1001;
    seq.run();
    TEST_ASSERT_EQUAL_UINT8(1, seq.loopCount);
    TEST_ASSERT_TRUE(seq.isRunning()); // second loop not yet done
    // Second loop completes → repeatCount(2) reached → stop
    mock_millis_value = 2002;
    seq.run();
    TEST_ASSERT_EQUAL_UINT8(2, seq.loopCount);
    TEST_ASSERT_FALSE(seq.isRunning());
}

// ── run() – infinite (repeatCount=0) ─────────────────────────────────────

void test_run_does_not_stop_for_infinite_sequence() {
    Sequence seq(0); // repeat forever
    seq.addMovement(makeHM(), 1000);
    mock_millis_value = 0;
    seq.start();
    // Two full loops – sequence must still be running
    mock_millis_value = 1001;
    seq.run();
    TEST_ASSERT_TRUE(seq.isRunning());
    mock_millis_value = 2002;
    seq.run();
    TEST_ASSERT_TRUE(seq.isRunning());
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

    RUN_TEST(test_start_sets_running_true);
    RUN_TEST(test_stop_sets_running_false);
    RUN_TEST(test_start_sets_current_to_zero);

    RUN_TEST(test_run_when_not_running_is_noop);
    RUN_TEST(test_run_does_not_advance_before_duration);
    RUN_TEST(test_run_advances_to_next_movement_after_duration);
    RUN_TEST(test_run_advances_through_two_movements);
    RUN_TEST(test_run_stops_finite_sequence_after_repeat_count);
    RUN_TEST(test_run_loopCount_increments_on_each_loop);
    RUN_TEST(test_run_does_not_stop_for_infinite_sequence);

    return UNITY_END();
}

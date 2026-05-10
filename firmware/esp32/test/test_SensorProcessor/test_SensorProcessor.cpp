// Unit tests for RealSensorProcessor – runs on native platform via Unity
#include <unity.h>
#include "OptionalSensorProcessor.h"

// Provide definitions required by the Arduino.h mock
uint32_t mock_millis_value = 0;
int mock_analog_read_value = 0;

// ── Spy Display ───────────────────────────────────────────────────────────

class SpyDisplay : public Display {
public:
    int         lastLineIdx  = -1;
    std::string lastLineText;

    void setTitle(String /*t*/)            override {}
    void setLine(int line, String content) override {
        lastLineIdx  = line;
        lastLineText = content.c_str();
    }
    void refresh()                         override {}
};

// ── Fixtures ──────────────────────────────────────────────────────────────

static Settings*             g_settings  = nullptr;
static Hand*                 g_hand      = nullptr;
static SpyDisplay*           g_display   = nullptr;
static RealSensorProcessor*  g_sensor    = nullptr;

void setUp() {
    mock_millis_value     = 0;
    mock_analog_read_value = 0;

    g_settings = new Settings();
    g_hand     = new Hand(g_settings);
    g_display  = new SpyDisplay();
    g_sensor   = new RealSensorProcessor(g_hand, g_settings, g_display);
}

void tearDown() {
    delete g_sensor;   g_sensor   = nullptr;
    delete g_display;  g_display  = nullptr;
    delete g_hand;     g_hand     = nullptr;
    delete g_settings; g_settings = nullptr;
}

// ── getAvg() – circular buffer ────────────────────────────────────────────

void test_getAvg_single_reading_equals_that_reading() {
    mock_analog_read_value = 1000;
    uint16_t avg = g_sensor->getAvg();
    TEST_ASSERT_EQUAL_UINT16(1000, avg);
}

void test_getAvg_two_readings_returns_mean() {
    mock_analog_read_value = 1000;
    g_sensor->getAvg();
    mock_analog_read_value = 2000;
    uint16_t avg = g_sensor->getAvg();
    TEST_ASSERT_EQUAL_UINT16(1500, avg);
}

void test_getAvg_returns_average_of_all_readings_below_max() {
    // Fill buffer with known values
    for (int i = 0; i < MAX_READINGS - 1; i++) {
        mock_analog_read_value = 100;
        g_sensor->getAvg();
    }
    mock_analog_read_value = 200;
    uint16_t avg = g_sensor->getAvg();
    // 4 readings of 100, 1 of 200 → sum = 600, count = 5 → avg = 120
    TEST_ASSERT_EQUAL_UINT16(120, avg);
}

void test_getAvg_oldest_reading_dropped_after_max_readings() {
    // Fill buffer completely with 100
    for (int i = 0; i < MAX_READINGS; i++) {
        mock_analog_read_value = 100;
        g_sensor->getAvg();
    }
    // Now push one more (200) – oldest 100 must be evicted
    mock_analog_read_value = 200;
    uint16_t avg = g_sensor->getAvg();
    // Buffer: 4×100 + 1×200 = 600 / 5 = 120
    TEST_ASSERT_EQUAL_UINT16(120, avg);
}

void test_getAvg_constant_input_returns_that_value() {
    mock_analog_read_value = 512;
    for (int i = 0; i < MAX_READINGS + 2; i++) {
        g_sensor->getAvg();
    }
    uint16_t avg = g_sensor->getAvg();
    TEST_ASSERT_EQUAL_UINT16(512, avg);
}

// ── run() – timing guard ─────────────────────────────────────────────────

void test_run_does_not_measure_before_interval() {
    // Trigger the first real measurement at t > measureIntervalMs
    mock_millis_value      = g_sensor->measureIntervalMs + 1;
    mock_analog_read_value = 0; // position=380, prev=0, change=380>threshold → move
    g_sensor->run();
    int targetAfterFirst = g_hand->fingers[0]->target;

    // Advance time but NOT enough for a second measurement (less than one full interval later)
    mock_millis_value     += g_sensor->measureIntervalMs - 1;
    mock_analog_read_value = 3800; // different reading – must be ignored
    g_sensor->run();

    // Target must not have changed
    TEST_ASSERT_EQUAL_INT(targetAfterFirst, g_hand->fingers[0]->target);
}

void test_run_measures_after_interval_elapsed() {
    // At t=0: condition (0 > measureIntervalMs) is false – no measurement fires
    mock_millis_value      = 0;
    mock_analog_read_value = 3800;
    g_sensor->run();
    int targetBeforeMeasurement = g_hand->fingers[0]->target;

    // At t > measureIntervalMs: condition passes; reading=0 → position=380,
    // prev=0, change=380 > threshold(2) → move
    mock_millis_value      = g_sensor->measureIntervalMs + 1;
    mock_analog_read_value = 0;
    g_sensor->run();

    TEST_ASSERT_NOT_EQUAL(targetBeforeMeasurement, g_hand->fingers[0]->target);
}

// ── run() – threshold guard ───────────────────────────────────────────────

void test_run_does_not_move_when_change_below_threshold() {
    // First measurement at t > measureIntervalMs: reading=0, position=380,
    // prev=0, change=380>threshold → move; previousPosition becomes 380
    mock_millis_value      = g_sensor->measureIntervalMs + 1;
    mock_analog_read_value = 0;
    g_sensor->run();
    int targetAfterFirst = g_hand->fingers[0]->target;

    // Second measurement: reading=10, position=380-1=379, |379-380|=1 ≤ threshold(2) → no move
    mock_millis_value      = 2 * (g_sensor->measureIntervalMs + 1);
    mock_analog_read_value = 10;
    g_sensor->run();

    TEST_ASSERT_EQUAL_INT(targetAfterFirst, g_hand->fingers[0]->target);
}

void test_run_moves_when_change_meets_threshold() {
    // Default offset=380, threshold=2
    // First measurement: reading=0 → position=380, previousPosition=380
    mock_millis_value     = 0;
    mock_analog_read_value = 0;
    g_sensor->run();
    int targetAfterFirst = g_hand->fingers[0]->target;

    // Second measurement: reading=40 → position = 380 - 4 = 376, change = 4 > threshold(2)
    mock_millis_value      = g_sensor->measureIntervalMs + 1;
    mock_analog_read_value = 40;
    g_sensor->run();

    TEST_ASSERT_NOT_EQUAL(targetAfterFirst, g_hand->fingers[0]->target);
}

// ── run() – position clamping ─────────────────────────────────────────────

void test_run_clamps_position_to_zero_when_reading_exceeds_offset() {
    // offset=380, reading=4000 → position = 380 - 400 = -20, clamped to 0
    // previousPosition=0, change=|0-0|=0 ≤ threshold(2) → no move, no display update
    mock_millis_value      = g_sensor->measureIntervalMs + 1;
    mock_analog_read_value = 4000;
    g_sensor->run();
    TEST_ASSERT_EQUAL_INT(-1, g_display->lastLineIdx); // display not updated
}

// ── run() – display update ────────────────────────────────────────────────

void test_run_updates_display_when_position_changes() {
    // reading=0: position=380, prev=0, change=380 > threshold(2) → display updated
    mock_millis_value      = g_sensor->measureIntervalMs + 1;
    mock_analog_read_value = 0;
    g_sensor->run();

    TEST_ASSERT_EQUAL_INT(SENSOR_DISPLAY_LINE, g_display->lastLineIdx);
}

void test_run_display_contains_sensor_label() {
    mock_millis_value      = g_sensor->measureIntervalMs + 1;
    mock_analog_read_value = 0;
    g_sensor->run();

    TEST_ASSERT_NOT_EQUAL(std::string::npos,
        g_display->lastLineText.find("Sensor: "));
}

// ── Initial state ─────────────────────────────────────────────────────────

void test_initial_previous_position_is_zero() {
    TEST_ASSERT_EQUAL_UINT16(0, g_sensor->previousPosition);
}

void test_initial_count_is_zero() {
    TEST_ASSERT_EQUAL_INT(0, g_sensor->count);
}

void test_initial_sum_is_zero() {
    TEST_ASSERT_EQUAL_INT(0, g_sensor->sum);
}

// ─────────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_getAvg_single_reading_equals_that_reading);
    RUN_TEST(test_getAvg_two_readings_returns_mean);
    RUN_TEST(test_getAvg_returns_average_of_all_readings_below_max);
    RUN_TEST(test_getAvg_oldest_reading_dropped_after_max_readings);
    RUN_TEST(test_getAvg_constant_input_returns_that_value);

    RUN_TEST(test_run_does_not_measure_before_interval);
    RUN_TEST(test_run_measures_after_interval_elapsed);

    RUN_TEST(test_run_does_not_move_when_change_below_threshold);
    RUN_TEST(test_run_moves_when_change_meets_threshold);

    RUN_TEST(test_run_clamps_position_to_zero_when_reading_exceeds_offset);

    RUN_TEST(test_run_updates_display_when_position_changes);
    RUN_TEST(test_run_display_contains_sensor_label);

    RUN_TEST(test_initial_previous_position_is_zero);
    RUN_TEST(test_initial_count_is_zero);
    RUN_TEST(test_initial_sum_is_zero);

    return UNITY_END();
}

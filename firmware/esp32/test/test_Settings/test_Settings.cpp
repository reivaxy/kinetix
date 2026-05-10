// Unit tests for Settings – runs on native platform via Unity
#include <unity.h>
#include "Settings.h"

// Provide definitions required by the Arduino.h mock
uint32_t mock_millis_value = 0;
int mock_analog_read_value = 0;

static Settings* g_s = nullptr;

void setUp() {
    g_s = new Settings();
}

void tearDown() {
    delete g_s; g_s = nullptr;
}

// ── getInt ────────────────────────────────────────────────────────────────

void test_getInt_returns_default_when_key_not_set() {
    TEST_ASSERT_EQUAL_INT(99, g_s->getInt("i_missing", 99));
}

void test_getInt_returns_zero_default() {
    TEST_ASSERT_EQUAL_INT(0, g_s->getInt("i_missing", 0));
}

// ── setInt / setString / setBool – no-crash coverage ─────────────────────

void test_setInt_no_crash() {
    g_s->setInt("i_speed", 42);
    TEST_PASS();
}

void test_setString_no_crash() {
    g_s->setString("s_label", "hello");
    TEST_PASS();
}

void test_setBool_no_crash() {
    g_s->setBool("b_flag", true);
    TEST_PASS();
}

// ── getString ─────────────────────────────────────────────────────────────

void test_getString_returns_default_when_key_not_set() {
    TEST_ASSERT_EQUAL_STRING("default", g_s->getString("s_missing", "default"));
}

void test_getString_returns_null_default_as_null() {
    TEST_ASSERT_NULL(g_s->getString("s_missing", nullptr));
}

// ── getBool ───────────────────────────────────────────────────────────────

void test_getBool_returns_false_default() {
    TEST_ASSERT_FALSE(g_s->getBool("b_missing", false));
}

void test_getBool_returns_true_default() {
    TEST_ASSERT_TRUE(g_s->getBool("b_missing", true));
}

// ── getPosition ───────────────────────────────────────────────────────────

void test_getPosition_returns_default_when_key_not_set() {
    TEST_ASSERT_EQUAL_INT(42, g_s->getPosition("finger_x", 42));
}

// ── getSettingJson / getPositionsJson ─────────────────────────────────────

void test_getSettingJson_returns_json_string() {
    String json = g_s->getSettingJson();
    // ArduinoJson stub always serialises to "{}"
    TEST_ASSERT_EQUAL_STRING("{}", json.c_str());
}

void test_getPositionsJson_returns_json_string() {
    String json = g_s->getPositionsJson();
    TEST_ASSERT_EQUAL_STRING("{}", json.c_str());
}

// ── updateSetting – all switch branches ──────────────────────────────────

void test_updateSetting_integer_type_no_crash() {
    char msg[] = "i_speed=100";
    g_s->updateSetting(msg);
    TEST_PASS();
}

void test_updateSetting_string_type_no_crash() {
    char msg[] = "s_label=TestLabel";
    g_s->updateSetting(msg);
    TEST_PASS();
}

void test_updateSetting_boolean_true_no_crash() {
    char msg[] = "b_enable=true";
    g_s->updateSetting(msg);
    TEST_PASS();
}

void test_updateSetting_boolean_false_no_crash() {
    char msg[] = "b_enable=false";
    g_s->updateSetting(msg);
    TEST_PASS();
}

void test_updateSetting_boolean_one_no_crash() {
    char msg[] = "b_enable=1";
    g_s->updateSetting(msg);
    TEST_PASS();
}

void test_updateSetting_unknown_prefix_no_crash() {
    char msg[] = "x_unknown=value";
    g_s->updateSetting(msg);
    TEST_PASS();
}

void test_updateSetting_no_equals_no_crash() {
    char msg[] = "invalidformat";
    g_s->updateSetting(msg);
    TEST_PASS();
}

// ── updatePosition ────────────────────────────────────────────────────────

void test_updatePosition_key_value_no_crash() {
    char msg[] = "thumb_max_open=30";
    g_s->updatePosition(msg);
    TEST_PASS();
}

void test_updatePosition_reset_no_crash() {
    char msg[] = "reset";
    g_s->updatePosition(msg);
    TEST_PASS();
}

void test_updatePosition_no_equals_no_crash() {
    char msg[] = "noequalssign";
    g_s->updatePosition(msg);
    TEST_PASS();
}

// ── getPassword / setPassword – round-trip via improved Preferences mock ──

void test_getPassword_default_is_empty() {
    TEST_ASSERT_EQUAL_STRING("", g_s->getPassword().c_str());
}

void test_setPassword_getPassword_round_trip() {
    g_s->setPassword("mysecret");
    TEST_ASSERT_EQUAL_STRING("mysecret", g_s->getPassword().c_str());
}

void test_setPassword_to_empty_clears_password() {
    g_s->setPassword("mysecret");
    g_s->setPassword("");
    TEST_ASSERT_EQUAL_STRING("", g_s->getPassword().c_str());
}

// ── getDeviceName / setDeviceName ─────────────────────────────────────────

void test_getDeviceName_returns_KinetiX_by_default() {
    TEST_ASSERT_EQUAL_STRING("KinetiX", g_s->getDeviceName().c_str());
}

void test_setDeviceName_getDeviceName_round_trip() {
    g_s->setDeviceName("MyRoboHand");
    TEST_ASSERT_EQUAL_STRING("MyRoboHand", g_s->getDeviceName().c_str());
}

// ─────────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_getInt_returns_default_when_key_not_set);
    RUN_TEST(test_getInt_returns_zero_default);

    RUN_TEST(test_setInt_no_crash);
    RUN_TEST(test_setString_no_crash);
    RUN_TEST(test_setBool_no_crash);

    RUN_TEST(test_getString_returns_default_when_key_not_set);
    RUN_TEST(test_getString_returns_null_default_as_null);

    RUN_TEST(test_getBool_returns_false_default);
    RUN_TEST(test_getBool_returns_true_default);

    RUN_TEST(test_getPosition_returns_default_when_key_not_set);

    RUN_TEST(test_getSettingJson_returns_json_string);
    RUN_TEST(test_getPositionsJson_returns_json_string);

    RUN_TEST(test_updateSetting_integer_type_no_crash);
    RUN_TEST(test_updateSetting_string_type_no_crash);
    RUN_TEST(test_updateSetting_boolean_true_no_crash);
    RUN_TEST(test_updateSetting_boolean_false_no_crash);
    RUN_TEST(test_updateSetting_boolean_one_no_crash);
    RUN_TEST(test_updateSetting_unknown_prefix_no_crash);
    RUN_TEST(test_updateSetting_no_equals_no_crash);

    RUN_TEST(test_updatePosition_key_value_no_crash);
    RUN_TEST(test_updatePosition_reset_no_crash);
    RUN_TEST(test_updatePosition_no_equals_no_crash);

    RUN_TEST(test_getPassword_default_is_empty);
    RUN_TEST(test_setPassword_getPassword_round_trip);
    RUN_TEST(test_setPassword_to_empty_clears_password);

    RUN_TEST(test_getDeviceName_returns_KinetiX_by_default);
    RUN_TEST(test_setDeviceName_getDeviceName_round_trip);

    return UNITY_END();
}

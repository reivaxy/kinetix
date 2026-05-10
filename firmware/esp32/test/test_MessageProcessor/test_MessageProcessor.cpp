// Unit tests for MessageProcessor – runs on native platform via Unity
#include <unity.h>
#include "MessageProcessor.h"
#include "BtServer.h"

uint32_t mock_millis_value = 0;
int mock_analog_read_value = 0;

// ── Spy display that records the last setLine call ────────────────────────
class SpyDisplay : public Display {
public:
    int         lastLineIdx  = -1;
    std::string lastLineText;

    void setTitle(String /*t*/)                override {}
    void setLine(int line, String content)     override {
        lastLineIdx  = line;
        lastLineText = content.c_str();
    }
    void refresh()                             override {}
};

// ── Shared fixtures ───────────────────────────────────────────────────────
static Settings*          g_settings = nullptr;
static Hand*              g_hand     = nullptr;
static SpyDisplay*        g_display  = nullptr;
static MessageProcessor*  g_mp       = nullptr;

void setUp() {
    mock_millis_value = 0;
    g_settings = new Settings();
    g_hand     = new Hand(g_settings);
    g_display  = new SpyDisplay();
    JsonDocument doc;
    g_mp = new MessageProcessor(g_hand, g_settings, g_display, doc);
}

void tearDown() {
    delete g_mp;     g_mp      = nullptr;
    delete g_display; g_display = nullptr;
    delete g_hand;   g_hand    = nullptr;
    delete g_settings; g_settings = nullptr;
}

// ── isIdle() ──────────────────────────────────────────────────────────────

void test_isIdle_initially_true() {
    TEST_ASSERT_TRUE(g_mp->isIdle());
}

// ── Named movements ───────────────────────────────────────────────────────

void test_named_movement_sets_handMovement() {
    char msg[] = "fist";
    g_mp->processWriteMsg(movement, msg);
    TEST_ASSERT_NOT_NULL(g_mp->handMovement);
}

void test_named_movement_not_idle() {
    char msg[] = "fist";
    g_mp->processWriteMsg(movement, msg);
    // Servo is mocked so fingers haven't physically moved → hand is not still
    TEST_ASSERT_FALSE(g_mp->isIdle());
}

void test_all_known_movements_resolve() {
    const char* names[] = {
        "fist", "ok", "one", "two", "three", "four", "five",
        "closePinch", "openPinch", "idle", "pointing", "fu",
        "rock", "love"
    };
    for (const char* name : names) {
        char buf[32];
        strncpy(buf, name, sizeof(buf));
        g_mp->processWriteMsg(movement, buf);
        TEST_ASSERT_NOT_NULL_MESSAGE(g_mp->handMovement, name);
    }
}

void test_unknown_movement_leaves_handMovement_null() {
    char msg[] = "doesnotexist";
    g_mp->processWriteMsg(movement, msg);
    TEST_ASSERT_NULL(g_mp->handMovement);
}

void test_unknown_movement_stays_idle() {
    char msg[] = "doesnotexist";
    g_mp->processWriteMsg(movement, msg);
    TEST_ASSERT_TRUE(g_mp->isIdle());
}

// ── Movement display line ─────────────────────────────────────────────────

void test_movement_sets_display_line() {
    char msg[] = "fist";
    g_mp->processWriteMsg(movement, msg);
    TEST_ASSERT_EQUAL_INT(MOVEMENT_DISPLAY_LINE, g_display->lastLineIdx);
    TEST_ASSERT_EQUAL_STRING("Movement: fist", g_display->lastLineText.c_str());
}

// ── Single-finger open / close commands ──────────────────────────────────

void test_single_finger_open_no_crash() {
    char msg[] = "o2";
    g_mp->processWriteMsg(movement, msg);
    TEST_PASS(); // reaching here means no crash
}

void test_single_finger_close_no_crash() {
    char msg[] = "c0";
    g_mp->processWriteMsg(movement, msg);
    TEST_PASS();
}

void test_single_finger_leaves_idle() {
    // Single-finger commands do not assign handMovement → isIdle() == true
    char msg[] = "o1";
    g_mp->processWriteMsg(movement, msg);
    TEST_ASSERT_TRUE(g_mp->isIdle());
}

// ── Sequences triggered by special movement names ────────────────────────

void test_scratch_creates_running_sequence() {
    char msg[] = "scratch";
    g_mp->processWriteMsg(movement, msg);
    TEST_ASSERT_NOT_NULL(g_mp->seq);
    TEST_ASSERT_TRUE(g_mp->seq->isRunning());
}

void test_come_creates_running_sequence() {
    char msg[] = "come";
    g_mp->processWriteMsg(movement, msg);
    TEST_ASSERT_NOT_NULL(g_mp->seq);
    TEST_ASSERT_TRUE(g_mp->seq->isRunning());
}

// ── Settings / Positions write ────────────────────────────────────────────

void test_setting_write_no_crash() {
    char msg[] = "i_foo=42";
    g_mp->processWriteMsg(setting, msg);
    TEST_PASS();
}

void test_positions_write_no_crash() {
    char msg[] = "thumb_max_open=170";
    g_mp->processWriteMsg(positions, msg);
    TEST_PASS();
}

// ── systemConfig write ────────────────────────────────────────────────────

void test_systemconfig_device_name_no_crash() {
    char msg[] = "s_deviceName=TestHand";
    g_mp->processWriteMsg(systemConfig, msg);
    TEST_PASS();
}

void test_systemconfig_empty_device_name_ignored() {
    char msg[] = "s_deviceName=";
    g_mp->processWriteMsg(systemConfig, msg);
    TEST_PASS();
}

void test_systemconfig_no_equals_no_crash() {
    char msg[] = "invalid";
    g_mp->processWriteMsg(systemConfig, msg);
    TEST_PASS();
}

// ── processReadMsg ────────────────────────────────────────────────────────

void test_readmsg_password_unauthenticated_without_btserver() {
    // btServer is nullptr → response must be "false"
    BLECharacteristic ch;
    g_mp->processReadMsg(password, &ch);
    TEST_ASSERT_EQUAL_STRING("false", ch.getValue().c_str());
}

void test_readmsg_setting_returns_json() {
    BLECharacteristic ch;
    g_mp->processReadMsg(setting, &ch);
    // Stub serializeJson always returns "{}"
    TEST_ASSERT_EQUAL_STRING("{}", ch.getValue().c_str());
}

void test_readmsg_positions_returns_json() {
    BLECharacteristic ch;
    g_mp->processReadMsg(positions, &ch);
    TEST_ASSERT_EQUAL_STRING("{}", ch.getValue().c_str());
}

// ── password write (empty stored password == empty pwdCheck → Auth OK) ───

void test_password_write_empty_stored_pwd_auth_ok() {
    // Preferences stub always returns "" for stored password.
    // ArduinoJson stub always returns "" for pwdCheck field.
    // strcmp("", "") == 0 → Auth: OK
    char msg[] = "{\"pwdCheck\":\"\"}";
    g_mp->processWriteMsg(password, msg);
    TEST_ASSERT_EQUAL_STRING("Auth: OK", g_display->lastLineText.c_str());
}

// ── password write – wrong stored password → Auth FAIL ───────────────────

void test_password_write_wrong_stored_password_shows_fail() {
    // Store a non-empty password; ArduinoJson stub always yields pwdCheck=""
    // strcmp("", "secret") != 0 → Auth: FAIL
    g_settings->setPassword("secret");
    char msg[] = "{\"pwdCheck\":\"wrong\"}";
    g_mp->processWriteMsg(password, msg);
    TEST_ASSERT_EQUAL_STRING("Auth: FAIL", g_display->lastLineText.c_str());
}

// ── setBtServer + authentication state ───────────────────────────────────

void test_btserver_set_auth_ok_sets_client_authenticated() {
    BtServer btServer(nullptr, nullptr);
    g_mp->setBtServer(&btServer);

    // Empty stored password, pwdCheck="" (stub) → match → Auth: OK
    char msg[] = "{\"pwdCheck\":\"\"}";
    g_mp->processWriteMsg(password, msg);

    TEST_ASSERT_TRUE(btServer.isClientAuthenticated());
    g_mp->setBtServer(nullptr); // cleanup
}

void test_readmsg_password_authenticated_returns_true() {
    BtServer btServer(nullptr, nullptr);
    g_mp->setBtServer(&btServer);

    // Authenticate
    char msg[] = "{\"pwdCheck\":\"\"}";
    g_mp->processWriteMsg(password, msg);

    // Now read password status
    BLECharacteristic ch;
    g_mp->processReadMsg(password, &ch);
    TEST_ASSERT_EQUAL_STRING("true", ch.getValue().c_str());
    g_mp->setBtServer(nullptr);
}

// ── processReadMsg – systemConfig ─────────────────────────────────────────

void test_readmsg_systemconfig_returns_json() {
    BLECharacteristic ch;
    g_mp->processReadMsg(systemConfig, &ch);
    // serializeJson stub always produces "{}" – just confirm no crash and value set
    TEST_ASSERT_EQUAL_STRING("{}", ch.getValue().c_str());
}

// ── calibration and demo sequences ───────────────────────────────────────

void test_calibration_movement_creates_running_sequence() {
    char msg[] = "calibration";
    g_mp->processWriteMsg(movement, msg);
    TEST_ASSERT_NOT_NULL(g_mp->seq);
    TEST_ASSERT_TRUE(g_mp->seq->isRunning());
}

void test_demo_movement_creates_running_sequence() {
    char msg[] = "demo";
    g_mp->processWriteMsg(movement, msg);
    TEST_ASSERT_NOT_NULL(g_mp->seq);
    TEST_ASSERT_TRUE(g_mp->seq->isRunning());
}

// ── Second movement command cleans up the first ───────────────────────────

void test_second_movement_replaces_first() {
    char msg1[] = "fist";
    g_mp->processWriteMsg(movement, msg1);
    HandMovement* first = g_mp->handMovement;
    TEST_ASSERT_NOT_NULL(first);

    char msg2[] = "five";
    g_mp->processWriteMsg(movement, msg2);
    // A new handMovement must have been created (old one was deleted)
    TEST_ASSERT_NOT_NULL(g_mp->handMovement);
}

void test_second_sequence_command_replaces_first() {
    char msg1[] = "scratch";
    g_mp->processWriteMsg(movement, msg1);
    TEST_ASSERT_NOT_NULL(g_mp->seq);

    char msg2[] = "come";
    g_mp->processWriteMsg(movement, msg2);
    TEST_ASSERT_NOT_NULL(g_mp->seq);
    TEST_ASSERT_TRUE(g_mp->seq->isRunning());
}

// ── run() ────────────────────────────────────────────────────────────────

void test_run_without_sequence_does_not_crash() {
    // seq is NULL; hand->run() must complete without crash
    g_mp->run();
    TEST_PASS();
}

void test_run_with_active_sequence_does_not_crash() {
    char msg[] = "scratch";
    g_mp->processWriteMsg(movement, msg); // creates seq
    g_mp->run();
    TEST_PASS();
}

// ── Single-finger movements – all 5 fingers each direction ───────────────

void test_open_all_fingers_no_crash() {
    for (char i = '0'; i <= '4'; i++) {
        char msg[] = "oX";
        msg[1] = i;
        g_mp->processWriteMsg(movement, msg);
    }
    TEST_PASS();
}

void test_close_all_fingers_no_crash() {
    for (char i = '0'; i <= '4'; i++) {
        char msg[] = "cX";
        msg[1] = i;
        g_mp->processWriteMsg(movement, msg);
    }
    TEST_PASS();
}

// ── Unknown write type falls through to default ───────────────────────────

void test_unknown_write_type_does_not_crash() {
    char msg[] = "payload";
    g_mp->processWriteMsg(ota, msg); // ota has no write handler → default branch
    TEST_PASS();
}

// ─────────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_isIdle_initially_true);

    RUN_TEST(test_named_movement_sets_handMovement);
    RUN_TEST(test_named_movement_not_idle);
    RUN_TEST(test_all_known_movements_resolve);
    RUN_TEST(test_unknown_movement_leaves_handMovement_null);
    RUN_TEST(test_unknown_movement_stays_idle);

    RUN_TEST(test_movement_sets_display_line);

    RUN_TEST(test_single_finger_open_no_crash);
    RUN_TEST(test_single_finger_close_no_crash);
    RUN_TEST(test_single_finger_leaves_idle);

    RUN_TEST(test_scratch_creates_running_sequence);
    RUN_TEST(test_come_creates_running_sequence);
    RUN_TEST(test_calibration_movement_creates_running_sequence);
    RUN_TEST(test_demo_movement_creates_running_sequence);

    RUN_TEST(test_second_movement_replaces_first);
    RUN_TEST(test_second_sequence_command_replaces_first);

    RUN_TEST(test_setting_write_no_crash);
    RUN_TEST(test_positions_write_no_crash);

    RUN_TEST(test_systemconfig_device_name_no_crash);
    RUN_TEST(test_systemconfig_empty_device_name_ignored);
    RUN_TEST(test_systemconfig_no_equals_no_crash);

    RUN_TEST(test_readmsg_password_unauthenticated_without_btserver);
    RUN_TEST(test_readmsg_setting_returns_json);
    RUN_TEST(test_readmsg_positions_returns_json);
    RUN_TEST(test_readmsg_systemconfig_returns_json);

    RUN_TEST(test_password_write_empty_stored_pwd_auth_ok);
    RUN_TEST(test_password_write_wrong_stored_password_shows_fail);
    RUN_TEST(test_btserver_set_auth_ok_sets_client_authenticated);
    RUN_TEST(test_readmsg_password_authenticated_returns_true);

    RUN_TEST(test_run_without_sequence_does_not_crash);
    RUN_TEST(test_run_with_active_sequence_does_not_crash);

    RUN_TEST(test_open_all_fingers_no_crash);
    RUN_TEST(test_close_all_fingers_no_crash);

    RUN_TEST(test_unknown_write_type_does_not_crash);

    return UNITY_END();
}

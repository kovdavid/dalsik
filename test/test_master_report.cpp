#include "acutest.h"
#include "dalsik.h"
#include "keymap.h"
#include "master_report.h"
#include "key_definitions.h"
#include "HID.h"
#include "Serial.h"
#include "avr/eeprom.h"

bool compare_base_report(size_t index, BaseHIDReport expected) {
    BaseHIDReport got = HID().base_hid_reports.at(index);
    return memcmp(&got, &expected, sizeof(BaseHIDReport)) == 0;
}

// Base REPort COMPare
#define BREP_COMP(i, ...) TEST_CHECK(compare_base_report(i, __VA_ARGS__))

#define RESET_TEST(msg) \
    HID().base_hid_reports.clear(); \
    keymap = KeyMap(); \
    master_report = MasterReport(&keymap);

#define HID_SIZE_CHECK(expected) \
    TEST_CHECK(HID().base_hid_reports.size() == expected); \
    TEST_MSG("size: %lu expected:%d", HID().base_hid_reports.size(), expected)

void print_hid_reports() {
    TEST_MSG("size: %lu", HID().base_hid_reports.size());
    for (uint8_t i = 0; i < HID().base_hid_reports.size(); i++ ) {
        char buffer[10];
        sprintf(buffer, "KEY %d", i);
        TEST_DUMP(buffer, &(HID().base_hid_reports.at(i)), sizeof(BaseHIDReport));
    }
}

void dump_hid_reports() {
    TEST_CHECK(1 == 0);
    print_hid_reports();
}

#define SEC(x) x*1000
#define P EVENT_KEY_PRESS
#define R EVENT_KEY_RELEASE

// Synchronize with test/mocks/mock_eeprom.cpp!
KeyCoords normal_KC_A = { 1, 1 };
KeyCoords normal_KC_B = { 1, 2 };
KeyCoords dual_ctrl_KC_C = { 1, 3 };
KeyCoords dual_shift_KC_D = { 1, 4 };

KeyCoords dual_layer_1 = { 1, 7 };
KeyCoords solo_dual_layer_1 = { 1, 8 };
KeyCoords one_shot_ctrl = { 1, 9 };

// Simple press test with short delay between events
void test_normal_key_1(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    master_report.handle_master_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });

    master_report.handle_master_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    now += SEC(10);

    master_report.handle_master_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(2, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Simple press test with long delay between events
void test_normal_key_2(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    now += SEC(10);

    master_report.handle_master_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Multiple normal keys pressed at once
void test_normal_key_3(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    master_report.handle_master_changed_key({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(2);

    master_report.handle_master_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    master_report.handle_master_changed_key({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, KC_A, KC_B, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Dual mod key quick tap
// After releasing the key, we get KC_C
// After pressing the key, we don't send anything yet, as we don't know
// if we will send the primary or secondary key yet
void test_dual_mod_key_1(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Dual mod key long press and release (timeout check)
// After DUAL_MODE_TIMEOUT_MS ellapses from the pressing of the key, we
// activate KC_C
void test_dual_mod_key_2(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    // Not yet...
    now += DUAL_MODE_TIMEOUT_MS - 1;
    master_report.key_timeout_check(now);
    HID_SIZE_CHECK(0);

    // Fire KC_C
    now++;
    master_report.key_timeout_check(now);
    HID_SIZE_CHECK(1);

    master_report.handle_master_changed_key({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test dual key with a normal key as well
// The press of a normal key should trigger CTRL+B, as at the time the
// dual key is still in STATE_PENDING
void test_dual_mod_key_3(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ P, normal_KC_B    }, now++);
    HID_SIZE_CHECK(2);

    master_report.handle_master_changed_key({ R, normal_KC_B    }, now++);
    HID_SIZE_CHECK(3);

    master_report.handle_master_changed_key({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x01, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test dual key with a normal key, but with a long delay
// The delay should trigger the primary key of the dual key, therefor it won't
// function as a modifier (CTRL) anymore. We expect to send K+B
void test_dual_mod_key_4(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    now += DUAL_MODE_TIMEOUT_MS;
    master_report.key_timeout_check(now);
    HID_SIZE_CHECK(1);

    master_report.handle_master_changed_key({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(2);

    master_report.handle_master_changed_key({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(3);

    master_report.handle_master_changed_key({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(0, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, KC_C, KC_B, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Dual mod key with a normal key, but we release the dual key first.
// Should still send CTRL+B
void test_dual_mod_key_5(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ P, normal_KC_B    }, now++);
    HID_SIZE_CHECK(2);

    master_report.handle_master_changed_key({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(3);

    master_report.handle_master_changed_key({ R, normal_KC_B    }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x01, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Multiple dual keys without timeout
// After pressing the second dual key, we trigger the modifier on the first.
// After pressing the normal key, we trigger the modifier on the second.
// We expect CTRL+SHIFT+B
void test_dual_mod_key_6(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    master_report.handle_master_changed_key({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(3);

    master_report.handle_master_changed_key({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    master_report.handle_master_changed_key({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(5);

    master_report.handle_master_changed_key({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(6);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x03, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(4, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(5, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Multiple dual keys with timeout
// After pressing the second dual key, we trigger the modifier on the first.
// After a long delay, we trigger the primary key on the second dual key,
// sending CTRL+D. Then we press a normal key, which should send CTRL+B
// We expect CTRL+SHIFT+B
void test_dual_mod_key_7(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    now += DUAL_MODE_TIMEOUT_MS;
    master_report.key_timeout_check(now);
    HID_SIZE_CHECK(2);

    master_report.handle_master_changed_key({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(3);

    master_report.handle_master_changed_key({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    master_report.handle_master_changed_key({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(5);

    master_report.handle_master_changed_key({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(6);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x01, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x01, 0x00, KC_D, KC_B, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x01, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(4, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(5, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Press only dual mod keys
// After pressing the second dual key, we trigger the modifier on the first one
// and leave the second one in STATE_PENDING. After releasing the second one
// we trigger the primary key on it
void test_dual_mod_key_8(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    master_report.handle_master_changed_key({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(3);

    master_report.handle_master_changed_key({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x01, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

void test_dual_mod_key_9(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    master_report.handle_master_changed_key({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    master_report.handle_master_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    master_report.handle_master_changed_key({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When tapping the dual layer key, the secondary key should trigger
void test_dual_layer_key_1(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ R, dual_layer_1 }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_G, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a different key after the dual layer key, we should
// move to given layer. So for the key `normal_KC_A` we want to trigger
// KC_E, as it is defined on layer 1
void test_dual_layer_key_2(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a solo dual layer key as the first key, it should trigger
// a new layer
void test_dual_layer_key_3(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, solo_dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a solo dual layer key NOT as the first key, it should trigger
// the secondary key instead of layer press immediately
void test_dual_layer_key_4(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    master_report.handle_master_changed_key({ P, solo_dual_layer_1 }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, KC_A, KC_H, 0x00, 0x00, 0x00, 0x00 });
}

// There was an issue with the `key_index` value in the PressedKeys
// structure after calling `remove_from_pressed_keys` - the value was not updated.
// That caused in this test to the dual_shift_KC_D key to be stuck even after
// releasing it.
void test_stuck_key(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    master_report.handle_master_changed_key({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(2);

    master_report.handle_master_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    master_report.handle_master_changed_key({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(3);

    master_report.handle_master_changed_key({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    master_report.handle_master_changed_key({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(6);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, KC_A, KC_B, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(4, { 0x00, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(5, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that the one-shot CTRL modifier is sent with the KC_A key
void test_one_shot_modifier(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x01, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that after the second press of the one-shot modifier the CTRL modifier
// is toggled off, so when pressing KC_A, it is not sent
void test_one_shot_modifier_toggle(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When a one-shot modifier is held with a different key, it is registered
// as a normal modifier - i.e. with the second press of KC_A it is not sent
// any more
void test_one_shot_modifier_hold(void) {
    KeyMap keymap;
    MasterReport master_report(&keymap);

    millisec now = 100;

    master_report.handle_master_changed_key({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(0);

    master_report.handle_master_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    master_report.handle_master_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    master_report.handle_master_changed_key({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(3);

    master_report.handle_master_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(4);

    master_report.handle_master_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(5);

    BREP_COMP(0, { 0x01, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(4, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

TEST_LIST = {
    { "test_normal_key_1", test_normal_key_1 },
    { "test_normal_key_2", test_normal_key_2 },
    { "test_normal_key_3", test_normal_key_3 },
    { "test_dual_mod_key_1", test_dual_mod_key_1 },
    { "test_dual_mod_key_2", test_dual_mod_key_2 },
    { "test_dual_mod_key_3", test_dual_mod_key_3 },
    { "test_dual_mod_key_4", test_dual_mod_key_4 },
    { "test_dual_mod_key_5", test_dual_mod_key_5 },
    { "test_dual_mod_key_6", test_dual_mod_key_6 },
    { "test_dual_mod_key_7", test_dual_mod_key_7 },
    { "test_dual_mod_key_8", test_dual_mod_key_8 },
    { "test_dual_mod_key_9", test_dual_mod_key_9 },
    { "test_dual_layer_key_1", test_dual_layer_key_1 },
    { "test_dual_layer_key_2", test_dual_layer_key_2 },
    { "test_dual_layer_key_3", test_dual_layer_key_3 },
    { "test_dual_layer_key_4", test_dual_layer_key_4 },
    { "test_stuck_key", test_stuck_key },
    { "test_one_shot_modifier", test_one_shot_modifier },
    { "test_one_shot_modifier_toggle", test_one_shot_modifier_toggle },
    { "test_one_shot_modifier_hold", test_one_shot_modifier_hold },
    { NULL, NULL }
};

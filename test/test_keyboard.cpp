#include "acutest.h"
#include "dalsik.h"
#include "keyboard.h"
#include "keymap.h"
#include "key_definitions.h"
#include "HID.h"
#include "Serial.h"
#include "avr/eeprom.h"

PressedKeys get_keyboard_pressed_keys(Keyboard k) {
    return k.pressed_keys;
}

bool compare_base_report(size_t index, BaseHIDReport expected) {
    BaseHIDReport got = HID().base_hid_reports.at(index);
    return memcmp(&got, &expected, sizeof(BaseHIDReport)) == 0;
}

// Base REPort COMPare
#define BREP_COMP(i, ...) TEST_CHECK(compare_base_report(i, __VA_ARGS__))

#define RESET_TEST(msg) \
    HID().base_hid_reports.clear(); \
    keyboard = Keyboard();

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

// Synchronize with test/mocks/mock_keymap.cpp!
KeyCoords kc_ctrl = { 1, 0 };
KeyCoords normal_KC_A = { 1, 1 };
KeyCoords normal_KC_B = { 1, 2 };
KeyCoords dual_ctrl_KC_C = { 1, 3 };
KeyCoords dual_shift_KC_D = { 1, 4 };

KeyCoords dual_layer_1 = { 1, 7 };
KeyCoords solo_dual_layer_1 = { 1, 8 };
KeyCoords one_shot_ctrl = { 1, 9 };
KeyCoords layer_hold_or_toggle = { 1, 10 };

KeyCoords layer_press_1 = { 1, 11 };

// Simple press test with short delay between events
void test_normal_key_1(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    now += SEC(10);

    keyboard.handle_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(2, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Simple press test with long delay between events
void test_normal_key_2(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    now += SEC(10);

    keyboard.handle_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Multiple normal keys pressed at once
void test_normal_key_3(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_changed_key({ R, normal_KC_B }, now++);
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
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Dual mod key long press and release (timeout check)
// After DUAL_MODE_TIMEOUT_MS ellapses from the pressing of the key, we
// activate KC_C
void test_dual_mod_key_2(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    // Not yet...
    now += DUAL_MODE_TIMEOUT_MS - 1;
    keyboard.key_timeout_check(now);
    HID_SIZE_CHECK(0);

    // Fire KC_C
    now++;
    keyboard.key_timeout_check(now);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test dual key with a normal key as well
// The press of a normal key should trigger CTRL+B, as at the time the
// dual key is still in STATE_PENDING
void test_dual_mod_key_3(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ P, normal_KC_B    }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_changed_key({ R, normal_KC_B    }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_changed_key({ R, dual_ctrl_KC_C }, now++);
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
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    now += DUAL_MODE_TIMEOUT_MS;
    keyboard.key_timeout_check(now);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_changed_key({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_changed_key({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(0, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, KC_C, KC_B, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Dual mod key with a normal key, but we release the dual key first.
// Should still send CTRL+B
void test_dual_mod_key_5(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ P, normal_KC_B    }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_changed_key({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_changed_key({ R, normal_KC_B    }, now++);
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
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_changed_key({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    keyboard.handle_changed_key({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(5);

    keyboard.handle_changed_key({ R, dual_ctrl_KC_C }, now++);
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
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    now += DUAL_MODE_TIMEOUT_MS;
    keyboard.key_timeout_check(now);
    HID_SIZE_CHECK(2);

    keyboard.handle_changed_key({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_changed_key({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    keyboard.handle_changed_key({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(5);

    keyboard.handle_changed_key({ R, dual_ctrl_KC_C }, now++);
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
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_changed_key({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x01, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

void test_dual_mod_key_9(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_changed_key({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When tapping the dual layer key, the secondary key should trigger
void test_dual_layer_key_1(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ R, dual_layer_1 }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_G, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a different key after the dual layer key, we should
// move to given layer. So for the key `normal_KC_A` we want to trigger
// KC_E, as it is defined on layer 1
void test_dual_layer_key_2(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a solo dual layer key as the first key, it should trigger
// a new layer
void test_dual_layer_key_3(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, solo_dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a solo dual layer key NOT as the first key, it should trigger
// the secondary key instead of layer press immediately
void test_dual_layer_key_4(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ P, solo_dual_layer_1 }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, KC_A, KC_H, 0x00, 0x00, 0x00, 0x00 });
}

// There was an issue with the `key_index` value in the PressedKeys
// structure after calling `remove_from_pressed_keys` - the value was not updated.
// That caused in this test to the dual_shift_KC_D key to be stuck even after
// releasing it.
void test_stuck_key(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_changed_key({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_changed_key({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    keyboard.handle_changed_key({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(6);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, KC_A, KC_B, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(4, { 0x00, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(5, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that the one-shot CTRL modifier is sent after tapping the one shot key
void test_one_shot_modifier(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x01, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that after the second press of the one-shot modifier the CTRL modifier
// is toggled off, so when pressing KC_A, it is not sent
void test_one_shot_modifier_toggle(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When a one-shot modifier is held with a different key, it is registered
// as a normal modifier - i.e. with the second press of KC_A it is not sent
// any more
void test_one_shot_modifier_hold(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_changed_key({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(4);

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(5);

    keyboard.handle_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(6);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x01, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(4, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(5, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that the layer switch is applied and we send KC_E (layer1)
void test_layer_hold_or_toggle(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ R, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that the second tap on the key toggles the layer off, so we send KC_A
void test_layer_hold_or_toggle_on_off(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ R, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ P, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ R, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When a LHT key is held while a different key is pressed, the LHT should
// register as a normal layer key. That is with the first KC_A press we send
// KC_E (layer 1) and with the second KC_A (layer 0)
void test_layer_hold_or_toggle_hold(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_changed_key({ R, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_changed_key({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that pressing an LP key changes the layer, so instead of KC_A we get
// KC_E from layer 1
void test_layer_press_1(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, layer_press_1 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that releasing the LP key changes the layer back, so we should get KC_A
void test_layer_press_2(void) {
    Keyboard keyboard;

    millisec now = 100;

    keyboard.handle_changed_key({ P, layer_press_1 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ R, layer_press_1 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test clearing of the pressed_keys structure. We first fill it up, Then
// remove all but one element (so that keyboard.clear() is not triggered).
void test_remove_from_pressed_keys(void) {
    Keyboard keyboard;

    millisec now = 100;

    for (uint8_t i = 0; i < PRESSED_KEY_BUFFER; i++) {
        keyboard.handle_changed_key({ P, normal_KC_A }, now++);
    }

    for (uint8_t i = 0; i < PRESSED_KEY_BUFFER - 1; i++) {
        keyboard.handle_changed_key({ R, normal_KC_A }, now++);
    }

    PressedKeys pressed_keys = get_keyboard_pressed_keys(keyboard);

    // The first key is still processed
    PressedKey pressed_key = pressed_keys.keys[0];
    TEST_CHECK(pressed_key.timestamp != 0);
    TEST_MSG("index:0 timestamp:%d expected 0", pressed_key.timestamp);
    TEST_CHECK(pressed_key.state == STATE_ACTIVE_KEY);
    TEST_MSG("index:0 state:%d expected 0", pressed_key.state);

    // The rest should be zeroed out
    for (uint8_t i = 1; i < PRESSED_KEY_BUFFER; i++) {
        PressedKey pressed_key = pressed_keys.keys[i];
        TEST_CHECK(pressed_key.timestamp == 0);
        TEST_MSG("index:%d timestamp:%d expected 0", i, pressed_key.timestamp);
        TEST_CHECK(pressed_key.state == STATE_NOT_PROCESSED);
        TEST_MSG("index:%d state:%d expected 0", i, pressed_key.state);
    }

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
    { "test_layer_hold_or_toggle", test_layer_hold_or_toggle },
    { "test_layer_hold_or_toggle_on_off", test_layer_hold_or_toggle_on_off },
    { "test_layer_hold_or_toggle_hold", test_layer_hold_or_toggle_hold },
    { "test_layer_press_1", test_layer_press_1 },
    { "test_layer_press_2", test_layer_press_2 },
    { "test_remove_from_pressed_keys", test_remove_from_pressed_keys },
    { NULL, NULL }
};

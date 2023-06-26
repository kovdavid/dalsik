#include "HID.h"
#include "Serial.h"

#include "acutest.h"
#include "avr/eeprom.h"

#include "dalsik.h"
#include "array_utils.h"
#include "combo.h"
#include "combo_handler.h"
#include "key_definitions.h"
#include "extended_key_event.h"
#include "key_event_handler.h"
#include "keyboard.h"
#include "keymap.h"
#include "tapdance.h"
#include "test_friend.h"

#define REPORT_COMPARE_AT(i, ...) TEST_CHECK(compare_keyboard_report(i, __VA_ARGS__))

#define HID_SIZE_CHECK(expected) \
    TEST_CHECK(HID().keyboard_reports.size() == expected); \
    TEST_MSG("size: %lu expected:%d", HID().keyboard_reports.size(), expected)

bool compare_keyboard_report(size_t index, KeyboardHIDReport expected) {
    KeyboardHIDReport got = HID().keyboard_reports.at(index);
    return memcmp(&got, &expected, sizeof(KeyboardHIDReport)) == 0;
}

void print_hid_reports() {
    TEST_MSG("size: %lu", HID().keyboard_reports.size());
    for (uint8_t i = 0; i < HID().keyboard_reports.size(); i++ ) {
        char buffer[10];
        sprintf(buffer, "KEY %d", i);
        TEST_DUMP(buffer, &(HID().keyboard_reports.at(i)), sizeof(KeyboardHIDReport));
    }
}

void print_combo_states() {
    printf("\nCombos:\n");
    for (uint8_t i = 0; i < combos_count; i++) {
        Combo* combo = COMBO_AT(i);
        combo->print_internal_state(i);
    }
}

void dump_hid_reports() {
    TEST_CHECK(1 == 0);
    print_hid_reports();
}

TestFriendClass test_friend;

#define SEC(x) x*1000
#define P EVENT_KEY_PRESS
#define R EVENT_KEY_RELEASE
#define T EVENT_TIMEOUT

#define PRESS(r,c) BaseKeyEvent { EVENT_KEY_PRESS, KeyCoords { r, c } }
#define RELEASE(r,c) BaseKeyEvent { EVENT_KEY_RELEASE, KeyCoords { r, c } }

// Synchronize with test/mocks/mock_keymap.cpp!
KeyCoords kc_no = { 0, 0 };
KeyCoords kc_ctrl = { 1, 0 };
KeyCoords normal_KC_A = { 1, 1 };
KeyCoords normal_KC_B = { 1, 2 };
KeyCoords dual_ctrl_KC_C = { 1, 3 };
KeyCoords dual_shift_KC_D = { 1, 4 };
KeyCoords normal_KC_1 = { 2, 3 };
KeyCoords normal_KC_SPACE = { 2, 4 };

KeyCoords dual_layer_1 = { 1, 7 };
KeyCoords dual_layer_2 = { 2, 0 };
KeyCoords dual_layer_3 = { 2, 1 };
KeyCoords solo_dual_layer_1 = { 1, 8 };
KeyCoords one_shot_ctrl = { 1, 9 };
KeyCoords layer_hold_or_toggle = { 1, 10 };

KeyCoords layer_press_1 = { 1, 11 };

KeyCoords caps_word = { 2, 2 };

KeyCoords dth_ctrl_j = { 2, 5 };

KeyCoords td1 = { 2, 6 };

void test_array_utils1(void) {
    uint8_t array[] = { 1, 2, 3, 0 };

    uint8_t result = ArrayUtils::remove_and_return_last_uint8(array, sizeof(array), 2);
    TEST_CHECK(result == 3);
    TEST_CHECK(array[0] == 1);
    TEST_CHECK(array[1] == 3);
    TEST_CHECK(array[2] == 0);
    TEST_CHECK(array[3] == 0);

    result = ArrayUtils::remove_and_return_last_uint8(array, sizeof(array), 1);
    TEST_CHECK(result == 3);
    TEST_CHECK(array[0] == 3);
    TEST_CHECK(array[1] == 0);
    TEST_CHECK(array[2] == 0);
    TEST_CHECK(array[3] == 0);

    result = ArrayUtils::remove_and_return_last_uint8(array, sizeof(array), 3);
    TEST_CHECK(result == 0);
    TEST_CHECK(array[0] == 0);
    TEST_CHECK(array[1] == 0);
    TEST_CHECK(array[2] == 0);
    TEST_CHECK(array[3] == 0);
}

// Simple press test with short delay between events
void test_normal_key_1(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Simple press test with long delay between events
void test_normal_key_2(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    now += SEC(10);

    key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Multiple normal keys pressed at once
void test_normal_key_3(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    key_event_handler.handle_key_event({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, KC_A, KC_B, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x00, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Dual mod key quick tap
// After releasing the key, we get KC_C
// After pressing the key, we don't send anything yet, as we don't know
// if we will send the primary or secondary key yet
void test_dual_mod_key_1(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Dual mod key long press and release (timeout check)
// After DUAL_MODE_TIMEOUT_MS ellapses from the pressing of the key, we
// activate KC_C
void test_dual_mod_key_2(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    // Not yet...
    now += DUAL_MODE_TIMEOUT_MS - 1;
    key_event_handler.handle_key_event({ T }, now);
    HID_SIZE_CHECK(0);

    // Fire KC_C
    now++;
    key_event_handler.handle_key_event({ T }, now);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test dual key with a normal key as well
// The press of a normal key should trigger CTRL+B, as at the time the
// dual key is still in STATE_PENDING
void test_dual_mod_key_3(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, normal_KC_B    }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ R, normal_KC_B    }, now++);
    HID_SIZE_CHECK(3);

    key_event_handler.handle_key_event({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(4);

    REPORT_COMPARE_AT(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x01, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test dual key with a normal key, but with a long delay
// The delay should trigger the primary key of the dual key, therefor it won't
// function as a modifier (CTRL) anymore. We expect to send K+B
void test_dual_mod_key_4(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    now += DUAL_MODE_TIMEOUT_MS;
    key_event_handler.handle_key_event({ T }, now);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(3);

    key_event_handler.handle_key_event({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(4);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, KC_C, KC_B, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Dual mod key with a normal key, but we release the dual key first.
// Should still send CTRL+B
void test_dual_mod_key_5(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, normal_KC_B    }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(3);

    key_event_handler.handle_key_event({ R, normal_KC_B    }, now++);
    HID_SIZE_CHECK(4);

    REPORT_COMPARE_AT(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x01, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x00, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Multiple dual keys without timeout
// After pressing the second dual key, we trigger the modifier on the first.
// After pressing the normal key, we trigger the modifier on the second.
// We expect CTRL+SHIFT+B
void test_dual_mod_key_6(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(3);

    key_event_handler.handle_key_event({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    key_event_handler.handle_key_event({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(5);

    key_event_handler.handle_key_event({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(6);

    REPORT_COMPARE_AT(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x03, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(4, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(5, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Multiple dual keys with timeout
// After pressing the second dual key, we trigger the modifier on the first.
// After a long delay, we trigger the primary key on the second dual key,
// sending CTRL+D. Then we press a normal key, which should send CTRL+B
// We expect CTRL+SHIFT+B
void test_dual_mod_key_7(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    now += DUAL_MODE_TIMEOUT_MS;
    key_event_handler.handle_key_event({ T }, now);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(3);

    key_event_handler.handle_key_event({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    key_event_handler.handle_key_event({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(5);

    key_event_handler.handle_key_event({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(6);

    REPORT_COMPARE_AT(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x01, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x01, 0x00, KC_D, KC_B, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x01, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(4, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(5, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Press only dual mod keys
// After pressing the second dual key, we trigger the modifier on the first one
// and leave the second one in STATE_PENDING. After releasing the second one
// we trigger the primary key on it
void test_dual_mod_key_8(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(3);

    key_event_handler.handle_key_event({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(4);

    REPORT_COMPARE_AT(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x01, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

void test_dual_mod_key_9(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(4);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x00, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When tapping the dual layer key, the secondary key should trigger
void test_dual_layer_key_1(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, dual_layer_1 }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_G, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a different key after the dual layer key, we should
// move to given layer. So for the key `normal_KC_A` we want to trigger
// KC_E, as it is defined on layer 1
void test_dual_layer_key_2(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a solo dual layer key as the first key, it should trigger
// a new layer
void test_dual_layer_key_3(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, solo_dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a solo dual layer key NOT as the first key, it should trigger
// the secondary key instead of layer press immediately
void test_dual_layer_key_4(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ P, solo_dual_layer_1 }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, KC_A, KC_H, 0x00, 0x00, 0x00, 0x00 });
}

// Press only dual layer keys
// After pressing the second dual key, we trigger the layer on the first one
// and leave the second one in STATE_PENDING. After releasing the second one
// we trigger the primary key on it
void test_dual_layer_key_5(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, dual_layer_2 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, dual_layer_3 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, dual_layer_3 }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ R, dual_layer_2 }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ R, dual_layer_1 }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_I, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// There was an issue with the `key_index` value in the PressedKeys
// structure after calling `remove_from_pressed_keys` - the value was not updated.
// That caused in this test to the dual_shift_KC_D key to be stuck even after
// releasing it.
void test_stuck_key(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    key_event_handler.handle_key_event({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(3);

    key_event_handler.handle_key_event({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    key_event_handler.handle_key_event({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(6);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, KC_A, KC_B, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x00, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(4, { 0x00, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(5, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that the one-shot CTRL modifier is sent after tapping the one shot key
void test_one_shot_modifier(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    REPORT_COMPARE_AT(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x01, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that after the second press of the one-shot modifier the CTRL modifier
// is toggled off, so when pressing KC_A, it is not sent
void test_one_shot_modifier_toggle(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    REPORT_COMPARE_AT(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When a one-shot modifier is held with a different key, it is registered
// as a normal modifier - i.e. with the second press of KC_A it is not sent
// any more
void test_one_shot_modifier_hold(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    key_event_handler.handle_key_event({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(4);

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(5);

    key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(6);

    REPORT_COMPARE_AT(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x01, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(4, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(5, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// We won't toggle OSM if the key is tapped/held for too long
void test_one_shot_modifier_long_tap(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    now += ONE_SHOT_MODIFIER_TAP_TIMEOUT_MS;

    key_event_handler.handle_key_event({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that the layer switch is applied and we send KC_E (layer1)
void test_layer_hold_or_toggle(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that the second tap on the key toggles the layer off, so we send KC_A
void test_layer_hold_or_toggle_on_off(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When a LHT key is held while a different key is pressed, the LHT should
// register as a normal layer key. That is with the first KC_A press we send
// KC_E (layer 1) and with the second KC_A (layer 0)
void test_layer_hold_or_toggle_hold(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ R, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(4);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that pressing an LP key changes the layer, so instead of KC_A we get
// KC_E from layer 1
void test_layer_press_1(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, layer_press_1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that releasing the LP key changes the layer back, so we should get KC_A
void test_layer_press_2(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, layer_press_1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, layer_press_1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test clearing of the pressed_keys structure. We first fill it up, Then
// remove all but one element (so that keyboard.clear() is not triggered).
void test_remove_from_pressed_keys(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    for (uint8_t i = 0; i < PRESSED_KEY_CAPACITY; i++) {
        key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    }

    for (uint8_t i = 0; i < PRESSED_KEY_CAPACITY - 1; i++) {
        key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    }

    PressedKeys* pressed_keys = test_friend.get_pressed_keys(&key_event_handler);

    // The first key is still processed
    PressedKey pressed_key = pressed_keys->keys[0];
    TEST_CHECK(pressed_key.timestamp != 0);
    TEST_MSG("index:0 timestamp:%d expected 0", pressed_key.timestamp);
    TEST_CHECK(pressed_key.state == STATE_ACTIVE_CODE);
    TEST_MSG("index:0 state:%d expected 0", pressed_key.state);

    // The rest should be zeroed out
    for (uint8_t i = 1; i < PRESSED_KEY_CAPACITY; i++) {
        PressedKey pressed_key = pressed_keys->keys[i];
        TEST_CHECK(pressed_key.timestamp == 0);
        TEST_MSG("index:%d timestamp:%d expected 0", i, pressed_key.timestamp);
        TEST_CHECK(pressed_key.state == STATE_NOT_PROCESSED);
        TEST_MSG("index:%d state:%d expected 0", i, pressed_key.state);
    }
}

// Test that pressing and releasing a combo in any order activates/deactivated
// the target key KC_A
void test_combo_simple_press_and_release(void) {
    struct TestCase {
        const char* name;
        BaseKeyEvent events[4];
    };

    TestCase test_cases[] = {
        { "1st", { PRESS(3,0), PRESS(3,7), RELEASE(3,7), RELEASE(3,0) } },
        { "2nd", { PRESS(3,0), PRESS(3,7), RELEASE(3,0), RELEASE(3,7) } },
        { "3rd", { PRESS(3,7), PRESS(3,0), RELEASE(3,7), RELEASE(3,0) } },
        { "4th", { PRESS(3,7), PRESS(3,0), RELEASE(3,0), RELEASE(3,7) } },
    };

    for (uint8_t t = 0; t < sizeof(test_cases)/sizeof(test_cases[0]); t++) {
        TestCase test_case = test_cases[t];
        TEST_CASE(test_case.name);

        HID().clear();

        KeyEventHandler key_event_handler;

        millisec now = 10000;

        // The first key is part of a combo, so it is buffered in combo_handler
        key_event_handler.handle_key_event(test_case.events[0], now++);
        HID_SIZE_CHECK(0);

        // The second key completes the combo and activates it
        key_event_handler.handle_key_event(test_case.events[1], now++);
        HID_SIZE_CHECK(1);

        // One of the combo key is still pressed, so we don't release the target key yet
        key_event_handler.handle_key_event(test_case.events[2], now++);
        HID_SIZE_CHECK(1);

        // That last key of the combo is released, so we release the target key
        key_event_handler.handle_key_event(test_case.events[3], now++);
        HID_SIZE_CHECK(2);

        REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
        REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    }
}

// Test the internal state of the combos when pressing a non-combo key
void test_combo_state_changes_1(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < combos_count; i++) {
        Combo* combo = COMBO_AT(i);

        TEST_CHECK(combo->flags == 0x00);
        TEST_CHECK(combo->pressed_coords == 0x00);
    }

    ComboHandlerState* state = test_friend.get_combo_handler_state(&key_event_handler);
    TEST_CHECK(state->held_up_keys.count == 0);
}

// Test the internal state after pressing a single combo key
// which is part of every combo
void test_combo_state_changes_2(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    for (uint8_t i = 0; i < 4; i++) {
        Combo* combo = COMBO_AT(i);
        TEST_CHECK(combo->flags == 0x00);
        TEST_CHECK(combo->pressed_coords == 0x01);
    }

    // Disabled
    Combo* combo = COMBO_AT(4);
    TEST_CHECK(combo->flags == FLAG_DISABLED);
    TEST_CHECK(combo->pressed_coords == 0x00);

    ComboHandlerState* state = test_friend.get_combo_handler_state(&key_event_handler);
    TEST_CHECK(state->held_up_keys.count == 1);
    TEST_CHECK(state->pending_combo_start == 10000);

    HeldUpKey* hkey = state->held_up_keys.get_by_index(0);
    TEST_CHECK(hkey->coords.row == 3);
    TEST_CHECK(hkey->coords.col == 0);
    TEST_CHECK(hkey->timestamp == 10000);
    TEST_CHECK(hkey->part_of_active_combo == 0);
    TEST_CHECK(hkey->active_combo_index == 0);
}

// Test internal state after pressing a single combo key
// which is part of a few combos
void test_combo_state_changes_3(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event(PRESS(3,3), now++);
    HID_SIZE_CHECK(0);

    for (uint8_t i = 0; i < 2; i++) { // These should be disabled
        Combo* combo = COMBO_AT(i);
        TEST_CHECK(combo->flags == FLAG_DISABLED);
        TEST_CHECK(combo->pressed_coords == 0x00);
    }
    for (uint8_t i = 2; i < 4; i++) { // These should be pending
        Combo* combo = COMBO_AT(i);
        TEST_CHECK(combo->flags == 0x00);
        TEST_CHECK(combo->pressed_coords == 0b100);
    }
    // Disabled
    Combo* combo = COMBO_AT(4);
    TEST_CHECK(combo->flags == FLAG_DISABLED);
    TEST_CHECK(combo->pressed_coords == 0x00);

    ComboHandlerState* state = test_friend.get_combo_handler_state(&key_event_handler);
    TEST_CHECK(state->held_up_keys.count == 1);
    TEST_CHECK(state->pending_combo_start == 10000);

    HeldUpKey* hkey = state->held_up_keys.get_by_index(0);
    TEST_CHECK(hkey->coords.row == 3);
    TEST_CHECK(hkey->coords.col == 3);
    TEST_CHECK(hkey->timestamp == 10000);
    TEST_CHECK(hkey->part_of_active_combo == 0);
    TEST_CHECK(hkey->active_combo_index == 0);
}

// Continuation of test_combo_state_changes_3, but we press another key,
// so now only 1 combo should be pending, the rest should be disabled
void test_combo_state_changes_4(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event(PRESS(3,3), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,4), now++);
    HID_SIZE_CHECK(0);

    uint8_t disabled_combo_indexes[] = { 0, 1, 2, 4 };
    uint8_t pending_combo_indexes[] = { 3 };

    for (uint8_t i = 0; i < sizeof(disabled_combo_indexes)/sizeof(uint8_t); i++) {
        Combo* combo = COMBO_AT(disabled_combo_indexes[i]);
        TEST_CHECK(combo->flags == FLAG_DISABLED);
    }
    for (uint8_t i = 0; i < sizeof(pending_combo_indexes)/sizeof(uint8_t); i++) {
        Combo* combo = COMBO_AT(pending_combo_indexes[i]);
        TEST_CHECK(combo->flags == 0x00);
        TEST_CHECK(combo->pressed_coords == 0b1100);
    }

    ComboHandlerState* state = test_friend.get_combo_handler_state(&key_event_handler);
    TEST_CHECK(state->held_up_keys.count == 2);
    TEST_CHECK(state->pending_combo_start == 10000);

    HeldUpKey* hkey1 = state->held_up_keys.get_by_index(0);
    TEST_CHECK(hkey1->coords.row == 3);
    TEST_CHECK(hkey1->coords.col == 3);
    TEST_CHECK(hkey1->timestamp == 10000);
    TEST_CHECK(hkey1->part_of_active_combo == 0);
    TEST_CHECK(hkey1->active_combo_index == 0);

    HeldUpKey* hkey2 = state->held_up_keys.get_by_index(1);
    TEST_CHECK(hkey2->coords.row == 3);
    TEST_CHECK(hkey2->coords.col == 4);
    TEST_CHECK(hkey2->timestamp == 10001);
    TEST_CHECK(hkey2->part_of_active_combo == 0);
    TEST_CHECK(hkey2->active_combo_index == 0);
}

// Continuation of test_combo_state_changes_4, but we press another key,
// activating the combo. The combo is activated, so the states should be
// reset except the active combo
void test_combo_state_changes_5(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event(PRESS(3,3), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,4), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,1), now++);
    HID_SIZE_CHECK(1);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < 3; i++) { // These should be disabled
        Combo* combo = COMBO_AT(i);
        TEST_CHECK(combo->flags == 0x00);
        TEST_CHECK(combo->pressed_coords == 0x00);
    }
    for (uint8_t i = 3; i < 4; i++) { // This is activated
        Combo* combo = COMBO_AT(i);
        TEST_CHECK(combo->flags == FLAG_ACTIVATED);
        TEST_CHECK(combo->pressed_coords == 0b1111);
    }

    ComboHandlerState* state = test_friend.get_combo_handler_state(&key_event_handler);
    TEST_CHECK(state->held_up_keys.count == 4);
    TEST_CHECK(state->pending_combo_start == 0);

    HeldUpKey* hkey1 = state->held_up_keys.get_by_index(0);
    TEST_CHECK(hkey1->coords.row == 3);
    TEST_CHECK(hkey1->coords.col == 3);
    TEST_CHECK(hkey1->timestamp == 10000);
    TEST_CHECK(hkey1->part_of_active_combo == true);
    TEST_CHECK(hkey1->active_combo_index == 3);

    HeldUpKey* hkey2 = state->held_up_keys.get_by_index(1);
    TEST_CHECK(hkey2->coords.row == 3);
    TEST_CHECK(hkey2->coords.col == 4);
    TEST_CHECK(hkey2->timestamp == 10001);
    TEST_CHECK(hkey2->part_of_active_combo == true);
    TEST_CHECK(hkey2->active_combo_index == 3);

    HeldUpKey* hkey3 = state->held_up_keys.get_by_index(2);
    TEST_CHECK(hkey3->coords.row == 3);
    TEST_CHECK(hkey3->coords.col == 0);
    TEST_CHECK(hkey3->timestamp == 10002);
    TEST_CHECK(hkey3->part_of_active_combo == true);
    TEST_CHECK(hkey3->active_combo_index == 3);

    HeldUpKey* hkey4 = state->held_up_keys.get_by_index(3);
    TEST_CHECK(hkey4->coords.row == 3);
    TEST_CHECK(hkey4->coords.col == 1);
    TEST_CHECK(hkey4->timestamp == 10003);
    TEST_CHECK(hkey4->part_of_active_combo == true);
    TEST_CHECK(hkey4->active_combo_index == 3);
}

// Continuation of test_combo_state_changes_5, but we release all but one key.
// The combo should be still active and the release event should not be sent
// to the keyboard yet
void test_combo_state_changes_6(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event(PRESS(3,3), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,4), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,1), now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event(RELEASE(3,4), now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event(RELEASE(3,1), now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event(RELEASE(3,3), now++);
    HID_SIZE_CHECK(1);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < 3; i++) { // These should be disabled
        Combo* combo = COMBO_AT(i);
        TEST_CHECK(combo->flags == 0x00);
        TEST_CHECK(combo->pressed_coords == 0x00);
    }
    for (uint8_t i = 3; i < 4; i++) { // This is activated
        Combo* combo = COMBO_AT(i);
        TEST_CHECK(combo->flags == FLAG_ACTIVATED);
        TEST_CHECK(combo->pressed_coords == 0b1);
    }

    ComboHandlerState* state = test_friend.get_combo_handler_state(&key_event_handler);
    TEST_CHECK(state->held_up_keys.count == 1);
    TEST_CHECK(state->pending_combo_start == 0);

    HeldUpKey* hkey1 = state->held_up_keys.get_by_index(0);
    TEST_CHECK(hkey1->coords.row == 3);
    TEST_CHECK(hkey1->coords.col == 0);
    TEST_CHECK(hkey1->timestamp == 10002);
    TEST_CHECK(hkey1->part_of_active_combo == true);
    TEST_CHECK(hkey1->active_combo_index == 3);
}

// Continuation of test_combo_state_changes_6; we release all keys.
void test_combo_state_changes_7(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event(PRESS(3,3), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,4), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,1), now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event(RELEASE(3,4), now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event(RELEASE(3,1), now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event(RELEASE(3,3), now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event(RELEASE(3,0), now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < combos_count; i++) { // These should be reset
        Combo* combo = COMBO_AT(i);
        TEST_CHECK(combo->flags == 0x00);
        TEST_CHECK(combo->pressed_coords == 0x00);
    }

    ComboHandlerState* state = test_friend.get_combo_handler_state(&key_event_handler);
    TEST_CHECK(state->held_up_keys.count == 0);
    TEST_CHECK(state->pending_combo_start == 0);
}

// We press 1 combo key and then 1 non-combo key. On the second press the
// pending combo processing should abort and both keys should be sent to
// the keyboard
void test_combo_state_changes_abort(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event(RELEASE(3,0), now++);
    HID_SIZE_CHECK(3);

    key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(4);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_Q, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, KC_Q, KC_A, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < combos_count; i++) { // These should be reset
        Combo* combo = COMBO_AT(i);
        TEST_CHECK(combo->flags == 0x00);
        TEST_CHECK(combo->pressed_coords == 0x00);
    }

    ComboHandlerState* state = test_friend.get_combo_handler_state(&key_event_handler);
    TEST_CHECK(state->held_up_keys.count == 0);
    TEST_CHECK(state->pending_combo_start == 0);
}

// Abort pending combo processing after timeout, because there are no fully
// pressed combos
void test_combo_timeout_abort(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event(PRESS(3,1), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    // Not yet
    key_event_handler.handle_key_event({ T }, now);
    HID_SIZE_CHECK(0);

    now += COMBO_PROCESSING_LIMIT_MS;

    key_event_handler.handle_key_event({ T }, now);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_W, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, KC_W, KC_Q, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < combos_count; i++) { // These should be reset
        Combo* combo = COMBO_AT(i);
        TEST_CHECK(combo->flags == 0x00);
        TEST_CHECK(combo->pressed_coords == 0x00);
    }

    ComboHandlerState* state = test_friend.get_combo_handler_state(&key_event_handler);
    TEST_CHECK(state->held_up_keys.count == 0);
    TEST_CHECK(state->pending_combo_start == 0);

    // Release the keys - no pending combo processing, so just forward them
    // to the keyboard
    key_event_handler.handle_key_event(RELEASE(3,1), now++);
    HID_SIZE_CHECK(3);

    key_event_handler.handle_key_event(RELEASE(3,0), now++);
    HID_SIZE_CHECK(4);

    REPORT_COMPARE_AT(2, { 0x00, 0x00, KC_Q, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Activate combo3 after timeout even if combo4 is pending as well, but
// only combo3 is fully pressed
void test_combo_timeout_activate(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,1), now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,3), now++);
    HID_SIZE_CHECK(0);

    // Not yet
    key_event_handler.handle_key_event({ T }, now);
    HID_SIZE_CHECK(0);

    now += COMBO_PROCESSING_LIMIT_MS;

    key_event_handler.handle_key_event({ T }, now);
    HID_SIZE_CHECK(1);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });

    // These should be reset
    Combo* combo1 = COMBO_AT(0);
    TEST_CHECK(combo1->flags == 0x00);
    TEST_CHECK(combo1->pressed_coords == 0x00);

    Combo* combo2 = COMBO_AT(1);
    TEST_CHECK(combo2->flags == 0x00);
    TEST_CHECK(combo2->pressed_coords == 0x00);

    Combo* combo3 = COMBO_AT(3);
    TEST_CHECK(combo3->flags == 0x00);
    TEST_CHECK(combo3->pressed_coords == 0x00);

    // This should be active
    Combo* combo4 = COMBO_AT(2);
    TEST_CHECK(combo4->flags == FLAG_ACTIVATED);
    TEST_CHECK(combo4->pressed_coords == 0b111);

    ComboHandlerState* state = test_friend.get_combo_handler_state(&key_event_handler);
    TEST_CHECK(state->held_up_keys.count == 3);
    TEST_CHECK(state->pending_combo_start == 0);

    for (uint8_t i = 0; i < state->held_up_keys.count; i++) {
        HeldUpKey* hkey = state->held_up_keys.get_by_index(i);
        TEST_CHECK(hkey->timestamp > 0);
        TEST_CHECK(hkey->part_of_active_combo == true);
        TEST_CHECK(hkey->active_combo_index == 2);
    }
}

// We activate 2 combos at once
void test_combo_multiple_active_combos(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    // Combo1
    key_event_handler.handle_key_event(PRESS(3,5), now++);
    HID_SIZE_CHECK(0);
    key_event_handler.handle_key_event(PRESS(3,6), now++);
    HID_SIZE_CHECK(1);

    // Combo2
    key_event_handler.handle_key_event(PRESS(3,7), now++);
    HID_SIZE_CHECK(1);
    key_event_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x01, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });

    // Relase just a single key from each combo
    key_event_handler.handle_key_event(RELEASE(3,6), now++);
    HID_SIZE_CHECK(2);
    key_event_handler.handle_key_event(RELEASE(3,7), now++);
    HID_SIZE_CHECK(2);

    // Release the rest
    key_event_handler.handle_key_event(RELEASE(3,5), now++);
    HID_SIZE_CHECK(3);
    key_event_handler.handle_key_event(RELEASE(3,0), now++);
    HID_SIZE_CHECK(4);

    REPORT_COMPARE_AT(2, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test stuck combo key - sometimes the release event is not sent to the keyboard
// Apparently it is happening, when after activating a combo I release-then-press-again
// one of the combo keys
void test_combo_stuck_key1(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    // We activate the combo - everything according to the plan
    key_event_handler.handle_key_event(PRESS(3,7), now++);
    HID_SIZE_CHECK(0);
    key_event_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(1);

    // We release one of the key - we send nothing, which is OK, because
    // the other key is still pressed
    key_event_handler.handle_key_event(RELEASE(3,7), now++);
    HID_SIZE_CHECK(1);
    // We press the same key again - this triggered the bug
    key_event_handler.handle_key_event(PRESS(3,7), now++);
    HID_SIZE_CHECK(1);

    // We release the last key of the combo. Note that (3,7) is still pressed,
    // but after the first release of that key we've already modified the state
    // of the combo; the second press did not change the existing active combo,
    // so after releasing (3,0) we consider it fully released.
    key_event_handler.handle_key_event(RELEASE(3,0), now++);
    HID_SIZE_CHECK(2);
    key_event_handler.handle_key_event(RELEASE(3,7), now++);
    HID_SIZE_CHECK(2);

    // At this point before fixing the error, HID_SIZE_CHECK(1) == true, so
    // the KC_A key is stuck = the release event was never sent to the keyboard

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When tapping a single combo key, only the PRESS event was sent to the keyboard
// upon aborting pending combo processing. In this case the RELEASE event should
// be sent as well.
void test_combo_stuck_key2(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    // We activate the combo - everything according to the plan
    key_event_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);
    key_event_handler.handle_key_event(RELEASE(3,0), now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_Q, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// We don't start a new pending combo processing until COMBO_START_THRESHOLD_MS
// passes. The first { P, kc_no } sets the timer to 'now', so the PRESS(3,0)
// event 1 ms later does not start the combo processing; instead the event
// is passed to the Keyboard and KC_Q is pressed.
void test_combo_start_threshold(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, kc_no }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(1);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_Q, 0x00, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < combos_count; i++) {
        Combo* combo = COMBO_AT(i);

        TEST_CHECK(combo->flags == 0x00);
        TEST_CHECK(combo->pressed_coords == 0x00);
    }

    ComboHandlerState* state = test_friend.get_combo_handler_state(&key_event_handler);
    TEST_CHECK(state->held_up_keys.count == 0);
}

// Test caps_word happy path. Pressing A/B should add the caps_word_modifier.
// Pressing 1 should keep caps_word on but without adding caps_word_modifier.
void test_caps_word_1(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    // Toggle caps_word
    key_event_handler.handle_key_event({ P, caps_word }, now++);
    HID_SIZE_CHECK(0);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == true);

    key_event_handler.handle_key_event({ R, caps_word }, now++);
    HID_SIZE_CHECK(0);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == true);

    // Press A
    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == true);

    key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == true);

    // Press B
    key_event_handler.handle_key_event({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(3);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == true);

    key_event_handler.handle_key_event({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == true);

    // Press 1
    key_event_handler.handle_key_event({ P, normal_KC_1 }, now++);
    HID_SIZE_CHECK(5);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == true);

    key_event_handler.handle_key_event({ R, normal_KC_1 }, now++);
    HID_SIZE_CHECK(6);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == true);

    // Toggle caps_word
    key_event_handler.handle_key_event({ P, caps_word }, now++);
    HID_SIZE_CHECK(6);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == false);

    key_event_handler.handle_key_event({ R, caps_word }, now++);
    HID_SIZE_CHECK(6);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == false);

    REPORT_COMPARE_AT(0, { 0x02, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x02, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(4, { 0x00, 0x00, KC_1, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(5, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Pressing SPACE should turn off caps_word. No caps_word_modifier is sent at all.
void test_caps_word_2(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    // Toggle caps_word
    key_event_handler.handle_key_event({ P, caps_word }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, caps_word }, now++);
    HID_SIZE_CHECK(0);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == true);

    // Press Space
    key_event_handler.handle_key_event({ P, normal_KC_SPACE }, now++);
    HID_SIZE_CHECK(1);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == false);

    key_event_handler.handle_key_event({ R, normal_KC_SPACE }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_SPC, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Automatically turn off caps_word after timeout
void test_caps_word_3(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    // Toggle caps_word
    key_event_handler.handle_key_event({ P, caps_word }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, caps_word }, now++);
    HID_SIZE_CHECK(0);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == true);

    now += CAPS_WORD_TIMEOUT_MS - 2;
    key_event_handler.handle_key_event({ T }, now);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == true);

    now += 1;
    key_event_handler.handle_key_event({ T }, now);
    TEST_CHECK(test_friend.get_caps_word_enabled(&key_event_handler) == false);

    HID_SIZE_CHECK(0);
}

// When pressing a DTH dual_mod key after a different key, it should go to pending state
// and not trigger any action at that moment
void test_double_tap_hold_mod_1(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ P, dth_ctrl_j }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });

    PressedKeys* pressed_keys = test_friend.get_pressed_keys(&key_event_handler);
    PressedKey* pk = pressed_keys->find(dth_ctrl_j);

    if (pk) {
        TEST_CHECK(pk->state == STATE_PENDING);
        TEST_MSG("expected state %d, got %d", STATE_PENDING, pk->state);
    }
}

// When pressing the DTH mod the second time, we immediately trigger KC_J
void test_double_tap_hold_mod_2(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, dth_ctrl_j }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, dth_ctrl_j }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ P, dth_ctrl_j }, now++);
    HID_SIZE_CHECK(3);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_J, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x00, 0x00, KC_J, 0x00, 0x00, 0x00, 0x00, 0x00 });

    PressedKeys* pressed_keys = test_friend.get_pressed_keys(&key_event_handler);
    PressedKey* pk = pressed_keys->find(dth_ctrl_j);

    if (pk) {
        TEST_CHECK(pk->state == STATE_ACTIVE_CODE);
        TEST_MSG("expected state %d, got %d", STATE_ACTIVE_CODE, pk->state);
    }
}

// When pressing the DTH mod the second time after the threshold, we don't immediately
// trigger KC_J as in the previous test
void test_double_tap_hold_mod_3(void) {
    KeyEventHandler key_event_handler;

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, dth_ctrl_j }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, dth_ctrl_j }, now++);
    HID_SIZE_CHECK(2);

    now += DUAL_TAP_HOLD_THRESHOLD_MS;

    key_event_handler.handle_key_event({ P, dth_ctrl_j }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_J, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });

    PressedKeys* pressed_keys = test_friend.get_pressed_keys(&key_event_handler);
    PressedKey* pk = pressed_keys->find(dth_ctrl_j);

    if (pk) {
        TEST_CHECK(pk->state == STATE_PENDING);
        TEST_MSG("expected state %d, got %d", STATE_PENDING, pk->state);
    }
}

// When tapping a TapDance key once, it should send KC_A after timeout.
// The TapDance key should not be in PressedKeys.
void test_tapdance_single_tap_idle_timeout(void) {
    KeyEventHandler key_event_handler;
    PressedKeys* pressed_keys = test_friend.get_pressed_keys(&key_event_handler);
    TapDanceHandlerState* td_state = test_friend.get_tapdance_state(&key_event_handler);

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(0);
    TEST_CHECK(pressed_keys->count == 0);
    TEST_CHECK(td_state->pending_tapdance_start > 0);
    TEST_CHECK(td_state->pending_tap_count == 1);
    TEST_CHECK(td_state->last_key_event.coords.equals(td1));

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(0);
    TEST_CHECK(pressed_keys->count == 0);
    TEST_CHECK(td_state->pending_tapdance_start > 0);
    TEST_CHECK(td_state->pending_tap_count == 1);
    TEST_CHECK(td_state->last_key_event.coords.equals(td1));

    now += TAPDANCE_IDLE_TRIGGER_THRESHOLD_MS - 1;

    // 1 ms before the timeout threshold
    key_event_handler.handle_key_event({ T }, now++);
    HID_SIZE_CHECK(0);
    TEST_CHECK(pressed_keys->count == 0);

    key_event_handler.handle_key_event({ T }, now++);
    HID_SIZE_CHECK(2);
    TEST_CHECK(pressed_keys->count == 0);
    TEST_CHECK(td_state->pending_tapdance_start == 0);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When tapping a TapDance key twice, it should send KC_B after timeout.
// The TapDance key should not be in PressedKeys.
void test_tapdance_double_tap_idle_timeout(void) {
    KeyEventHandler key_event_handler;
    PressedKeys* pressed_keys = test_friend.get_pressed_keys(&key_event_handler);
    TapDanceHandlerState* td_state = test_friend.get_tapdance_state(&key_event_handler);

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(0);
    TEST_CHECK(td_state->pending_tap_count == 1);

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(0);
    TEST_CHECK(td_state->pending_tap_count == 2);

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(0);

    now += TAPDANCE_IDLE_TRIGGER_THRESHOLD_MS - 1;

    // 1 ms before the timeout threshold
    key_event_handler.handle_key_event({ T }, now++);
    HID_SIZE_CHECK(0);
    TEST_CHECK(pressed_keys->count == 0);

    key_event_handler.handle_key_event({ T }, now++);
    HID_SIZE_CHECK(2);
    TEST_CHECK(pressed_keys->count == 0);
    TEST_CHECK(td_state->pending_tapdance_start == 0);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When tapping for the 3rd time - which is the number of target keys for this TapDance -,
// we immediately send LGUI(KC_C)
void test_tapdance_triple_tap(void) {
    KeyEventHandler key_event_handler;
    PressedKeys* pressed_keys = test_friend.get_pressed_keys(&key_event_handler);
    TapDanceHandlerState* td_state = test_friend.get_tapdance_state(&key_event_handler);

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(0);

    // This is the final tap for this TapDance, so we immediately trigger the target key
    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(1);
    TEST_CHECK(td_state->pending_tapdance_start == 0);

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x08, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing and holding a TapDance key on the first tap, we send KC_A after timeout
void test_tapdance_single_tap_hold_timeout(void) {
    KeyEventHandler key_event_handler;
    PressedKeys* pressed_keys = test_friend.get_pressed_keys(&key_event_handler);
    TapDanceHandlerState* td_state = test_friend.get_tapdance_state(&key_event_handler);

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(0);

    now += TAPDANCE_HOLD_TRIGGER_THRESHOLD_MS - 1;

    // 1 ms before the timeout threshold
    key_event_handler.handle_key_event({ T }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ T }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing and holding a TapDance key on the second tap, we send KC_B after timeout
void test_tapdance_double_tap_hold_timeout(void) {
    KeyEventHandler key_event_handler;
    PressedKeys* pressed_keys = test_friend.get_pressed_keys(&key_event_handler);
    TapDanceHandlerState* td_state = test_friend.get_tapdance_state(&key_event_handler);

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(0);

    now += TAPDANCE_HOLD_TRIGGER_THRESHOLD_MS - 1;

    // 1 ms before the timeout threshold
    key_event_handler.handle_key_event({ T }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ T }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_B, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing and holding a TapDance key on the third tap, we immediately trigger LGUI(KC_C),
// as 3 is the count of target keys for this TapDance
void test_tapdance_triple_tap_hold_timeout(void) {
    KeyEventHandler key_event_handler;
    PressedKeys* pressed_keys = test_friend.get_pressed_keys(&key_event_handler);
    TapDanceHandlerState* td_state = test_friend.get_tapdance_state(&key_event_handler);

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(1);

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(2);

    REPORT_COMPARE_AT(0, { 0x08, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a different key after tapping a TapDance, we trigger KC_A and after that we process
// KC_1 as usual
void test_tapdance_tap_and_different_key(void) {
    KeyEventHandler key_event_handler;
    PressedKeys* pressed_keys = test_friend.get_pressed_keys(&key_event_handler);
    TapDanceHandlerState* td_state = test_friend.get_tapdance_state(&key_event_handler);

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, normal_KC_1 }, now++);
    HID_SIZE_CHECK(3);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x00, 0x00, KC_1, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a different key after pressing and holding a TapDance, we trigger KC_A and after that
// we process KC_1 as usual
void test_tapdance_hold_and_different_key(void) {
    KeyEventHandler key_event_handler;
    PressedKeys* pressed_keys = test_friend.get_pressed_keys(&key_event_handler);
    TapDanceHandlerState* td_state = test_friend.get_tapdance_state(&key_event_handler);

    millisec now = 10000;

    key_event_handler.handle_key_event({ P, td1 }, now++);
    HID_SIZE_CHECK(0);

    key_event_handler.handle_key_event({ P, normal_KC_1 }, now++);
    HID_SIZE_CHECK(2);

    key_event_handler.handle_key_event({ R, td1 }, now++);
    HID_SIZE_CHECK(3);

    REPORT_COMPARE_AT(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(1, { 0x00, 0x00, KC_A, KC_1, 0x00, 0x00, 0x00, 0x00 });
    REPORT_COMPARE_AT(2, { 0x00, 0x00, KC_1, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

TEST_LIST = {
    { "test_array_utils1", test_array_utils1 },
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
    { "test_dual_layer_key_5", test_dual_layer_key_5 },
    { "test_stuck_key", test_stuck_key },
    { "test_one_shot_modifier", test_one_shot_modifier },
    { "test_one_shot_modifier_toggle", test_one_shot_modifier_toggle },
    { "test_one_shot_modifier_hold", test_one_shot_modifier_hold },
    { "test_one_shot_modifier_long_tap", test_one_shot_modifier_long_tap },
    { "test_layer_hold_or_toggle", test_layer_hold_or_toggle },
    { "test_layer_hold_or_toggle_on_off", test_layer_hold_or_toggle_on_off },
    { "test_layer_hold_or_toggle_hold", test_layer_hold_or_toggle_hold },
    { "test_layer_press_1", test_layer_press_1 },
    { "test_layer_press_2", test_layer_press_2 },
    { "test_remove_from_pressed_keys", test_remove_from_pressed_keys },
    { "test_combo_simple_press_and_release", test_combo_simple_press_and_release },
    { "test_combo_state_changes_1", test_combo_state_changes_1 },
    { "test_combo_state_changes_2", test_combo_state_changes_2 },
    { "test_combo_state_changes_3", test_combo_state_changes_3 },
    { "test_combo_state_changes_4", test_combo_state_changes_4 },
    { "test_combo_state_changes_5", test_combo_state_changes_5 },
    { "test_combo_state_changes_6", test_combo_state_changes_6 },
    { "test_combo_state_changes_7", test_combo_state_changes_7 },
    { "test_combo_state_changes_abort", test_combo_state_changes_abort },
    { "test_combo_timeout_abort", test_combo_timeout_abort },
    { "test_combo_timeout_activate", test_combo_timeout_activate },
    { "test_combo_multiple_active_combos", test_combo_multiple_active_combos },
    { "test_combo_stuck_key1", test_combo_stuck_key1 },
    { "test_combo_stuck_key2", test_combo_stuck_key2 },
    { "test_combo_start_threshold", test_combo_start_threshold },
    { "test_caps_word_1", test_caps_word_1 },
    { "test_caps_word_2", test_caps_word_2 },
    { "test_caps_word_3", test_caps_word_3 },
    { "test_double_tap_hold_mod_1", test_double_tap_hold_mod_1 },
    { "test_double_tap_hold_mod_2", test_double_tap_hold_mod_2 },
    { "test_double_tap_hold_mod_3", test_double_tap_hold_mod_3 },
    { "test_tapdance_single_tap_idle_timeout", test_tapdance_single_tap_idle_timeout },
    { "test_tapdance_double_tap_idle_timeout", test_tapdance_double_tap_idle_timeout },
    { "test_tapdance_triple_tap", test_tapdance_triple_tap },
    { "test_tapdance_single_tap_hold_timeout", test_tapdance_single_tap_hold_timeout },
    { "test_tapdance_double_tap_hold_timeout", test_tapdance_double_tap_hold_timeout },
    { "test_tapdance_triple_tap_hold_timeout", test_tapdance_triple_tap_hold_timeout },
    { "test_tapdance_tap_and_different_key", test_tapdance_tap_and_different_key },
    { "test_tapdance_hold_and_different_key", test_tapdance_hold_and_different_key },
    { NULL, NULL }
};

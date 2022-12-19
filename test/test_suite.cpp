#include "acutest.h"
#include "dalsik.h"
#include "combos_handler.h"
#include "keyboard.h"
#include "keymap.h"
#include "combos.h"
#include "key_definitions.h"
#include "HID.h"
#include "Serial.h"
#include "avr/eeprom.h"

PressedKeys get_keyboard_pressed_keys(Keyboard k) {
    return k.pressed_keys;
}

CombosKeyBuffer get_combos_key_buffer(CombosHandler c) {
    return c.key_buffer;
}

bool compare_base_report(size_t index, BaseHIDReport expected) {
    BaseHIDReport got = HID().base_hid_reports.at(index);
    return memcmp(&got, &expected, sizeof(BaseHIDReport)) == 0;
}

void print_hid_reports() {
    TEST_MSG("size: %lu", HID().base_hid_reports.size());
    for (uint8_t i = 0; i < HID().base_hid_reports.size(); i++ ) {
        char buffer[10];
        sprintf(buffer, "KEY %d", i);
        TEST_DUMP(buffer, &(HID().base_hid_reports.at(i)), sizeof(BaseHIDReport));
    }
}

void print_combo_states() {
    printf("\nComboStates:\n");
    for (uint8_t i = 0; i < COMBOS_COUNT; i++) {
        ComboState* combo_state = COMBO_STATE(i);
        combo_state->print_internal_state(i, 0);
    }
}

void dump_hid_reports() {
    TEST_CHECK(1 == 0);
    print_hid_reports();
}

#define SEC(x) x*1000
#define P EVENT_KEY_PRESS
#define R EVENT_KEY_RELEASE
#define T EVENT_TIMEOUT

#define PRESS(r,c) ChangedKeyEvent { EVENT_KEY_PRESS, KeyCoords { r, c } }
#define RELEASE(r,c) ChangedKeyEvent { EVENT_KEY_RELEASE, KeyCoords { r, c } }

// Synchronize with test/mocks/mock_keymap.cpp!
KeyCoords kc_no = { 0, 0 };
KeyCoords kc_ctrl = { 1, 0 };
KeyCoords normal_KC_A = { 1, 1 };
KeyCoords normal_KC_B = { 1, 2 };
KeyCoords dual_ctrl_KC_C = { 1, 3 };
KeyCoords dual_shift_KC_D = { 1, 4 };

KeyCoords dual_layer_1 = { 1, 7 };
KeyCoords dual_layer_2 = { 2, 0 };
KeyCoords dual_layer_3 = { 2, 1 };
KeyCoords solo_dual_layer_1 = { 1, 8 };
KeyCoords one_shot_ctrl = { 1, 9 };
KeyCoords layer_hold_or_toggle = { 1, 10 };

KeyCoords layer_press_1 = { 1, 11 };

// Simple press test with short delay between events
void test_normal_key_1(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    now += SEC(10);

    keyboard.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(2, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Simple press test with long delay between events
void test_normal_key_2(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    now += SEC(10);

    keyboard.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Multiple normal keys pressed at once
void test_normal_key_3(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_key_event({ R, normal_KC_B }, now++);
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

    millisec now = 10000;

    keyboard.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Dual mod key long press and release (timeout check)
// After DUAL_MODE_TIMEOUT_MS ellapses from the pressing of the key, we
// activate KC_C
void test_dual_mod_key_2(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    // Not yet...
    now += DUAL_MODE_TIMEOUT_MS - 1;
    keyboard.handle_key_event({ T }, now);
    HID_SIZE_CHECK(0);

    // Fire KC_C
    now++;
    keyboard.handle_key_event({ T }, now);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test dual key with a normal key as well
// The press of a normal key should trigger CTRL+B, as at the time the
// dual key is still in STATE_PENDING
void test_dual_mod_key_3(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, normal_KC_B    }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ R, normal_KC_B    }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_key_event({ R, dual_ctrl_KC_C }, now++);
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

    millisec now = 10000;

    keyboard.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    now += DUAL_MODE_TIMEOUT_MS;
    keyboard.handle_key_event({ T }, now);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_key_event({ R, dual_ctrl_KC_C }, now++);
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

    millisec now = 10000;

    keyboard.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, normal_KC_B    }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_key_event({ R, normal_KC_B    }, now++);
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

    millisec now = 10000;

    keyboard.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_key_event({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    keyboard.handle_key_event({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(5);

    keyboard.handle_key_event({ R, dual_ctrl_KC_C }, now++);
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

    millisec now = 10000;

    keyboard.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    now += DUAL_MODE_TIMEOUT_MS;
    keyboard.handle_key_event({ T }, now);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_key_event({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    keyboard.handle_key_event({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(5);

    keyboard.handle_key_event({ R, dual_ctrl_KC_C }, now++);
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

    millisec now = 10000;

    keyboard.handle_key_event({ P, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_key_event({ R, dual_ctrl_KC_C }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x01, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

void test_dual_mod_key_9(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ R, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When tapping the dual layer key, the secondary key should trigger
void test_dual_layer_key_1(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ R, dual_layer_1 }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_G, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a different key after the dual layer key, we should
// move to given layer. So for the key `normal_KC_A` we want to trigger
// KC_E, as it is defined on layer 1
void test_dual_layer_key_2(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a solo dual layer key as the first key, it should trigger
// a new layer
void test_dual_layer_key_3(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, solo_dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When pressing a solo dual layer key NOT as the first key, it should trigger
// the secondary key instead of layer press immediately
void test_dual_layer_key_4(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ P, solo_dual_layer_1 }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, KC_A, KC_H, 0x00, 0x00, 0x00, 0x00 });
}

// Press only dual layer keys
// After pressing the second dual key, we trigger the layer on the first one
// and leave the second one in STATE_PENDING. After releasing the second one
// we trigger the primary key on it
void test_dual_layer_key_5(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, dual_layer_1 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, dual_layer_2 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, dual_layer_3 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ R, dual_layer_3 }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ R, dual_layer_2 }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ R, dual_layer_1 }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_I, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// There was an issue with the `key_index` value in the PressedKeys
// structure after calling `remove_from_pressed_keys` - the value was not updated.
// That caused in this test to the dual_shift_KC_D key to be stuck even after
// releasing it.
void test_stuck_key(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ P, normal_KC_B }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_key_event({ P, dual_shift_KC_D }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_key_event({ R, normal_KC_B }, now++);
    HID_SIZE_CHECK(4);

    keyboard.handle_key_event({ R, dual_shift_KC_D }, now++);
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

    millisec now = 10000;

    keyboard.handle_key_event({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x01, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that after the second press of the one-shot modifier the CTRL modifier
// is toggled off, so when pressing KC_A, it is not sent
void test_one_shot_modifier_toggle(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
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

    millisec now = 10000;

    keyboard.handle_key_event({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_key_event({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(4);

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(5);

    keyboard.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(6);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x01, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(4, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(5, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// We won't toggle OSM if the key is tapped/held for too long
void test_one_shot_modifier_long_tap(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(1);

    now += ONE_SHOT_MODIFIER_TAP_TIMEOUT_MS;

    keyboard.handle_key_event({ R, one_shot_ctrl }, now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that the layer switch is applied and we send KC_E (layer1)
void test_layer_hold_or_toggle(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ R, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that the second tap on the key toggles the layer off, so we send KC_A
void test_layer_hold_or_toggle_on_off(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ R, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ R, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// When a LHT key is held while a different key is pressed, the LHT should
// register as a normal layer key. That is with the first KC_A press we send
// KC_E (layer 1) and with the second KC_A (layer 0)
void test_layer_hold_or_toggle_hold(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    keyboard.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ R, layer_hold_or_toggle }, now++);
    HID_SIZE_CHECK(2);

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(3);

    keyboard.handle_key_event({ R, normal_KC_A }, now++);
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

    millisec now = 10000;

    keyboard.handle_key_event({ P, layer_press_1 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_E, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test that releasing the LP key changes the layer back, so we should get KC_A
void test_layer_press_2(void) {
    Keyboard keyboard;

    millisec now = 10000;

    keyboard.handle_key_event({ P, layer_press_1 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ R, layer_press_1 }, now++);
    HID_SIZE_CHECK(0);

    keyboard.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test clearing of the pressed_keys structure. We first fill it up, Then
// remove all but one element (so that keyboard.clear() is not triggered).
void test_remove_from_pressed_keys(void) {
    Keyboard keyboard;

    millisec now = 10000;

    for (uint8_t i = 0; i < PRESSED_KEY_BUFFER; i++) {
        keyboard.handle_key_event({ P, normal_KC_A }, now++);
    }

    for (uint8_t i = 0; i < PRESSED_KEY_BUFFER - 1; i++) {
        keyboard.handle_key_event({ R, normal_KC_A }, now++);
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

// Test that pressing and releasing a combo in any order activates/deactivated
// the target key KC_A
void test_combo_simple_press_and_release(void) {
    struct TestCase {
        const char* name;
        ChangedKeyEvent events[4];
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

        Keyboard keyboard;
        CombosHandler combos_handler(&keyboard);

        millisec now = 10000;

        // The first key is part of a combo, so it is buffered in combos_handler
        combos_handler.handle_key_event(test_case.events[0], now++);
        HID_SIZE_CHECK(0);

        // The second key completes the combo and activates it
        combos_handler.handle_key_event(test_case.events[1], now++);
        HID_SIZE_CHECK(1);

        // One of the combo key is still pressed, so we don't release the target key yet
        combos_handler.handle_key_event(test_case.events[2], now++);
        HID_SIZE_CHECK(1);

        // That last key of the combo is released, so we release the target key
        combos_handler.handle_key_event(test_case.events[3], now++);
        HID_SIZE_CHECK(2);

        BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
        BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    }
}

// Test the internal state of the combos when pressing a non-combo key
void test_combo_state_changes_1(void) {
    Keyboard keyboard;
    CombosHandler combos_handler(&keyboard);

    millisec now = 10000;

    combos_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < COMBOS_COUNT; i++) {
        ComboState* combo_state = COMBO_STATE(i);

        TEST_CHECK(combo_state->timestamp == 0);
        TEST_CHECK(combo_state->flags == 0x00);
        TEST_CHECK(combo_state->state == 0x00);
    }

    CombosKeyBuffer ckb = get_combos_key_buffer(combos_handler);
    TEST_CHECK(ckb.count == 0);
}

// Test the internal state after pressing a single combo key
// which is part of every combo
void test_combo_state_changes_2(void) {
    Keyboard keyboard;
    CombosHandler combos_handler(&keyboard);

    millisec now = 10000;

    combos_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    for (uint8_t i = 0; i < 4; i++) {
        ComboState* combo_state = COMBO_STATE(i);
        TEST_CHECK(combo_state->timestamp == 10000);
        TEST_CHECK(combo_state->flags == 0x00);
        TEST_CHECK(combo_state->state == 0x01);
    }

    // Disabled
    ComboState* combo_state = COMBO_STATE(4);
    TEST_CHECK(combo_state->timestamp == 0);
    TEST_CHECK(combo_state->flags == FLAG_DISABLED);
    TEST_CHECK(combo_state->state == 0x00);

    CombosKeyBuffer ckb = get_combos_key_buffer(combos_handler);
    TEST_CHECK(ckb.count == 1);

    CombosBufferedKey* cbk = ckb.get(0);
    TEST_CHECK(cbk->coords.row == 3);
    TEST_CHECK(cbk->coords.col == 0);
    TEST_CHECK(cbk->timestamp == 10000);
    TEST_CHECK(cbk->part_of_active_combo == 0);
    TEST_CHECK(cbk->active_combo_index == 0);
}

// Test internal state after pressing a single combo key
// which is part of a few combos
void test_combo_state_changes_3(void) {
    Keyboard keyboard;
    CombosHandler combos_handler(&keyboard);

    millisec now = 10000;

    combos_handler.handle_key_event(PRESS(3,3), now++);
    HID_SIZE_CHECK(0);

    for (uint8_t i = 0; i < 2; i++) { // These should be disabled
        ComboState* combo_state = COMBO_STATE(i);
        TEST_CHECK(combo_state->timestamp == 0);
        TEST_CHECK(combo_state->flags == FLAG_DISABLED);
        TEST_CHECK(combo_state->state == 0x00);
    }
    for (uint8_t i = 2; i < 4; i++) { // These should be pending
        ComboState* combo_state = COMBO_STATE(i);
        TEST_CHECK(combo_state->timestamp == 10000);
        TEST_CHECK(combo_state->flags == 0x00);
        TEST_CHECK(combo_state->state == 0b100);
    }
    // Disabled
    ComboState* combo_state = COMBO_STATE(4);
    TEST_CHECK(combo_state->timestamp == 0);
    TEST_CHECK(combo_state->flags == FLAG_DISABLED);
    TEST_CHECK(combo_state->state == 0x00);

    CombosKeyBuffer ckb = get_combos_key_buffer(combos_handler);
    TEST_CHECK(ckb.count == 1);

    CombosBufferedKey* cbk = ckb.get(0);
    TEST_CHECK(cbk->coords.row == 3);
    TEST_CHECK(cbk->coords.col == 3);
    TEST_CHECK(cbk->timestamp == 10000);
    TEST_CHECK(cbk->part_of_active_combo == 0);
    TEST_CHECK(cbk->active_combo_index == 0);
}

// Continuation of test_combo_state_changes_3, but we press another key,
// so now only 1 combo should be pending, the rest should be disabled
void test_combo_state_changes_4(void) {
    Keyboard keyboard;
    CombosHandler combos_handler(&keyboard);

    millisec now = 10000;

    combos_handler.handle_key_event(PRESS(3,3), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,4), now++);
    HID_SIZE_CHECK(0);

    for (uint8_t i = 0; i < 3; i++) { // These should be disabled
        ComboState* combo_state = COMBO_STATE(i);
        TEST_CHECK(combo_state->timestamp == 0);
        TEST_CHECK(combo_state->flags == FLAG_DISABLED);
        TEST_CHECK(combo_state->state == 0x00);
    }
    for (uint8_t i = 3; i < 4; i++) { // These should be pending
        ComboState* combo_state = COMBO_STATE(i);
        TEST_CHECK(combo_state->timestamp == 10000);
        TEST_CHECK(combo_state->flags == 0x00);
        TEST_CHECK(combo_state->state == 0b1100);
    }
    // Disabled
    ComboState* combo_state = COMBO_STATE(4);
    TEST_CHECK(combo_state->timestamp == 0);
    TEST_CHECK(combo_state->flags == FLAG_DISABLED);
    TEST_CHECK(combo_state->state == 0x00);

    CombosKeyBuffer ckb = get_combos_key_buffer(combos_handler);
    TEST_CHECK(ckb.count == 2);

    CombosBufferedKey* cbk1 = ckb.get(0);
    TEST_CHECK(cbk1->coords.row == 3);
    TEST_CHECK(cbk1->coords.col == 3);
    TEST_CHECK(cbk1->timestamp == 10000);
    TEST_CHECK(cbk1->part_of_active_combo == 0);
    TEST_CHECK(cbk1->active_combo_index == 0);

    CombosBufferedKey* cbk2 = ckb.get(1);
    TEST_CHECK(cbk2->coords.row == 3);
    TEST_CHECK(cbk2->coords.col == 4);
    TEST_CHECK(cbk2->timestamp == 10001);
    TEST_CHECK(cbk2->part_of_active_combo == 0);
    TEST_CHECK(cbk2->active_combo_index == 0);
}

// Continuation of test_combo_state_changes_4, but we press another key,
// activating the combo. The combo is activated, so the states should be
// reset except the active combo
void test_combo_state_changes_5(void) {
    Keyboard keyboard;
    CombosHandler combos_handler(&keyboard);

    millisec now = 10000;

    combos_handler.handle_key_event(PRESS(3,3), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,4), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,1), now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < 3; i++) { // These should be disabled
        ComboState* combo_state = COMBO_STATE(i);
        TEST_CHECK(combo_state->timestamp == 0);
        TEST_CHECK(combo_state->flags == 0x00);
        TEST_CHECK(combo_state->state == 0x00);
    }
    for (uint8_t i = 3; i < 4; i++) { // This is activated
        ComboState* combo_state = COMBO_STATE(i);
        TEST_CHECK(combo_state->timestamp == 10000);
        TEST_CHECK(combo_state->flags == FLAG_ACTIVATED);
        TEST_CHECK(combo_state->state == 0b1111);
    }

    CombosKeyBuffer ckb = get_combos_key_buffer(combos_handler);
    TEST_CHECK(ckb.count == 4);

    CombosBufferedKey* cbk1 = ckb.get(0);
    TEST_CHECK(cbk1->coords.row == 3);
    TEST_CHECK(cbk1->coords.col == 3);
    TEST_CHECK(cbk1->timestamp == 10000);
    TEST_CHECK(cbk1->part_of_active_combo == true);
    TEST_CHECK(cbk1->active_combo_index == 3);

    CombosBufferedKey* cbk2 = ckb.get(1);
    TEST_CHECK(cbk2->coords.row == 3);
    TEST_CHECK(cbk2->coords.col == 4);
    TEST_CHECK(cbk2->timestamp == 10001);
    TEST_CHECK(cbk2->part_of_active_combo == true);
    TEST_CHECK(cbk2->active_combo_index == 3);

    CombosBufferedKey* cbk3 = ckb.get(2);
    TEST_CHECK(cbk3->coords.row == 3);
    TEST_CHECK(cbk3->coords.col == 0);
    TEST_CHECK(cbk3->timestamp == 10002);
    TEST_CHECK(cbk3->part_of_active_combo == true);
    TEST_CHECK(cbk3->active_combo_index == 3);

    CombosBufferedKey* cbk4 = ckb.get(3);
    TEST_CHECK(cbk4->coords.row == 3);
    TEST_CHECK(cbk4->coords.col == 1);
    TEST_CHECK(cbk4->timestamp == 10003);
    TEST_CHECK(cbk4->part_of_active_combo == true);
    TEST_CHECK(cbk4->active_combo_index == 3);
}

// Continuation of test_combo_state_changes_5, but we release all but one key.
// The combo should be still active and the release event should not be sent
// to the keyboard yet
void test_combo_state_changes_6(void) {
    Keyboard keyboard;
    CombosHandler combos_handler(&keyboard);

    millisec now = 10000;

    combos_handler.handle_key_event(PRESS(3,3), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,4), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,1), now++);
    HID_SIZE_CHECK(1);

    combos_handler.handle_key_event(RELEASE(3,4), now++);
    HID_SIZE_CHECK(1);

    combos_handler.handle_key_event(RELEASE(3,1), now++);
    HID_SIZE_CHECK(1);

    combos_handler.handle_key_event(RELEASE(3,3), now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < 3; i++) { // These should be disabled
        ComboState* combo_state = COMBO_STATE(i);
        TEST_CHECK(combo_state->timestamp == 0);
        TEST_CHECK(combo_state->flags == 0x00);
        TEST_CHECK(combo_state->state == 0x00);
    }
    for (uint8_t i = 3; i < 4; i++) { // This is activated
        ComboState* combo_state = COMBO_STATE(i);
        TEST_CHECK(combo_state->timestamp == 10000);
        TEST_CHECK(combo_state->flags == FLAG_ACTIVATED);
        TEST_CHECK(combo_state->state == 0b1);
    }

    CombosKeyBuffer ckb = get_combos_key_buffer(combos_handler);
    TEST_CHECK(ckb.count == 1);

    CombosBufferedKey* cbk1 = ckb.get(0);
    TEST_CHECK(cbk1->coords.row == 3);
    TEST_CHECK(cbk1->coords.col == 0);
    TEST_CHECK(cbk1->timestamp == 10002);
    TEST_CHECK(cbk1->part_of_active_combo == true);
    TEST_CHECK(cbk1->active_combo_index == 3);
}

// Continuation of test_combo_state_changes_6; we release all keys.
void test_combo_state_changes_7(void) {
    Keyboard keyboard;
    CombosHandler combos_handler(&keyboard);

    millisec now = 10000;

    combos_handler.handle_key_event(PRESS(3,3), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,4), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,1), now++);
    HID_SIZE_CHECK(1);

    combos_handler.handle_key_event(RELEASE(3,4), now++);
    HID_SIZE_CHECK(1);

    combos_handler.handle_key_event(RELEASE(3,1), now++);
    HID_SIZE_CHECK(1);

    combos_handler.handle_key_event(RELEASE(3,3), now++);
    HID_SIZE_CHECK(1);

    combos_handler.handle_key_event(RELEASE(3,0), now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_D, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < COMBOS_COUNT; i++) { // These should be reset
        ComboState* combo_state = COMBO_STATE(i);
        TEST_CHECK(combo_state->timestamp == 0);
        TEST_CHECK(combo_state->flags == 0x00);
        TEST_CHECK(combo_state->state == 0x00);
    }

    CombosKeyBuffer ckb = get_combos_key_buffer(combos_handler);
    TEST_CHECK(ckb.count == 0);
}

// We press 1 combo key and then 1 non-combo key. On the second press the
// pending combo processing should abort and both keys should be sent to
// the keyboard
void test_combo_state_changes_abort(void) {
    Keyboard keyboard;
    CombosHandler combos_handler(&keyboard);

    millisec now = 10000;

    combos_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event({ P, normal_KC_A }, now++);
    HID_SIZE_CHECK(2);

    combos_handler.handle_key_event(RELEASE(3,0), now++);
    HID_SIZE_CHECK(3);

    combos_handler.handle_key_event({ R, normal_KC_A }, now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(0, { 0x00, 0x00, KC_Q, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, KC_Q, KC_A, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(2, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < COMBOS_COUNT; i++) { // These should be reset
        ComboState* combo_state = COMBO_STATE(i);
        TEST_CHECK(combo_state->timestamp == 0);
        TEST_CHECK(combo_state->flags == 0x00);
        TEST_CHECK(combo_state->state == 0x00);
    }

    CombosKeyBuffer ckb = get_combos_key_buffer(combos_handler);
    TEST_CHECK(ckb.count == 0);
}

// Abort pending combo processing after timeout, because there are no fully
// pressed combos
void test_combo_timeout_abort(void) {
    Keyboard keyboard;
    CombosHandler combos_handler(&keyboard);

    millisec now = 10000;

    combos_handler.handle_key_event(PRESS(3,1), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    // Not yet
    combos_handler.handle_key_event({ T }, now);
    HID_SIZE_CHECK(0);

    now += COMBO_ACTIVATION_TIMEOUT_MS;

    combos_handler.handle_key_event({ T }, now);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x00, 0x00, KC_W, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, KC_W, KC_Q, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < COMBOS_COUNT; i++) { // These should be reset
        ComboState* combo_state = COMBO_STATE(i);
        TEST_CHECK(combo_state->timestamp == 0);
        TEST_CHECK(combo_state->flags == 0x00);
        TEST_CHECK(combo_state->state == 0x00);
    }

    CombosKeyBuffer ckb = get_combos_key_buffer(combos_handler);
    TEST_CHECK(ckb.count == 0);

    // Release the keys - no pending combo processing, so just forward them
    // to the keyboard
    combos_handler.handle_key_event(RELEASE(3,1), now++);
    HID_SIZE_CHECK(3);

    combos_handler.handle_key_event(RELEASE(3,0), now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(2, { 0x00, 0x00, KC_Q, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Activate combo3 after timeout even if combo4 is pending as well, but
// only combo3 is fully pressed
void test_combo_timeout_activate(void) {
    Keyboard keyboard;
    CombosHandler combos_handler(&keyboard);

    millisec now = 10000;

    combos_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,1), now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,3), now++);
    HID_SIZE_CHECK(0);

    // Not yet
    combos_handler.handle_key_event({ T }, now);
    HID_SIZE_CHECK(0);

    now += COMBO_ACTIVATION_TIMEOUT_MS;

    combos_handler.handle_key_event({ T }, now);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_C, 0x00, 0x00, 0x00, 0x00, 0x00 });

    // These should be reset
    ComboState* combo_state1 = COMBO_STATE(0);
    TEST_CHECK(combo_state1->timestamp == 0);
    TEST_CHECK(combo_state1->flags == 0x00);
    TEST_CHECK(combo_state1->state == 0x00);

    ComboState* combo_state2 = COMBO_STATE(1);
    TEST_CHECK(combo_state2->timestamp == 0);
    TEST_CHECK(combo_state2->flags == 0x00);
    TEST_CHECK(combo_state2->state == 0x00);

    ComboState* combo_state3 = COMBO_STATE(3);
    TEST_CHECK(combo_state3->timestamp == 0);
    TEST_CHECK(combo_state3->flags == 0x00);
    TEST_CHECK(combo_state3->state == 0x00);

    // This should be active
    ComboState* combo_state4 = COMBO_STATE(2);
    TEST_CHECK(combo_state4->timestamp == 10000);
    TEST_CHECK(combo_state4->flags == FLAG_ACTIVATED);
    TEST_CHECK(combo_state4->state == 0b111);

    CombosKeyBuffer ckb = get_combos_key_buffer(combos_handler);
    TEST_CHECK(ckb.count == 3);

    for (uint8_t i = 0; i < ckb.count; i++) {
        CombosBufferedKey* cbk = ckb.get(i);
        TEST_CHECK(cbk->timestamp > 0);
        TEST_CHECK(cbk->part_of_active_combo == true);
        TEST_CHECK(cbk->active_combo_index == 2);
    }
}

// We activate 2 combos at once
void test_combo_multiple_active_combos(void) {
    Keyboard keyboard;
    CombosHandler combos_handler(&keyboard);

    millisec now = 10000;

    // Combo1
    combos_handler.handle_key_event(PRESS(3,5), now++);
    HID_SIZE_CHECK(0);
    combos_handler.handle_key_event(PRESS(3,6), now++);
    HID_SIZE_CHECK(1);

    // Combo2
    combos_handler.handle_key_event(PRESS(3,7), now++);
    HID_SIZE_CHECK(1);
    combos_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(2);

    BREP_COMP(0, { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x01, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });

    // Relase just a single key from each combo
    combos_handler.handle_key_event(RELEASE(3,6), now++);
    HID_SIZE_CHECK(2);
    combos_handler.handle_key_event(RELEASE(3,7), now++);
    HID_SIZE_CHECK(2);

    // Release the rest
    combos_handler.handle_key_event(RELEASE(3,5), now++);
    HID_SIZE_CHECK(3);
    combos_handler.handle_key_event(RELEASE(3,0), now++);
    HID_SIZE_CHECK(4);

    BREP_COMP(2, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(3, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// Test stuck combo key - sometimes the release event is not sent to the keyboard
// Apparently it is happening, when after activating a combo I release-then-press-again
// one of the combo keys
void test_combo_stuck_key(void) {
    Keyboard keyboard;
    CombosHandler combos_handler(&keyboard);

    millisec now = 10000;

    // We activate the combo - everything according to the plan
    combos_handler.handle_key_event(PRESS(3,7), now++);
    HID_SIZE_CHECK(0);
    combos_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(1);

    // We release one of the key - we send nothing, which is OK, because
    // the other key is still pressed
    combos_handler.handle_key_event(RELEASE(3,7), now++);
    HID_SIZE_CHECK(1);
    // We press the same key again - this triggered the bug
    combos_handler.handle_key_event(PRESS(3,7), now++);
    HID_SIZE_CHECK(1);

    // We release the last key of the combo. Note that (3,7) is still pressed,
    // but after the first release of that key we've already modified the state
    // of the combo; the second press did not change the existing active combo,
    // so after releasing (3,0) we consider it fully released.
    combos_handler.handle_key_event(RELEASE(3,0), now++);
    HID_SIZE_CHECK(2);
    combos_handler.handle_key_event(RELEASE(3,7), now++);
    HID_SIZE_CHECK(2);

    // At this point before fixing the error, HID_SIZE_CHECK(1) == true, so
    // the KC_A key is stuck = the release event was never sent to the keyboard

    BREP_COMP(0, { 0x00, 0x00, KC_A, 0x00, 0x00, 0x00, 0x00, 0x00 });
    BREP_COMP(1, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
}

// We don't start a new pending combo processing until COMBO_START_THRESHOLD_MS
// passes. The first { P, kc_no } sets the timer to 'now', so the PRESS(3,0)
// event 1 ms later does not start the combo processing; instead the event
// is passed to the Keyboard and KC_Q is pressed.
void test_combo_start_threshold(void) {
    Keyboard keyboard;
    CombosHandler combos_handler(&keyboard);

    millisec now = 10000;

    combos_handler.handle_key_event({ P, kc_no }, now++);
    HID_SIZE_CHECK(0);

    combos_handler.handle_key_event(PRESS(3,0), now++);
    HID_SIZE_CHECK(1);

    BREP_COMP(0, { 0x00, 0x00, KC_Q, 0x00, 0x00, 0x00, 0x00, 0x00 });

    for (uint8_t i = 0; i < COMBOS_COUNT; i++) {
        ComboState* combo_state = COMBO_STATE(i);

        TEST_CHECK(combo_state->timestamp == 0);
        TEST_CHECK(combo_state->flags == 0x00);
        TEST_CHECK(combo_state->state == 0x00);
    }

    CombosKeyBuffer ckb = get_combos_key_buffer(combos_handler);
    TEST_CHECK(ckb.count == 0);
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
    { "test_combo_stuck_key", test_combo_stuck_key },
    { "test_combo_start_threshold", test_combo_start_threshold },
    { NULL, NULL }
};

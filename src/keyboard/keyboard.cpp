#include "Arduino.h"
#include "HID.h"

#include "array_utils.h"
#include "dalsik_hid_descriptor.h"
#include "keyboard.h"
#include "keymap.h"
#include "extended_key_event.h"

#if DEBUG_KEYBOARD_STATE
#define PRINT_INTERNAL_STATE this->print_internal_state(event.timestamp);
#else
#define PRINT_INTERNAL_STATE
#endif

Keyboard::Keyboard(KeyMap *keymap)
    : key_press_counter(0)
    , one_shot_modifiers(0x00)
    , caps_word_enabled(false)
    , caps_word_apply_modifier(false)
    , current_reports()
    , last_reports()
    , pressed_keys()
    , last_press_key_event()
    , keymap(keymap)
{}

void Keyboard::handle_key_event(ExtendedKeyEvent event) {
    if (event.type == EVENT_KEY_PRESS) {
        this->handle_key_press(event);
    } else if (event.type == EVENT_KEY_RELEASE) {
        this->handle_key_release(event);
    } else if (event.type == EVENT_TIMEOUT) {
        this->handle_timeout(event);
    }

    this->send_hid_report();
}

void Keyboard::handle_timeout(ExtendedKeyEvent event) {
    // We need to check the last pressed key. If it is a pending dual key
    // that has timeouted, we activate the primary key
    PressedKey *pk = this->pressed_keys.get_last();
    if (pk == NULL) {
        // No key is pressed at the moment
        bool caps_word_timeout = event.timestamp - this->last_press_key_event.timestamp > CAPS_WORD_TIMEOUT_MS;
        if (this->caps_word_enabled && caps_word_timeout) {
            this->caps_word_turn_off();
        }
        return;
    }

    if (pk->state != STATE_PENDING) {
        return;
    }

    bool dual_key_timeout = event.timestamp - pk->timestamp > DUAL_MODE_TIMEOUT_MS;
    if (pk->key.is_any_dual_key() && dual_key_timeout) {
        this->press_basic_key(pk->key.clone_and_keep_code());
        pk->state = STATE_ACTIVE_CODE;

        PRINT_INTERNAL_STATE
    }
}

// Handle key press & release {{{
void Keyboard::handle_key_press(ExtendedKeyEvent event) {
    this->key_press_counter++;

    event.look_up_key(this->keymap);

    PressedKey *pk = this->pressed_keys.add(event, this->key_press_counter);
    if (pk == NULL) return;

    this->press(pk);
    this->last_press_key_event = event;

    PRINT_INTERNAL_STATE
}

void Keyboard::run_press_hooks() {
    PressedKey *pk = this->pressed_keys.get_last();
    if (pk == NULL) return;
    if (pk->state != STATE_PENDING) return;

    Key key = pk->key;

    if (key.is_any_dual_modifiers_key()) {
        this->press_basic_key(key.clone_and_keep_modifiers());
        pk->state = STATE_ACTIVE_MODIFIERS;
        this->send_hid_report();
    } else if (key.is_any_dual_layer_key() || key.type == KEY_LAYER_HOLD_OR_TOGGLE) {
        this->press_layer_hold_key(key.clone_and_keep_layer());
        pk->state = STATE_ACTIVE_LAYER;
    }
}

void Keyboard::press(PressedKey *pk) {
    Key key = pk->key;

    switch (key.type) {
        case KEY_BASIC:
            pk->state = STATE_ACTIVE_CODE;
            return this->press_basic_key(key);
        case KEY_LAYER_HOLD:
            pk->state = STATE_ACTIVE_LAYER;
            return this->press_layer_hold_key(key);
        case KEY_LAYER_TOGGLE:
            pk->state = STATE_ACTIVE_LAYER;
            return this->press_layer_toggle_key(key);
        case KEY_DUAL_MODIFIERS:
            return this->press_dual_key(pk);
        case KEY_SOLO_DUAL_MODIFIERS:
            return this->press_solo_dual_key(pk);
        case KEY_DUAL_LAYER:
            return this->press_dual_key(pk);
        case KEY_SOLO_DUAL_LAYER:
            return this->press_solo_dual_key(pk);
        case KEY_LAYER_HOLD_OR_TOGGLE:
            return this->press_layer_hold_or_toggle_key(pk);
        case KEY_TAP_HOLD_DUAL_MODIFIERS:
            return this->press_tap_hold_dual_key(pk);
        case KEY_TAP_HOLD_DUAL_LAYER:
            return this->press_tap_hold_dual_key(pk);
        case KEY_ONE_SHOT_MODIFIER:
            return this->press_one_shot_modifier_key(pk);
    }

    pk->state = STATE_ACTIVE_CODE;

    switch (key.type) {
        case KEY_TOGGLE_CAPS_WORD:
            return this->press_toggle_caps_word_key();
        case KEY_DESKTOP:
            return this->press_desktop_key(key);
        case KEY_CONSUMER:
            return this->press_consumer_key(key);
        case KEY_MOUSE_BUTTON:
            return this->press_mouse_button(pk);
    }
}

void Keyboard::handle_key_release(ExtendedKeyEvent event) {
    PressedKey *pk = this->pressed_keys.find(event.coords);
    if (pk == NULL) return;

    this->release(pk, event.timestamp);

    this->pressed_keys.remove(pk);

    PRINT_INTERNAL_STATE
}

void Keyboard::release(PressedKey *pk, millisec now) {
    Key key = pk->key;

    switch (key.type) {
        case KEY_BASIC:
            this->release_basic_key(key);
            break;
        case KEY_LAYER_HOLD:
            this->release_layer_hold_key(key);
            break;
        case KEY_LAYER_TOGGLE:
            break; // do nothing; toggle_layer key has only effect on press
        case KEY_LAYER_HOLD_OR_TOGGLE:
            this->release_layer_hold_or_toggle_key(pk);
            break;
        case KEY_ONE_SHOT_MODIFIER:
            this->release_one_shot_modifier_key(pk, now);
            break;
        case KEY_TOGGLE_CAPS_WORD:
            break;
        case KEY_DESKTOP:
            this->release_desktop_key(key);
            break;
        case KEY_CONSUMER:
            this->release_consumer_key(key);
            break;
        case KEY_MOUSE_BUTTON:
            this->release_mouse_button(pk);
            break;
        default:
            if (key.is_any_dual_key()) {
                this->release_dual_key(pk);
            }
            break;
    }

    pk->state = STATE_RELEASED;
}
// }}}

// Basic key {{{
void Keyboard::press_basic_key(Key key) {
    if (key.modifiers) {
        BIT_SET(this->current_reports.keyboard.modifiers, key.modifiers);
    }

    if (this->caps_word_enabled) {
        this->caps_word_check(key);
    }

    if (key.code) {
        ArrayUtils::append_uniq_uint8(
            this->current_reports.keyboard.keys, KEYBOARD_REPORT_KEYS, key.code
        );
    }
}

void Keyboard::release_basic_key(Key key) {
    if (key.modifiers) {
        BIT_CLEAR(this->current_reports.keyboard.modifiers, key.modifiers);
    }

    if (this->one_shot_modifiers) {
        BIT_CLEAR(this->current_reports.keyboard.modifiers, this->one_shot_modifiers);
        this->one_shot_modifiers = 0x00;
    }

    if (key.code) {
        ArrayUtils::remove_uint8(this->current_reports.keyboard.keys, KEYBOARD_REPORT_KEYS, key.code);
    }
}
// }}}

// Layer key {{{
void Keyboard::press_layer_hold_key(Key key) {
    this->keymap->activate_layer(key.layer);
}

void Keyboard::release_layer_hold_key(Key key) {
    this->keymap->deactivate_layer(key.layer);
}

void Keyboard::press_layer_toggle_key(Key key) {
    this->keymap->toggle_layer(key.layer);
}
// }}}

// Dual keys {{{
void Keyboard::press_dual_key(PressedKey *pk) {
    pk->state = STATE_PENDING;
}

void Keyboard::release_dual_key(PressedKey *pk) {
    if (pk->state == STATE_ACTIVE_CODE) {
        this->release_basic_key(pk->key.clone_and_keep_code());
    } else if (pk->state == STATE_ACTIVE_MODIFIERS) {
        this->release_basic_key(pk->key.clone_and_keep_modifiers());
    } else if (pk->state == STATE_ACTIVE_LAYER) {
        this->release_layer_hold_key(pk->key.clone_and_keep_layer());
    } else if (pk->state == STATE_PENDING) { // Tap
        Key key = pk->key.clone_and_keep_code();
        this->press_basic_key(key);
        this->send_hid_report();
        this->release_basic_key(key);
    }
}

void Keyboard::press_solo_dual_key(PressedKey *pk) {
    if (pk->key_index == 0) {
        pk->state = STATE_PENDING;
    } else {
        this->press_basic_key(pk->key.clone_and_keep_code());
        pk->state = STATE_ACTIVE_CODE;
    }
}
// }}}

// One-shot modifier key {{{
void Keyboard::press_one_shot_modifier_key(PressedKey *pk) {
    this->press_basic_key(pk->key.clone_and_keep_modifiers());
    pk->state = STATE_ACTIVE_MODIFIERS;
}

void Keyboard::release_one_shot_modifier_key(PressedKey *pk, millisec now) {
    if (pk->key_press_counter == this->key_press_counter) {
        // No other key was pressed after this one, act as one-shot modifier

        if (pk->timestamp + ONE_SHOT_MODIFIER_TAP_TIMEOUT_MS > now) {
            uint8_t modifiers = pk->key.modifiers;

            // Toggle OSM
            if (this->one_shot_modifiers & modifiers) {
                BIT_CLEAR(this->one_shot_modifiers, modifiers);
                BIT_CLEAR(this->current_reports.keyboard.modifiers, modifiers);
            } else {
                BIT_SET(this->one_shot_modifiers, modifiers);
                BIT_SET(this->current_reports.keyboard.modifiers, modifiers);
            }
        } else {
            this->release_basic_key(pk->key.clone_and_keep_modifiers());
        }
    } else {
        this->release_basic_key(pk->key.clone_and_keep_modifiers());
    }
}
// }}}

// Desktop key {{{
void Keyboard::press_desktop_key(Key key) {
    this->caps_word_turn_off();

    this->current_reports.desktop.key = key.code;
}

void Keyboard::release_desktop_key(Key key) {
    if (this->current_reports.desktop.key == key.code) {
        this->current_reports.desktop.key = 0x00;
    }
}
// }}}

// Consumer key {{{
void Keyboard::press_consumer_key(Key key) {
    this->caps_word_turn_off();

    uint16_t consumer_key = (key.code << 8) | key.modifiers;
    this->current_reports.consumer.key = consumer_key;
}

void Keyboard::release_consumer_key(Key key) {
    uint16_t consumer_key = (key.code << 8) | key.modifiers;

    if (this->current_reports.consumer.key == consumer_key) {
        this->current_reports.consumer.key = 0x00;
    }
}
// }}}

// Layer hold or toggle {{{
void Keyboard::press_layer_hold_or_toggle_key(PressedKey *pk) {
    pk->state = STATE_PENDING;
}

void Keyboard::release_layer_hold_or_toggle_key(PressedKey *pk) {
    if (
        // We have not decided what to do with this key, but no other keys
        // were pressed after this - let's toggle a layer
        pk->state == STATE_PENDING
        && pk->key_press_counter == this->key_press_counter
    ) {
        this->keymap->toggle_layer(pk->key.layer);
    } else {
        this->keymap->deactivate_layer(pk->key.layer);
    }
}
// }}}

// Mouse {{{
void Keyboard::press_mouse_button(PressedKey *pk) {
    BIT_SET(this->current_reports.mouse.buttons, pk->key.code);
}
void Keyboard::release_mouse_button(PressedKey *pk) {
    BIT_CLEAR(this->current_reports.mouse.buttons, pk->key.code);
}
// }}}

// Caps Word {{{
void Keyboard::press_toggle_caps_word_key() {
    this->caps_word_toggle();
}

void Keyboard::caps_word_turn_off() {
    if (this->caps_word_enabled) {
        this->caps_word_toggle();
    }
}

void Keyboard::caps_word_toggle() {
    this->caps_word_enabled = !this->caps_word_enabled;
#ifdef REPORT_CAPS_WORD_CHANGE
    Serial.print("CW:");
    Serial.print(this->caps_word_enabled, HEX);
    Serial.print("\n");
#endif
}

void Keyboard::caps_word_check(Key key) {
    uint8_t code = key.code;
    uint8_t mod = key.modifiers;

    uint8_t mods_except_shift = BIT_CLEAR(mod, MOD_RAW_LSHIFT | MOD_RAW_RSHIFT);

    if (code == KC_NO) {
        // Pressing LSHIFT/RSHIFT keeps caps_word on. If we press any other
        // mod, we turn it off.
        if (mods_except_shift) {
            this->caps_word_turn_off();
        }
    } else if ((KC_A <= code && code <= KC_Z) || code == KC_MINUS) {
        // Keys A-Z + `-` keeps caps_word on (if pressed without mods other
        // than LSHIFT/RSHIFT) and we apply LSHIFT to them (`-` becomes `_`)
        if (mods_except_shift) {
            this->caps_word_turn_off();
        } else {
            this->caps_word_apply_modifier = true;
        }
    } else if (
        (KC_1 <= code && code <= KC_0)
        || code == KC_BACKSPACE
        || code == KC_DELETE
    ) {
        // Keys 1-9,0 + BACKSPACE + DELETE keeps caps_word on (if pressed
        // without other mods - e.g. `LSHIFT(KC_1)` turns it off), but we don't
        // apply LSHIFT to them - we want to keep numbers as numbers.
        if (mod) {
            this->caps_word_turn_off();
        } else {
            this->caps_word_apply_modifier = false;
        }
    } else {
        this->caps_word_turn_off();
    }
}
// }}}

void Keyboard::press_tap_hold_dual_key(PressedKey *pk) {
    // If the last key event was the same as the current one within the threshold,
    // we immediately trigger the normal key.
    if (
        this->last_press_key_event.key.equals(pk->key)
        && pk->timestamp - this->last_press_key_event.timestamp < DUAL_TAP_HOLD_THRESHOLD_MS
    ) {
        this->press_basic_key(pk->key.clone_and_keep_code());
        pk->state = STATE_ACTIVE_CODE;
    } else {
        pk->state = STATE_PENDING;
    }
}

void Keyboard::send_hid_report() {
    this->send_keyboard_hid_report();
    this->send_desktop_hid_report();
    this->send_consumer_hid_report();
    this->send_mouse_hid_report();
}

void Keyboard::send_keyboard_hid_report() {
    size_t report_size = sizeof(KeyboardHIDReport);
    KeyboardHIDReport* current_report = &(this->current_reports.keyboard);
    KeyboardHIDReport* last_report = &(this->last_reports.keyboard);

    if (memcmp(current_report, last_report, report_size) == 0) {
        return;
    }

#ifdef REPORT_MODIFIER_CHANGE
    if (current_report->modifiers != last_report->modifiers) {
        Serial.print("M:");
        Serial.print(current_report->modifiers, HEX);
        Serial.print("\n");
    }
#endif

    uint8_t current_modifiers = current_report->modifiers;

    if (this->caps_word_apply_modifier) {
        BIT_SET(current_report->modifiers, MOD_RAW_LSHIFT);
    }

#if DEBUG_KEYREPORT_KEYBOARD
    Serial.print(F("Keyboard:"));
    Serial.print(this->current_reports.keyboard.modifiers, HEX);
    Serial.print(F("|"));
    Serial.print(this->current_reports.keyboard.reserved, HEX);
    for (uint8_t i = 0; i < 6; i++) {
        Serial.print(F("|"));
        Serial.print(this->current_reports.keyboard.keys[i], HEX);
    }
    Serial.print(F("\n"));
#endif

    HID().SendReport(KEYBOARD_REPORT_ID, current_report, report_size);

    if (this->caps_word_apply_modifier) {
        this->caps_word_apply_modifier = false;
        current_report->modifiers = current_modifiers;
    }

    *last_report = *current_report;
}

void Keyboard::send_desktop_hid_report() {
    size_t report_size = sizeof(DesktopHIDReport);
    DesktopHIDReport* current_report = &(this->current_reports.desktop);
    DesktopHIDReport* last_report = &(this->last_reports.desktop);

    if (memcmp(current_report, last_report, report_size) == 0) {
        return;
    }

#if DEBUG_KEYREPORT_DESKTOP
    Serial.print(F("Desktop:"));
    Serial.print(this->current_reports.desktop.key, HEX);
    Serial.print(F("\n"));
#endif

    HID().SendReport(DESKTOP_REPORT_ID, current_report, report_size);
    *last_report = *current_report;
}

void Keyboard::send_consumer_hid_report() {
    size_t report_size = sizeof(ConsumerHIDReport);
    ConsumerHIDReport* current_report = &(this->current_reports.consumer);
    ConsumerHIDReport* last_report = &(this->last_reports.consumer);

    if (memcmp(current_report, last_report, report_size) == 0) {
        return;
    }

#if DEBUG_KEYREPORT_CONSUMER
    Serial.print(F("Consumer:"));
    Serial.print(this->current_reports.consumer.key, HEX);
    Serial.print(F("\n"));
#endif

    HID().SendReport(CONSUMER_REPORT_ID, current_report, report_size);
    *last_report = *current_report;
}

void Keyboard::send_mouse_hid_report() {
    size_t report_size = sizeof(MouseHIDReport);
    MouseHIDReport* current_report = &(this->current_reports.mouse);
    MouseHIDReport* last_report = &(this->last_reports.mouse);

    if (memcmp(current_report, last_report, report_size)) {
        HID().SendReport(MOUSE_REPORT_ID, current_report, report_size);
        *last_report = *current_report;
    }
}

void Keyboard::print_internal_state(millisec now) {
    Serial.print("=========================PRESSED_KEYS\n");
    Serial.print("  -count:");
    Serial.print(this->pressed_keys.count);
    Serial.print(" this->key_press_counter:");
    Serial.print(this->key_press_counter);
    Serial.print(" KeyMap:");
    this->keymap->print_internal_state();
    Serial.print("\n  -caps_word_enabled:");
    Serial.print(this->caps_word_enabled);
    Serial.print(" caps_word_apply_modifier:");
    Serial.print(this->caps_word_apply_modifier);
    Serial.print("\n");
    this->pressed_keys.print_internal_state(now);
}

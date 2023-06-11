#include "Arduino.h"
#include "HID.h"

#include "array_utils.h"
#include "dalsik_hid_descriptor.h"
#include "keyboard.h"
#include "keymap.h"
#include "key_event.h"

#if DEBUG_KEYBOARD_STATE
#define PRINT_INTERNAL_STATE this->print_internal_state(event.timestamp);
#else
#define PRINT_INTERNAL_STATE
#endif

Keyboard::Keyboard(KeyMap *keymap) {
    static HIDSubDescriptor node(KEYBOARD_HID_DESCRIPTOR, sizeof(KEYBOARD_HID_DESCRIPTOR));
    HID().AppendDescriptor(&node);

    this->keymap = keymap;
    this->current_hid_reports = HIDReports {};
    this->last_hid_reports = HIDReports {};
    this->key_press_counter = 0;
    this->one_shot_modifiers = 0x00;
    this->caps_word_enabled = false;
    this->caps_word_apply_modifier = false;
    this->pressed_keys = PressedKeys();
    this->last_press_key_event = KeyEvent {};
}

void Keyboard::handle_key_event(KeyEvent event) {
    if (event.type == EVENT_KEY_PRESS) {
        this->handle_key_press(event);
    } else if (event.type == EVENT_KEY_RELEASE) {
        this->handle_key_release(event);
    } else if (event.type == EVENT_TIMEOUT) {
        this->handle_timeout(event);
    }
}

inline void Keyboard::handle_timeout(KeyEvent event) {
#ifdef CAPS_WORD_TIMEOUT
    if (this->caps_word_enabled && this->pressed_keys.is_empty()) {
        if (event.timestamp - this->pressed_keys.last_press > CAPS_WORD_TIMEOUT) {
            this->caps_word_turn_off();
        }
    }
#endif

    if (this->pressed_keys.is_empty()) {
        return; // No key is pressed
    }

    // We need to check the last pressed key. If it is a pending dual key
    // that has timeouted, we activate the primary key
    PressedKey *pk = this->pressed_keys.get_last();

    if (pk->state != STATE_PENDING) {
        return;
    }

    if (
        pk->key_info.is_any_dual_key()
        && pk->timestamp + DUAL_MODE_TIMEOUT_MS < event.timestamp
    ) {
        pk->state = STATE_ACTIVE_KEY;
        this->press_normal_key(pk->key_info.use_key());
        this->send_hid_report();

        PRINT_INTERNAL_STATE
    }
}

void Keyboard::reload_keys_on_new_layer(PressedKey *pk) {
    if (pk->key_info.skip_layer_reload()) return; // Missing coords info
    pk->key_info = this->keymap->get_key(pk->key_info.coords);
}

// Handle key press & release {{{
inline void Keyboard::handle_key_press(KeyEvent event) {
    this->key_press_counter++;

    if (!event.key_looked_up) {
        event.key_info = this->keymap->get_key(event.coords);
        event.key_looked_up = true;
    }

    PressedKey *pk = this->pressed_keys.add(event, this->key_press_counter);
    if (pk == NULL) return;

    if (this->pressed_keys.count > 1) { // Not the first pressed key
        this->run_press_hooks(pk);
    }

    this->press(pk);
    this->last_press_key_event = event;
    this->send_hid_report();

    PRINT_INTERNAL_STATE
}

inline void Keyboard::handle_key_release(KeyEvent event) {
    PressedKey *pk = this->pressed_keys.find(event.coords);
    if (pk == NULL) return;

    this->release(pk, event.timestamp);
    this->send_hid_report();

    this->pressed_keys.remove(pk);

    PRINT_INTERNAL_STATE
}

void Keyboard::press(PressedKey *pk) {
    KeyInfo key_info = pk->key_info;

    if (key_info.is_any_dual_mod_key()) {
        this->press_dual_mod_key(pk);
    } else if (key_info.is_any_dual_layer_key()) {
        this->press_dual_layer_key(pk);
    } else if (key_info.type == KEY_ONE_SHOT_MODIFIER) {
        this->press_one_shot_modifier_key(pk);
    } else if (key_info.type == KEY_LAYER_TOGGLE_OR_HOLD) {
        this->press_layer_toggle_or_hold(pk);
    } else if (key_info.type == KEY_MOUSE) {
        this->press_mouse_button(pk);
    } else if (key_info.type == KEY_LAYER_PRESS) {
        this->press_layer_key(key_info.layer);
        pk->state = STATE_ACTIVE_LAYER;
    } else if (key_info.type == KEY_LAYER_TOGGLE) {
        this->press_toggle_layer_key(key_info.key);
        pk->state = STATE_ACTIVE_LAYER;
    } else {
        pk->state = STATE_ACTIVE_KEY;

        if (key_info.type == KEY_NORMAL) {
            this->press_normal_key(key_info);
        } else if (key_info.type == KEY_SYSTEM)  {
            this->press_system_key(key_info);
        } else if (key_info.is_multimedia_key()) {
            this->press_multimedia_key(key_info);
        } else if (key_info.type == KEY_TOGGLE_CAPS_WORD) {
            this->press_toggle_caps_word();
        }
    }
}

void Keyboard::release(PressedKey *pk, millisec now) {
    KeyInfo key_info = pk->key_info;

    if (key_info.is_any_dual_mod_key()) {
        this->release_dual_mod_key(pk);
    } else if (key_info.is_any_dual_layer_key()) {
        this->release_dual_layer_key(pk);
    } else if (key_info.type == KEY_ONE_SHOT_MODIFIER) {
        this->release_one_shot_modifier_key(pk, now);
    } else if (key_info.type == KEY_LAYER_TOGGLE_OR_HOLD) {
        this->release_layer_toggle_or_hold(pk);
    } else if (key_info.type == KEY_MOUSE) {
        this->release_mouse_button(pk);
    } else {
        pk->state = STATE_RELEASED;

        if (key_info.type == KEY_NORMAL) {
            this->release_normal_key(key_info);
        } else if (key_info.type == KEY_LAYER_PRESS) {
            this->release_layer_key(key_info.layer);
        } else if (key_info.type == KEY_LAYER_TOGGLE) {
            // do nothing; toggle_layer key has only effect on press
        } else if (key_info.type == KEY_SYSTEM)  {
            this->release_system_key(key_info);
        } else if (key_info.is_multimedia_key()) {
            this->release_multimedia_key(key_info);
        } else if (key_info.type == KEY_TOGGLE_CAPS_WORD) {
        }
    }
}
// }}}

// Press & Release hooks {{{
inline void Keyboard::run_press_hooks(PressedKey* new_pk) {
    PressedKey *pk = this->pressed_keys.get_before(new_pk);
    if (pk == NULL) return;
    if (pk->state != STATE_PENDING) return;

    KeyInfo key_info = pk->key_info;

    if (key_info.is_any_dual_mod_key()) {
        this->press_dual_mod_key(pk);
        this->send_hid_report();
    } else if (key_info.is_any_dual_layer_key()) {
        this->press_dual_layer_key(pk);
        this->reload_keys_on_new_layer(new_pk);
    } else if (pk->key_info.type == KEY_LAYER_TOGGLE_OR_HOLD) {
        this->press_layer_toggle_or_hold(pk);
        this->reload_keys_on_new_layer(new_pk);
    }
}
// }}}

// Normal key {{{
inline void Keyboard::press_normal_key(KeyInfo key_info) {
    if (key_info.mod) {
        BIT_SET(this->current_hid_reports.base.modifiers, key_info.mod);
    }

    if (this->caps_word_enabled) {
        this->caps_word_check(key_info);
    }

    if (key_info.key) {
        ArrayUtils::append_uniq_uint8(
            this->current_hid_reports.base.keys, BASE_HID_REPORT_KEYS, key_info.key
        );
    }
}

inline void Keyboard::release_normal_key(KeyInfo key_info) {
    if (key_info.mod) {
        BIT_CLEAR(this->current_hid_reports.base.modifiers, key_info.mod);
    }

    if (this->one_shot_modifiers) {
        BIT_CLEAR(this->current_hid_reports.base.modifiers, this->one_shot_modifiers);
        this->one_shot_modifiers = 0x00;
    }

    if (key_info.key) {
        ArrayUtils::remove_and_return_last_uint8(
            this->current_hid_reports.base.keys, BASE_HID_REPORT_KEYS, key_info.key
        );
    }
}
// }}}
// One-shot modifier key {{{
inline void Keyboard::press_one_shot_modifier_key(PressedKey *pk) {
    this->press_normal_key(pk->key_info.use_mod());
    pk->state = STATE_ACTIVE_MODIFIER;
}

inline void Keyboard::release_one_shot_modifier_key(PressedKey *pk, millisec now) {
    if (pk->key_press_counter == this->key_press_counter) {
        // No other key was pressed after this one, act as one-shot modifier

        uint8_t modifier = pk->key_info.mod;

        if (pk->timestamp + ONE_SHOT_MODIFIER_TAP_TIMEOUT_MS > now) {
            // Toggle OSM
            if (this->one_shot_modifiers & modifier) {
                BIT_CLEAR(this->one_shot_modifiers, modifier);
                BIT_CLEAR(this->current_hid_reports.base.modifiers, modifier);
            } else {
                BIT_SET(this->one_shot_modifiers, modifier);
                BIT_SET(this->current_hid_reports.base.modifiers, modifier);
            }
        } else {
            this->release_normal_key(pk->key_info.use_mod());
        }
    } else {
        this->release_normal_key(pk->key_info.use_mod());
    }
}
// }}}
// Layer key {{{
inline void Keyboard::press_layer_key(uint8_t layer) {
    this->keymap->activate_layer(layer);
}

inline void Keyboard::release_layer_key(uint8_t layer) {
    this->keymap->deactivate_layer(layer);
}
// }}}
// Layer toggle key {{{
inline void Keyboard::press_toggle_layer_key(uint8_t layer) {
    this->keymap->toggle_layer(layer);
}
// }}}
// System key {{{
inline void Keyboard::press_system_key(KeyInfo key_info) {
    this->caps_word_turn_off();

    this->current_hid_reports.system.key = key_info.key;
}

inline void Keyboard::release_system_key(KeyInfo key_info) {
    if (this->current_hid_reports.system.key == key_info.key) {
        this->current_hid_reports.system.key = 0x00;
    }
}
// }}}
// Multimedia key {{{
inline void Keyboard::press_multimedia_key(KeyInfo key_info) {
    this->caps_word_turn_off();

    this->current_hid_reports.multimedia.key = key_info.key;
    if (key_info.type == KEY_MULTIMEDIA_2) {
        this->current_hid_reports.multimedia.prefix = 0x02;
    } else if (key_info.type == KEY_MULTIMEDIA_1) {
        this->current_hid_reports.multimedia.prefix = 0x01;
    } else {
        this->current_hid_reports.multimedia.prefix = 0x00;
    }
}

inline void Keyboard::release_multimedia_key(KeyInfo key_info) {
    uint8_t key = key_info.key;
    uint8_t prefix = 0x00;
    if (key_info.type == KEY_MULTIMEDIA_1) {
        prefix = 0x01;
    } else if (key_info.type == KEY_MULTIMEDIA_2) {
        prefix = 0x02;
    }

    if (
        this->current_hid_reports.multimedia.prefix == prefix
        && this->current_hid_reports.multimedia.key == key
    ) {
        memset(&(this->current_hid_reports.multimedia), 0, sizeof(MultimediaHIDReport));
    }
}
// }}}
// Dual key {{{
inline void Keyboard::press_dual_mod_key(PressedKey *pk) {
    if (pk->state == STATE_NOT_PROCESSED) {
        if (pk->key_index > 0 && pk->key_info.is_any_solo_dual_key()) {
            // Not the first key
            this->press_normal_key(pk->key_info.use_key());
            pk->state = STATE_ACTIVE_KEY;
        } else if (pk->key_info.type == KEY_DUAL_DTH_MOD) {
            this->press_dual_dth_key(pk);
        } else {
            pk->state = STATE_PENDING;
        }
    } else if (pk->state == STATE_PENDING) {
        // The primary key is activated only in release_dual_mod_key
        this->press_normal_key(pk->key_info.use_mod());
        pk->state = STATE_ACTIVE_MODIFIER;
    }
}

inline void Keyboard::release_dual_mod_key(PressedKey *pk) {
    KeyInfo key_info = pk->key_info;

    if (pk->state == STATE_ACTIVE_KEY) {
        this->release_normal_key(key_info.use_key());
        pk->state = STATE_RELEASED;
    } else if (pk->state == STATE_ACTIVE_MODIFIER) {
        this->release_normal_key(key_info.use_mod());
        pk->state = STATE_RELEASED;
    } else if (pk->state == STATE_PENDING) {
        // If there were other key pressed after this dual_mod key, Then
        // the `run_press_hooks` method would have called `press_dual_mod_key`
        // and thus the state of this key would be `STATE_ACTIVE_MODIFIER`.
        // That is why here we can assume, that this dual_mod key was tapped,
        // so we must send/release it as a normal key.
        KeyInfo ki = key_info.use_key();
        this->press_normal_key(ki);
        this->send_hid_report();
        this->release_normal_key(ki);
        pk->state = STATE_RELEASED;
    }
}
// }}}

// Dual layer key {{{
inline void Keyboard::press_dual_layer_key(PressedKey *pk) {
    if (pk->state == STATE_NOT_PROCESSED) {
        if (pk->key_index > 0 && pk->key_info.is_any_solo_dual_key()) {
            // Not the first key
            this->press_normal_key(pk->key_info.use_key());
            pk->state = STATE_ACTIVE_KEY;
        } else if (pk->key_info.type == KEY_DUAL_DTH_LAYER) {
            this->press_dual_dth_key(pk);
        } else {
            pk->state = STATE_PENDING;
        }
    } else if (pk->state == STATE_PENDING) {
        // The primary key is activated only in release_dual_layer_key
        this->press_layer_key(pk->key_info.layer);
        pk->state = STATE_ACTIVE_LAYER;
    }
}

inline void Keyboard::release_dual_layer_key(PressedKey *pk) {
    KeyInfo key_info = pk->key_info;

    if (pk->state == STATE_ACTIVE_KEY) {
        this->release_normal_key(key_info.use_key());
        pk->state = STATE_RELEASED;
    } else if (pk->state == STATE_ACTIVE_LAYER) {
        this->release_layer_key(key_info.layer);
        pk->state = STATE_RELEASED;
    } else if (pk->state == STATE_PENDING) {
        KeyInfo ki = key_info.use_key();
        this->press_normal_key(ki);
        this->send_hid_report();
        this->release_normal_key(ki);
        pk->state = STATE_RELEASED;
    }
}
// }}}

// Layer toggle or hold {{{
inline void Keyboard::press_layer_toggle_or_hold(PressedKey *pk) {
    if (pk->state == STATE_NOT_PROCESSED) {
        pk->state = STATE_PENDING;
    } else if (pk->state == STATE_PENDING) {
        this->press_layer_key(pk->key_info.layer);
        pk->state = STATE_ACTIVE_LAYER;
    }
}
inline void Keyboard::release_layer_toggle_or_hold(PressedKey *pk) {
    if (
        // We have not decided what to do with this key, but no other keys
        // were pressed after this - let's toggle a layer
        pk->state == STATE_PENDING
        && pk->key_press_counter == this->key_press_counter
    ) {
        this->keymap->toggle_layer(pk->key_info.layer);
    } else {
        this->keymap->deactivate_layer(pk->key_info.layer);
    }
    pk->state = STATE_RELEASED;
}
// }}}

// Mouse {{{
inline void Keyboard::press_mouse_button(PressedKey *pk) {
    BIT_SET(this->current_hid_reports.mouse.buttons, pk->key_info.key);
}
inline void Keyboard::release_mouse_button(PressedKey *pk) {
    BIT_CLEAR(this->current_hid_reports.mouse.buttons, pk->key_info.key);
}
// }}}

// Caps Word {{{
inline void Keyboard::press_toggle_caps_word() {
    this->caps_word_toggle();
}

inline void Keyboard::caps_word_turn_off() {
    if (this->caps_word_enabled) {
        this->caps_word_toggle();
    }
}

inline void Keyboard::caps_word_toggle() {
    this->caps_word_enabled = !this->caps_word_enabled;
#ifdef REPORT_CAPS_WORD_CHANGE
    Serial.print("CW:");
    Serial.print(this->caps_word_enabled, HEX);
    Serial.print("\n");
#endif
}

inline void Keyboard::caps_word_check(KeyInfo key_info) {
    uint8_t key = key_info.key;
    uint8_t mod = key_info.mod;

    uint8_t mod_without_shift = BIT_CLEAR(mod, MOD_RAW_LSHIFT | MOD_RAW_RSHIFT);

    if (key == KC_NO) {
        // Pressing LSHIFT/RSHIFT keeps caps_word on. If we press any other
        // mod, we turn it off.
        if (mod_without_shift) {
            this->caps_word_turn_off();
        }
    } else if ((KC_A <= key && key <= KC_Z) || key == KC_MINUS) {
        // Keys A-Z + `-` keeps caps_word on (if pressed without mods other
        // than LSHIFT/RSHIFT) and we apply LSHIFT to them (`-` becomes `_`)
        if (mod_without_shift) {
            this->caps_word_turn_off();
        } else {
            this->caps_word_apply_modifier = true;
        }
    } else if (
        (KC_1 <= key && key <= KC_0)
        || key == KC_BACKSPACE
        || key == KC_DELETE
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

inline void Keyboard::press_dual_dth_key(PressedKey *pk) {
    // If the last key event was the same as the current one within the threshold,
    // we immediately trigger the normal key.
    if (
        this->last_press_key_event.key_info.equals(pk->key_info)
        && pk->timestamp - this->last_press_key_event.timestamp < DUAL_TAP_HOLD_THRESHOLD_MS
    ) {
        this->press_normal_key(pk->key_info.use_key());
        pk->state = STATE_ACTIVE_KEY;
    } else {
        pk->state = STATE_PENDING;
    }
}

inline void Keyboard::send_hid_report() {
    // Base
    size_t base_size = sizeof(BaseHIDReport);
    BaseHIDReport* base = &(this->current_hid_reports.base);
    BaseHIDReport* last_base = &(this->last_hid_reports.base);

    if (memcmp(base, last_base, base_size)) {
#ifdef REPORT_MODIFIER_CHANGE
        if (base->modifiers != last_base->modifiers) {
            Serial.print("M:");
            Serial.print(this->current_hid_reports.base.modifiers, HEX);
            Serial.print("\n");
        }
#endif

        uint8_t current_modifiers = base->modifiers;

        if (this->caps_word_apply_modifier) {
            BIT_SET(base->modifiers, MOD_RAW_LSHIFT);
        }

        this->print_base_report_to_serial();
        HID().SendReport(BASE_KEYBOARD_REPORT_ID, base, base_size);

        if (this->caps_word_apply_modifier) {
            this->caps_word_apply_modifier = false;
            base->modifiers = current_modifiers;
        }

        *last_base = *base;
    }

    // System
    size_t system_size = sizeof(SystemHIDReport);
    SystemHIDReport* system = &(this->current_hid_reports.system);
    SystemHIDReport* last_system = &(this->last_hid_reports.system);

    if (memcmp(system, last_system, system_size)) {
        this->print_system_report_to_serial();
        HID().SendReport(SYSTEM_KEYBOARD_REPORT_ID, system, system_size);
        *last_system = *system;
    }

    // Multimedia
    size_t multimedia_size = sizeof(MultimediaHIDReport);
    MultimediaHIDReport* multimedia = &(this->current_hid_reports.multimedia);
    MultimediaHIDReport* last_multimedia = &(this->last_hid_reports.multimedia);

    if (memcmp(multimedia, last_multimedia, multimedia_size)) {
        this->print_multimedia_report_to_serial();
        HID().SendReport(MULTIMEDIA_KEYBOARD_REPORT_ID, multimedia, multimedia_size);
        *last_multimedia = *multimedia;
    }

    // Mouse
    size_t mouse_size = sizeof(MouseHIDReport);
    MouseHIDReport* mouse = &(this->current_hid_reports.mouse);
    MouseHIDReport* last_mouse = &(this->last_hid_reports.mouse);

    if (memcmp(mouse, last_mouse, mouse_size)) {
        HID().SendReport(MOUSE_REPORT_ID, mouse, mouse_size);
        *last_mouse = *mouse;
    }
}

void Keyboard::print_base_report_to_serial() {
#if DEBUG_KEYREPORT_BASE
    Serial.print(F("Keyboard[BASE]:"));
    Serial.print(this->current_hid_reports.base.modifiers, HEX);
    Serial.print(F("|"));
    Serial.print(this->current_hid_reports.base.reserved, HEX);
    for (uint8_t i = 0; i < 6; i++) {
        Serial.print(F("|"));
        Serial.print(this->current_hid_reports.base.keys[i], HEX);
    }
    Serial.print(F("\n"));
#endif
}

void Keyboard::print_system_report_to_serial() {
#if DEBUG_KEYREPORT_SYSTEM
    Serial.print(F("Keyboard[SYSTEM]:"));
    Serial.print(this->current_hid_reports.system.key, HEX);
    Serial.print(F("\n"));
#endif
}

void Keyboard::print_multimedia_report_to_serial() {
#if DEBUG_KEYREPORT_MULTIMEDIA
    Serial.print(F("Keyboard[MULTIMEDIA]:"));
    Serial.print(this->current_hid_reports.multimedia.prefix, HEX);
    Serial.print(F("|"));
    Serial.print(this->current_hid_reports.multimedia.key, HEX);
    Serial.print(F("\n"));
#endif
}

void Keyboard::print_internal_state(millisec now) {
    Serial.print("=========================PRESSED_KEYS\n");
    Serial.print("  -count:");
    Serial.print(this->pressed_keys.count);
    Serial.print(" last_press:");
    Serial.print(this->pressed_keys.last_press);
    Serial.print(" now-last_press:");
    Serial.print(now - this->pressed_keys.last_press);
    Serial.print(" this->key_press_counter:");
    Serial.print(this->key_press_counter);
    Serial.print(" KeyMap:");
    this->keymap->print_internal_state();
    Serial.print("\n  -caps_word_enabled:");
    Serial.print(this->caps_word_enabled);
    Serial.print(" caps_word_apply_modifier:");
    Serial.print(this->caps_word_apply_modifier);
    Serial.print("\n");
    for (uint8_t i = 0; i < this->pressed_keys.count; i++) {
        PressedKey* pk = this->pressed_keys.get(i);
        KeyInfo key_info = pk->key_info;
        uint8_t state = pk->state;

        Serial.print("  -[");
        Serial.print(i);
        Serial.print("/");
        Serial.print(pk->key_index);
        Serial.print("] - KeyInfo[T:");
        Serial.print(key_info.type);
        Serial.print(",L:");
        Serial.print(key_info.layer);
        Serial.print(",M:");
        Serial.print(key_info.mod);
        Serial.print(",K:");
        Serial.print(key_info.key);
        Serial.print(",R:");
        Serial.print(key_info.coords.row);
        Serial.print(",C:");
        Serial.print(key_info.coords.col);
        Serial.print("]\n       - key_press_counter:");
        Serial.print(pk->key_press_counter);
        Serial.print("\n        - timestamp:");
        Serial.print(pk->timestamp);
        Serial.print(" now-timestamp:");
        Serial.print(now - pk->timestamp);
        Serial.print("\n        - state:");
        if (state == STATE_NOT_PROCESSED) {
            Serial.print("STATE_NOT_PROCESSED");
        } else if (state == STATE_PENDING) {
            Serial.print("STATE_PENDING");
        } else if (state == STATE_ACTIVE_LAYER) {
            Serial.print("STATE_ACTIVE_LAYER");
        } else if (state == STATE_ACTIVE_MODIFIER) {
            Serial.print("STATE_ACTIVE_MODIFIER");
        } else if (state == STATE_ACTIVE_KEY) {
            Serial.print("STATE_ACTIVE_KEY");
        } else if (state == STATE_RELEASED) {
            Serial.print("STATE_RELEASED");
        }
        Serial.print("\n");
    }
}

#include "Arduino.h"
#include "array_utils.h"
#include "dalsik_eeprom.h"
#include "dalsik_global.h"
#include "dalsik_hid.h"
#include "dalsik_led.h"
#include "key_info.h"
#include "keyboard.h"

Keyboard::Keyboard() {
    DalsikHid::init_descriptor();

    this->toggled_layer_index = 0;
    this->clear();

    memset(&(this->last_base_hid_report), 0, sizeof(BaseHIDReport));
    memset(&(this->last_system_hid_report), 0, sizeof(SystemHIDReport));
    memset(&(this->last_multimedia_hid_report), 0, sizeof(MultimediaHIDReport));
    this->key_press_counter = 0;
    this->one_shot_modifiers = 0x00;
}

void Keyboard::clear() {
    memset(this->layer_history, 0, sizeof(this->layer_history));
    memset(&(this->pressed_keys), 0, sizeof(PressedKeys));
    memset(&(this->base_hid_report), 0, sizeof(BaseHIDReport));
    memset(&(this->system_hid_report), 0, sizeof(SystemHIDReport));
    memset(&(this->multimedia_hid_report), 0, sizeof(MultimediaHIDReport));

    this->layer_index = 0;
    if (this->toggled_layer_index > 0) {
        this->set_layer(this->toggled_layer_index);
    }
}

void Keyboard::handle_changed_key(ChangedKeyEvent event, millisec now) {
    KeyInfo key_info = this->get_key(event.coords);

    if (event.type == EVENT_KEY_PRESS) {
        this->handle_key_press(key_info, now);
    } else if (event.type == EVENT_KEY_RELEASE) {
        this->handle_key_release(key_info, now);
    }

#if DEBUG_KEYBOARD_STATE
    this->print_internal_state();
#endif
}

void Keyboard::key_timeout_check(millisec now) {
    if (this->pressed_keys.count == 0) {
        return; // No key is pressed
    }

    // We need to check the last pressed key. If it is a pending dual key
    // that has timeouted, we activate the primary key
    PressedKey *pk = &(this->pressed_keys.keys[this->pressed_keys.count-1]);

    if (pk->state != STATE_PENDING) {
        return;
    }

    if (
        pk->key_info.is_any_dual_key()
        && pk->timestamp + DUAL_MODE_TIMEOUT_MS < now
    ) {
        pk->state = STATE_PRIMARY_KEY;
        this->press_normal_key(pk->key_info);
        this->send_hid_report();
    }
    if (
        pk->key_info.type == KEY_ONE_SHOT_MODIFIER
        && pk->timestamp + ONE_SHOT_MODIFIER_TIMEOUT_MS < now
    ) {
        this->press_one_shot_modifier_key(pk);
        this->send_hid_report();
    }
}

KeyInfo Keyboard::get_key(KeyCoords c) {
    KeyInfo key_info = EEPROM::get_key(this->layer_index, c);

    if (key_info.type == KEY_TRANSPARENT) { // Get the key from lower layers
        key_info = this->get_non_transparent_key(c);
    }

    return key_info;
}

KeyInfo Keyboard::get_non_transparent_key(KeyCoords c) {
    for (int8_t i = LAYER_HISTORY_CAPACITY-1; i >= 0; i--) {
        uint8_t layer = this->layer_history[i];
        if (layer == 0x00) {
            continue;
        }

        KeyInfo key_info = EEPROM::get_key(layer, c);
        if (key_info.type != KEY_TRANSPARENT) {
            return key_info;
        }
    }

    KeyInfo key_info = EEPROM::get_key(0, c);
    if (key_info.type == KEY_TRANSPARENT) {
        return KeyInfo(KEY_UNSET, 0x00, c);
    } else {
        return key_info;
    }
}

void Keyboard::reload_key_by_coords(KeyInfo* ki) {
    if (ki->has_no_coords()) {
        return; // Missing coords info
    }

    KeyInfo new_ki = this->get_key(ki->coords);
    ki->type = new_ki.type;
    ki->key = new_ki.key;
}

void Keyboard::set_layer(uint8_t layer) {
    this->layer_index = layer;
    append_uniq_to_uint8_array(this->layer_history, LAYER_HISTORY_CAPACITY, layer);
#ifdef LED_PIN
    set_led_rgb(LED_LAYER_COLORS[layer]);
#endif
}

// We could have pressed multiple layer keys, so releasing one means we switch to the other one
void Keyboard::remove_layer(uint8_t layer) {
    remove_uniq_from_uint8_array(this->layer_history, LAYER_HISTORY_CAPACITY, layer);
    uint8_t prev_layer = last_nonzero_elem_of_uint8_array(
        this->layer_history, LAYER_HISTORY_CAPACITY
    );

    if (prev_layer > 0) {
        this->set_layer(prev_layer);
    } else {
        this->set_layer(this->toggled_layer_index); // toggled layer or 0
    }
}

void Keyboard::toggle_layer(uint8_t layer) {
    if (this->toggled_layer_index == layer) {
        this->toggled_layer_index = 0;
        this->remove_layer(layer);
    } else {
        this->toggled_layer_index = layer;
        this->set_layer(layer);
    }
}

// Handle key press & release {{{
inline void Keyboard::handle_key_press(KeyInfo key_info, millisec now) {
    this->held_keys_count++;
    this->key_press_counter++;

    PressedKey *pk = this->append_to_pressed_keys(key_info, now);
    if (pk == NULL) return;

    if (this->held_keys_count > 1) { // Not the first pressed key
        this->run_press_hooks(pk->key_index, now);
    }

    this->press(pk, now);
    this->send_hid_report();
}

void Keyboard::handle_key_release(KeyInfo key_info, millisec now) {
    this->held_keys_count--;

    PressedKey *pk = this->find_in_pressed_keys(key_info);
    if (pk == NULL) return;

    if (this->held_keys_count > 0) {
        this->run_release_hooks(pk->key_index, now);
    }

    this->release(pk, now);

    if (this->held_keys_count == 0) {
        this->clear();
    } else {
        this->remove_from_pressed_keys(pk);
    }

    this->send_hid_report();
}

void Keyboard::press(PressedKey *pk, millisec now) {
    KeyInfo key_info = pk->key_info;

    if (key_info.type == KEY_NORMAL) {
        this->press_normal_key(key_info);
        pk->state = STATE_PRIMARY_KEY;
    } else if (key_info.type == KEY_LAYER_PRESS) {
        this->press_layer_key(key_info.key);
        pk->state = STATE_PRIMARY_KEY;
    } else if (key_info.type == KEY_LAYER_TOGGLE) {
        this->press_toggle_layer_key(key_info.key);
        pk->state = STATE_PRIMARY_KEY;
    } else if (key_info.type == KEY_SYSTEM)  {
        this->press_system_key(key_info);
        pk->state = STATE_PRIMARY_KEY;
    } else if (key_info.is_multimedia_key()) {
        this->press_multimedia_key(key_info);
        pk->state = STATE_PRIMARY_KEY;
    } else if (key_info.is_key_with_mod()) {
        this->press_key_with_mod(key_info);
        pk->state = STATE_PRIMARY_KEY;
    } else if (key_info.type == KEY_TAPDANCE) {
        // this->press_tapdance_key(key_info, now); // TODO
    } else if (key_info.type == KEY_LAYER_TOGGLE_OR_HOLD) {
        this->press_layer_toggle_or_hold();
    } else if (key_info.is_any_dual_mod_key()) {
        this->press_dual_key(pk);
    } else if (key_info.is_any_dual_layer_key()) {
        this->press_dual_layer_key(pk);
    } else if (key_info.type == KEY_ONE_SHOT_MODIFIER) {
        this->press_one_shot_modifier_key(pk);
    }
}

void Keyboard::release(PressedKey *pk, millisec now) {
    KeyInfo key_info = pk->key_info;

    if (pk->state == STATE_RELEASED) {
        // Already released in run_release_hooks
        return;
    }

    if (key_info.is_any_dual_mod_key()) {
        this->release_dual_key(pk);
    } else if (key_info.is_any_dual_layer_key()) {
        this->release_dual_layer_key(pk);
    } else if (key_info.type == KEY_ONE_SHOT_MODIFIER) {
        this->release_one_shot_modifier_key(pk);
    } else {
        pk->state = STATE_RELEASED;

        if (key_info.type == KEY_NORMAL) {
            this->release_normal_key(key_info);
        } else if (key_info.type == KEY_LAYER_PRESS) {
            this->release_layer_key(key_info.key);
        } else if (key_info.type == KEY_LAYER_TOGGLE) {
            // do nothing; toggle_layer key has only effect on press
        } else if (key_info.type == KEY_SYSTEM)  {
            this->release_system_key(key_info);
        } else if (key_info.is_multimedia_key()) {
            this->release_multimedia_key(key_info);
        } else if (key_info.is_key_with_mod()) {
            this->release_key_with_mod(key_info);
        } else if (key_info.type == KEY_TAPDANCE) {
            // this->release_tapdance_key(key_info, now); // TODO
        } else if (key_info.type == KEY_LAYER_TOGGLE_OR_HOLD) {
            this->release_layer_toggle_or_hold(pk);
        }
    }
}
// }}}

// this->pressed_keys helpers {{{
inline PressedKey* Keyboard::append_to_pressed_keys(
    KeyInfo key_info, millisec now
) {
    for (uint8_t key_index = 0; key_index < PRESSED_KEY_BUFFER; key_index++) {
        if (this->pressed_keys.keys[key_index].timestamp > 0) {
            continue;
        }
        this->pressed_keys.count++;

        PressedKey *pk = &(this->pressed_keys.keys[key_index]);
        pk->key_info = key_info;
        pk->timestamp = now;
        pk->key_press_counter = this->key_press_counter;
        pk->state = STATE_NOT_PROCESSED;
        pk->key_index = key_index;

        return pk;
    }

    return NULL;
}

inline void Keyboard::remove_from_pressed_keys(PressedKey *pk) {
    this->pressed_keys.count--;
    uint8_t last_index = PRESSED_KEY_BUFFER - 1;

    for (uint8_t i = pk->key_index; i < last_index; i++) {
        this->pressed_keys.keys[i] = this->pressed_keys.keys[i+1];
        this->pressed_keys.keys[i].key_index = i;
    }

    memset(&(this->pressed_keys.keys[last_index]), 0, sizeof(PressedKey));
}

inline PressedKey* Keyboard::find_in_pressed_keys(KeyInfo key_info_arg) {
    for (uint8_t i = 0; i < PRESSED_KEY_BUFFER; i++) {
        if (!this->pressed_keys.keys[i].timestamp) {
            break;
        }

        PressedKey *pk = &(this->pressed_keys.keys[i]);

        if (
            pk->key_info.coords.row == key_info_arg.coords.row
            && pk->key_info.coords.col == key_info_arg.coords.col
        ) {
            return pk;
        }
    }

    return NULL;
}
// }}}

// Press & Release hooks {{{
inline void Keyboard::run_press_hooks(
    uint8_t event_key_index, millisec now
) {
    // This is not called, when there are pending timed keys, but we are still
    // checking those, as `press_hook` is called from `run_release_hooks` as well

    for (uint8_t key_index = 0; key_index < event_key_index; key_index++) {
        this->run_press_hook(key_index, now);
    }
}
inline void Keyboard::run_press_hook(uint8_t key_index, millisec now) {
    PressedKey *pk = &(this->pressed_keys.keys[key_index]);
    KeyInfo key_info = pk->key_info;


    if (pk->state != STATE_PENDING) {
        return;
    }

    if (key_info.is_any_dual_mod_key()) {
        this->press_dual_key(pk);
        this->send_hid_report();
    } else if (
        key_info.is_any_dual_layer_key()
        || pk->key_info.type == KEY_LAYER_TOGGLE_OR_HOLD
    ) {
        this->press_dual_layer_key(pk);

        // Reload the rest of the keys as of this layer were applied
        // when they were pressed
        for (uint8_t i = key_index + 1; i < PRESSED_KEY_BUFFER; i++) {
            PressedKey *pk = &(this->pressed_keys.keys[i]);

            if (!pk->timestamp || pk->state != STATE_NOT_PROCESSED) {
                break;
            }

            this->reload_key_by_coords(&(pk->key_info));
        }
    } else if (key_info.type == KEY_ONE_SHOT_MODIFIER) {
        this->press_one_shot_modifier_key(pk);
    }
}

inline void Keyboard::run_release_hooks(
    uint8_t event_key_index, millisec now
) {
    for (uint8_t key_index = 0; key_index < PRESSED_KEY_BUFFER; key_index++) {
        bool result = this->run_release_hook(key_index, event_key_index, now);
        if (!result) {
            break;
        }
    }
}
inline bool Keyboard::run_release_hook(
    uint8_t key_index, uint8_t event_key_index, millisec now
) {
    PressedKey *pk = &(this->pressed_keys.keys[key_index]);
    KeyInfo key_info = pk->key_info;

    if (!pk->timestamp) { // Key not pressed
        return 0;
    }

    bool cond = pk->state == STATE_NOT_PROCESSED || pk->state == STATE_PENDING;

    if (key_index < event_key_index) {
        if (cond) {
            if (key_index > 0) {
                this->run_press_hook(key_index - 1, pk->timestamp);
            }

            this->press(pk, pk->timestamp);
            this->send_hid_report();
        }
    } else if (key_index == event_key_index) {
        if (cond) {
            if (key_index > 0) {
                this->run_press_hook(key_index - 1, pk->timestamp);
            }
            if (pk->state == STATE_NOT_PROCESSED) {
                this->press(pk, pk->timestamp);
                this->send_hid_report();
            }
            this->release(pk, pk->timestamp);
            this->send_hid_report();
        }
    } else {
        // This is prepared for timed dual keys, which are not implemented yet
        if (0 && cond) {
            if (key_info.is_any_timed_dual_key()) {
                return 0;
            }

            if (key_index < this->pressed_keys.count) {
                this->run_press_hook(key_index - 1, pk->timestamp);
            }
            this->press(pk, pk->timestamp);
            this->send_hid_report();
        } else {
            return 0;
        }
    }

    return 1;
}
// }}}

// Normal key {{{
inline void Keyboard::press_normal_key(KeyInfo key_info) {
    uint8_t key = key_info.key;

    if (KC_LCTRL <= key && key <= KC_RGUI) { // Modifier
        // For 'Left Shift' 0xE1 bitmask is 0x0000_0010
        uint8_t modifier_bit = key & 0x0F;
        uint8_t bitmask = 1 << modifier_bit;
        this->base_hid_report.modifiers |= bitmask;
    } else { // Normal key
        if (this->one_shot_modifiers) {
            this->base_hid_report.modifiers |= this->one_shot_modifiers;
            this->one_shot_modifiers = 0x00;
        }
        append_uniq_to_uint8_array(
            this->base_hid_report.keys, BASE_HID_REPORT_KEYS, key
        );
    }
}

inline void Keyboard::release_normal_key(KeyInfo key_info) {
    uint8_t key = key_info.key;

    if (KC_LCTRL <= key && key <= KC_RGUI) {
        // For 'Left Shift' 0xE1 bitmask is 0x1111_1101
        uint8_t modifier_bit = key & 0x0F;
        uint8_t bitmask = (1 << modifier_bit) ^ 0xFF;
        this->base_hid_report.modifiers &= bitmask;
    } else { // key
        remove_uniq_from_uint8_array(
            this->base_hid_report.keys, BASE_HID_REPORT_KEYS, key
        );
    }
}
// }}}
// One-shot modifier key {{{
inline void Keyboard::press_one_shot_modifier_key(PressedKey *pk) {
    if (pk->state == STATE_NOT_PROCESSED) {
        if (pk->key_index > 0) {
            // Not the first key
            uint8_t modifier = pk->key_info.key;
            this->press_normal_key(
                KeyInfo(KEY_NORMAL, modifier, pk->key_info.coords)
            );
            pk->state = STATE_PRIMARY_KEY;
        } else {
            pk->state = STATE_PENDING;
        }
    } else if (pk->state == STATE_PENDING) {
        // If a different key is pressed, act as modifier
        uint8_t modifier = pk->key_info.key;
        this->press_normal_key(
            KeyInfo(KEY_NORMAL, modifier, pk->key_info.coords)
        );
        pk->state = STATE_PRIMARY_KEY;
    }
}
inline void Keyboard::release_one_shot_modifier_key(PressedKey *pk) {
    if (pk->state == STATE_PRIMARY_KEY) {
        uint8_t modifier = pk->key_info.key;
        this->release_normal_key(
            KeyInfo(KEY_NORMAL, modifier, pk->key_info.coords)
        );
    } else if (pk->state == STATE_PENDING) {
        // No other key was pressed after this one, act as one-shot modifier
        uint8_t modifier = pk->key_info.key & 0x0F;
        uint8_t bitmask = 1 << modifier;

        this->one_shot_modifiers ^= bitmask;
    }
}
// }}}
// Layer key {{{
inline void Keyboard::press_layer_key(uint8_t layer) {
    this->set_layer(layer);
}

inline void Keyboard::release_layer_key(uint8_t layer) {
    this->remove_layer(layer);
}
// }}}
// Layer toggle key {{{
inline void Keyboard::press_toggle_layer_key(uint8_t layer) {
    this->toggle_layer(layer);
}
// }}}
// System key {{{
inline void Keyboard::press_system_key(KeyInfo key_info) {
    this->system_hid_report.key = key_info.key;
}

inline void Keyboard::release_system_key(KeyInfo key_info) {
    if (this->system_hid_report.key == key_info.key) {
        this->system_hid_report.key = 0x00;
    }
}
// }}}
// Multimedia key {{{
inline void Keyboard::press_multimedia_key(KeyInfo key_info) {
    this->multimedia_hid_report.key = key_info.key;
    if (key_info.type == KEY_MULTIMEDIA_2) {
        this->multimedia_hid_report.prefix = 0x02;
    } else if (key_info.type == KEY_MULTIMEDIA_1) {
        this->multimedia_hid_report.prefix = 0x01;
    } else {
        this->multimedia_hid_report.prefix = 0x00;
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
        this->multimedia_hid_report.prefix == prefix
        && this->multimedia_hid_report.key == key
    ) {
        memset(&(this->multimedia_hid_report), 0, sizeof(MultimediaHIDReport));
    }
}
// }}}
// Key with modifier {{{
inline void Keyboard::press_key_with_mod(KeyInfo key_info) {
    uint8_t modifier = key_info.get_key_with_mod_modifier();
    this->press_normal_key(KeyInfo(KEY_NORMAL, modifier, key_info.coords));

    this->press_normal_key(KeyInfo(KEY_NORMAL, key_info.key, key_info.coords));
}

inline void Keyboard::release_key_with_mod(KeyInfo key_info) {
    uint8_t modifier = key_info.get_key_with_mod_modifier();
    this->release_normal_key(
        KeyInfo(KEY_NORMAL, modifier, key_info.coords)
    );
    this->release_normal_key(
        KeyInfo(KEY_NORMAL, key_info.key, key_info.coords)
    );
}
// }}}
// Dual key {{{
inline void Keyboard::press_dual_key(PressedKey *pk) {
    if (pk->state == STATE_NOT_PROCESSED) {
        if (pk->key_index > 0 && pk->key_info.is_any_solo_dual_key()) {
            // Not the first key
            this->press_normal_key(pk->key_info);
            pk->state = STATE_PRIMARY_KEY;
        } else {
            pk->state = STATE_PENDING;
        }
    } else if (pk->state == STATE_PENDING) {
        // The primary key is activated only in release_dual_key
        uint8_t modifier = pk->key_info.get_dual_key_modifier();
        this->press_normal_key(
            KeyInfo(KEY_NORMAL, modifier, pk->key_info.coords)
        );
        pk->state = STATE_SECONDARY_KEY;
    }
}

inline void Keyboard::release_dual_key(PressedKey *pk) {
    KeyInfo key_info = pk->key_info;

    if (pk->state == STATE_PRIMARY_KEY) {
        this->release_normal_key(
            KeyInfo(KEY_NORMAL, key_info.key, key_info.coords)
        );
        pk->state = STATE_RELEASED;
    } else if (pk->state == STATE_SECONDARY_KEY) {
        uint8_t modifier = key_info.get_dual_key_modifier();
        this->release_normal_key(
            KeyInfo(KEY_NORMAL, modifier, key_info.coords)
        );
        pk->state = STATE_RELEASED;
    } else if (pk->state == STATE_PENDING || pk->state == STATE_NOT_PROCESSED) {
        // This state is called from run_release_hook. Then release is called
        // and we call release_normal_key
        this->press_normal_key(
            KeyInfo(KEY_NORMAL, key_info.key, key_info.coords)
        );
        this->send_hid_report();
        pk->state = STATE_PRIMARY_KEY;
    }
}
// }}}

// Dual layer key {{{
inline void Keyboard::press_dual_layer_key(PressedKey *pk) {
    if (pk->state == STATE_NOT_PROCESSED) {
        if (pk->key_index > 0 && pk->key_info.is_any_solo_dual_key()) {
            // Not the first key
            this->press_normal_key(pk->key_info);
            pk->state = STATE_PRIMARY_KEY;
        } else {
            pk->state = STATE_PENDING;
        }
    } else if (pk->state == STATE_PENDING) {
        uint8_t layer = pk->key_info.get_dual_layer_key_layer();
        this->press_layer_key(layer);
        pk->state = STATE_SECONDARY_KEY;
    }
}

inline void Keyboard::release_dual_layer_key(PressedKey *pk) {
    KeyInfo key_info = pk->key_info;

    if (pk->state == STATE_PRIMARY_KEY) {
        this->release_normal_key(
            KeyInfo(KEY_NORMAL, key_info.key, key_info.coords)
        );
        pk->state = STATE_RELEASED;
    } else if (pk->state == STATE_SECONDARY_KEY) {
        uint8_t layer = key_info.get_dual_layer_key_layer();
        this->release_layer_key(layer);
        pk->state = STATE_RELEASED;
    } else if (pk->state == STATE_PENDING) {
        this->press_normal_key(
            KeyInfo(KEY_NORMAL, key_info.key, key_info.coords)
        );
        this->send_hid_report();
        pk->state = STATE_PRIMARY_KEY;
    }
}
// }}}

// Layer toggle or hold {{{
inline void Keyboard::press_layer_toggle_or_hold() {}
inline void Keyboard::release_layer_toggle_or_hold(PressedKey *pk) {
    if (
        // We have not decided what to do with this key, but no other keys
        // were pressed after this - let's toggle a layer
        pk->state == STATE_PENDING
        && pk->key_press_counter == this->key_press_counter
    ) {
        this->toggle_layer(pk->key_info.key);
    } else {
        this->remove_layer(pk->key_info.key);
    }
}
// }}}

inline void Keyboard::send_hid_report() {
    // Base
    size_t base_size = sizeof(BaseHIDReport);
    void *base = &(this->base_hid_report);
    void *last_base = &(this->last_base_hid_report);

    if (memcmp(&(this->base_hid_report), last_base, base_size)) {
        this->print_base_report_to_serial();
        DalsikHid::send_report(BASE_KEYBOARD_REPORT_ID, base, base_size);
        this->last_base_hid_report = this->base_hid_report;
    }

    // System
    size_t system_size = sizeof(SystemHIDReport);
    void *system = &(this->system_hid_report);
    void *last_system = &(this->last_system_hid_report);

    if (memcmp(system, last_system, system_size)) {
        this->print_system_report_to_serial();
        DalsikHid::send_report(SYSTEM_KEYBOARD_REPORT_ID, system, system_size);
        this->last_system_hid_report = this->system_hid_report;
    }

    // Multimedia
    size_t multimedia_size = sizeof(MultimediaHIDReport);
    void *multimedia = &(this->multimedia_hid_report);
    void *last_multimedia = &(this->last_multimedia_hid_report);

    if (memcmp(multimedia, last_multimedia, multimedia_size)) {
        this->print_multimedia_report_to_serial();
        DalsikHid::send_report(MULTIMEDIA_KEYBOARD_REPORT_ID, multimedia, multimedia_size);
        this->last_multimedia_hid_report = this->multimedia_hid_report;
    }
}

void Keyboard::print_base_report_to_serial() {
#if DEBUG_KEYREPORT_BASE
    Serial.print(F("Keyboard[BASE]:"));
    Serial.print(this->base_hid_report.modifiers, HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.reserved, HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.keys[0], HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.keys[1], HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.keys[2], HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.keys[3], HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.keys[4], HEX);
    Serial.print(F("|"));
    Serial.print(this->base_hid_report.keys[5], HEX);
    Serial.print(F("\n"));
#endif
}

void Keyboard::print_system_report_to_serial() {
#if DEBUG_KEYREPORT_SYSTEM
    Serial.print(F("Keyboard[SYSTEM]:"));
    Serial.print(this->system_hid_report.key, HEX);
    Serial.print(F("\n"));
#endif
}

void Keyboard::print_multimedia_report_to_serial() {
#if DEBUG_KEYREPORT_MULTIMEDIA
    Serial.print(F("Keyboard[MULTIMEDIA]:"));
    Serial.print(this->multimedia_hid_report.prefix, HEX);
    Serial.print(F("|"));
    Serial.print(this->multimedia_hid_report.key, HEX);
    Serial.print(F("\n"));
#endif
}

void Keyboard::print_internal_state() {
    Serial.print("=========================PRESSED_KEYS\n");
    Serial.print("  -count:");
    Serial.print(this->pressed_keys.count);
    Serial.print(" this->key_press_counter:");
    Serial.print(this->key_press_counter);
    Serial.print("\n");
    for (uint8_t i = 0; i < this->pressed_keys.count; i++) {
        KeyInfo key_info = this->pressed_keys.keys[i].key_info;
        uint8_t state = this->pressed_keys.keys[i].state;

        Serial.print("  -[");
        Serial.print(i);
        Serial.print("/");
        Serial.print(this->pressed_keys.keys[i].key_index);
        Serial.print("] - KeyInfo[T:");
        Serial.print(key_info.type);
        Serial.print(",K:");
        Serial.print(key_info.key);
        Serial.print(",R:");
        Serial.print(key_info.coords.row);
        Serial.print(",C:");
        Serial.print(key_info.coords.col);
        Serial.print("]\n       - key_press_counter:");
        Serial.print(this->pressed_keys.keys[i].key_press_counter);
        Serial.print("\n        - timestamp:");
        Serial.print(this->pressed_keys.keys[i].timestamp);
        Serial.print("\n        - state:");
        if (state == STATE_NOT_PROCESSED) {
            Serial.print("STATE_NOT_PROCESSED");
        } else if (state == STATE_PENDING) {
            Serial.print("STATE_PENDING");
        } else if (state == STATE_PRIMARY_KEY) {
            Serial.print("STATE_PRIMARY_KEY");
        } else if (state == STATE_SECONDARY_KEY) {
            Serial.print("STATE_SECONDARY_KEY");
        } else if (state == STATE_RELEASED) {
            Serial.print("STATE_RELEASED");
        }
        Serial.print("\n");
    }
}
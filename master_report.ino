#include "keyboard.h"
#include "HID.h"
#include "keyboard_hid_desc.h"
#include "keymap.h"
#include "array_utils.h"

MasterReport::MasterReport(KeyMap* keymap) {
    this->keymap = keymap;
    this->clear();
}

void MasterReport::clear() {
    this->hid_report.modifiers = 0;
    this->hid_report.reserved = 0;
    memset(this->hid_report.keys, 0, HID_KEYS_COUNT);
    memset(&(this->dual_key_state), 0, sizeof(DualKeyState));
    memset(&(this->hold_or_toggle_state), 0, sizeof(LayerHoldOrToggleState));

    this->num_keys_pressed = 0;

    this->keymap->clear();
}

void MasterReport::handle_changed_key(ChangedKeyCoords coords) {
    if (coords.type == EVENT_NONE) {
        return;
    }

    KeyInfo key_info = this->keymap->get_key(coords.row, coords.col);
    if (key_info.type == KEY_TRANSPARENT) { // Get the key from lower layers
        key_info = this->keymap->get_non_transparent_key(coords.row, coords.col);
    }

    if (coords.type == EVENT_KEY_PRESS) {
        this->num_keys_pressed++;
        this->press(key_info);
        this->send_report();
    }
    if (coords.type == EVENT_KEY_RELEASE) {
        this->num_keys_pressed--;
        this->release(key_info);
        this->send_report();
    }
}

void MasterReport::press(KeyInfo key_info) {
#if DEBUG
    Serial.print("Pressed key:");
    Serial.print(KeyMap::key_type_to_string(key_info));
    Serial.print("|");
    Serial.print(key_info.key, HEX);
    Serial.print("\n");
#endif

    this->press_hook_for_dual_keys();
    this->press_hook_for_layer_hold_or_toggle();

    if (key_info.type == KEY_NORMAL) {
        this->press_normal_key(key_info);
    } else if (key_info.type == KEY_LAYER_PRESS) {
        this->press_layer_key(key_info);
    } else if (key_info.type == KEY_LAYER_TOGGLE) {
        this->press_toggle_layer_key(key_info);
    } else if (key_info.type == KEY_LAYER_HOLD_OR_TOGGLE) {
        this->press_layer_hold_or_toggle(key_info);
    } else if (KeyMap::is_dual_key(key_info)) {
        this->press_dual_key(key_info);
    } else if (KeyMap::is_key_with_mod(key_info)) {
        this->press_key_with_mod(key_info);
    }
}

void MasterReport::release(KeyInfo key_info) {
#if DEBUG
    Serial.print("Released key:");
    Serial.print(KeyMap::key_type_to_string(key_info));
    Serial.print("|");
    Serial.print(key_info.key, HEX);
    Serial.print("\n");
#endif

    if (key_info.type == KEY_NORMAL) {
        this->release_normal_key(key_info);
    } else if (key_info.type == KEY_LAYER_PRESS) {
        this->release_layer_key(key_info);
    } else if (key_info.type == KEY_LAYER_TOGGLE) {
        this->release_toggle_layer_key(key_info);
    } else if (key_info.type == KEY_LAYER_HOLD_OR_TOGGLE) {
        this->release_layer_hold_or_toggle(key_info);
    } else if (KeyMap::is_dual_key(key_info)) {
        this->release_dual_key(key_info);
    } else if (KeyMap::is_key_with_mod(key_info)) {
        this->release_key_with_mod(key_info);
    }

    if (this->num_keys_pressed == 0) {
        this->clear();
    }
}

inline void MasterReport::press_normal_key(KeyInfo key_info) {
    uint8_t key = key_info.key;
    if (key >= 0xE0 && key <= 0xE7) { // modifier
        // For 'Left Shift' 0xE1 bitmask is 0x0000_0010
        uint8_t modifier_bit = key & 0x0F;
        uint8_t bitmask = 1 << modifier_bit;
        this->hid_report.modifiers |= bitmask;
    } else { // key
        append_uniq_to_uint8_array(this->hid_report.keys, HID_KEYS_COUNT, key);
    }
}

inline void MasterReport::release_normal_key(KeyInfo key_info) {
    uint8_t key = key_info.key;
    if (key >= 0xE0 && key <= 0xE7) { // modifier
        // For 'Left Shift' 0xE1 bitmask is 0x1111_1101
        uint8_t modifier_bit = key & 0x0F;
        uint8_t bitmask = (1 << modifier_bit) ^ 0xFF;
        this->hid_report.modifiers &= bitmask;
    } else { // key
        remove_uniq_from_uint8_array(this->hid_report.keys, HID_KEYS_COUNT, key);
    }
}

inline void MasterReport::press_layer_key(KeyInfo key_info) {
    this->keymap->set_layer(key_info.key);
}

inline void MasterReport::release_layer_key(KeyInfo key_info) {
    this->keymap->remove_layer(key_info.key);
}

inline void MasterReport::press_toggle_layer_key(KeyInfo key_info) {
    this->keymap->toggle_layer(key_info.key);
}

inline void MasterReport::release_toggle_layer_key(KeyInfo key_info) {
    // do nothing; toggle_layer key has only effect on press
}

inline void MasterReport::press_dual_key(KeyInfo key_info) {
    if (this->dual_key_state.mode == DUAL_MODE_NOT_PRESSED) {
        this->dual_key_state.key_info = key_info;
        if (this->num_keys_pressed > 1) {
            this->dual_key_state.mode = DUAL_MODE_HOLD_MODIFIER;
            uint8_t modifier = KeyMap::get_dual_key_modifier(key_info);
            this->press_normal_key(KeyInfo { KEY_NORMAL, modifier });
        } else {
            this->dual_key_state.mode = DUAL_MODE_PENDING;
        }
    } else {
        uint8_t modifier = KeyMap::get_dual_key_modifier(key_info);
        this->press_normal_key(KeyInfo { KEY_NORMAL, modifier });
    }
}

inline void MasterReport::release_dual_key(KeyInfo key_info) {
    if (KeyMap::key_info_compare(key_info, this->dual_key_state.key_info) == 0) {
        if (this->dual_key_state.mode == DUAL_MODE_HOLD_MODIFIER) {
            uint8_t modifier = KeyMap::get_dual_key_modifier(key_info);
            this->release_normal_key(KeyInfo { KEY_NORMAL, modifier });
        } else {
            KeyInfo key_info_tap = { KEY_NORMAL, key_info.key };

            this->press_normal_key(key_info_tap);
            this->send_report();
            this->release_normal_key(key_info_tap);
        }

        memset(&(this->dual_key_state), 0, sizeof(DualKeyState));
    } else {
        // There were more dual_keys pressed, this one is not the first
        uint8_t modifier = KeyMap::get_dual_key_modifier(key_info);
        this->release_normal_key(KeyInfo { KEY_NORMAL, modifier });
    }
}

inline void MasterReport::press_hook_for_dual_keys() {
    if (this->dual_key_state.mode == DUAL_MODE_PENDING) {
        this->dual_key_state.mode = DUAL_MODE_HOLD_MODIFIER;
        uint8_t modifier = KeyMap::get_dual_key_modifier(this->dual_key_state.key_info);
        this->press_normal_key(KeyInfo { KEY_NORMAL, modifier });
    }
}

inline void MasterReport::press_layer_hold_or_toggle(KeyInfo key_info) {
    if (this->hold_or_toggle_state.mode == HOLD_OR_TOGGLE_NOT_PRESSED) {
        this->hold_or_toggle_state.key_info = key_info;
        if (this->num_keys_pressed > 1) {
            this->hold_or_toggle_state.mode = HOLD_OR_TOGGLE_HOLD_LAYER;
            this->keymap->set_layer(key_info.key);
        } else {
            this->hold_or_toggle_state.mode = HOLD_OR_TOGGLE_PENDING;
        }
    } else {
        this->keymap->set_layer(key_info.key);
    }
}

inline void MasterReport::release_layer_hold_or_toggle(KeyInfo key_info) {
    if (KeyMap::key_info_compare(key_info, this->hold_or_toggle_state.key_info) == 0) {
        if (this->hold_or_toggle_state.mode == HOLD_OR_TOGGLE_HOLD_LAYER) {
            this->keymap->remove_layer(key_info.key);
        } else {
            this->keymap->toggle_layer(key_info.key);
        }

        memset(&(this->hold_or_toggle_state), 0, sizeof(LayerHoldOrToggleState));
    } else {
        this->keymap->remove_layer(key_info.key);
    }
}

inline void MasterReport::press_hook_for_layer_hold_or_toggle() {
    if (this->hold_or_toggle_state.mode == HOLD_OR_TOGGLE_PENDING) {
        this->hold_or_toggle_state.mode = HOLD_OR_TOGGLE_HOLD_LAYER;
        this->keymap->set_layer(this->hold_or_toggle_state.key_info.key);
    }
}

inline void MasterReport::press_key_with_mod(KeyInfo key_info) {
    uint8_t modifier = KeyMap::get_key_with_mod_modifier(key_info);
    this->press(KeyInfo { KEY_NORMAL, modifier });
    this->press(KeyInfo { KEY_NORMAL, key_info.key });
}

inline void MasterReport::release_key_with_mod(KeyInfo key_info) {
    uint8_t modifier = KeyMap::get_key_with_mod_modifier(key_info);
    this->release(KeyInfo { KEY_NORMAL, modifier });
    this->release(KeyInfo { KEY_NORMAL, key_info.key });
}

void MasterReport::print_to_serial() {
    Serial.print("MasterReport:");
    Serial.print(this->hid_report.modifiers, HEX);
    Serial.print("|");
    Serial.print(this->hid_report.reserved, HEX);
    Serial.print("|");
    Serial.print(this->hid_report.keys[0], HEX);
    Serial.print("|");
    Serial.print(this->hid_report.keys[1], HEX);
    Serial.print("|");
    Serial.print(this->hid_report.keys[2], HEX);
    Serial.print("|");
    Serial.print(this->hid_report.keys[3], HEX);
    Serial.print("|");
    Serial.print(this->hid_report.keys[4], HEX);
    Serial.print("|");
    Serial.print(this->hid_report.keys[5], HEX);
    Serial.print("\n");
}

void MasterReport::send_report() {
#if DEBUG
    this->print_to_serial();
#endif
    HID().SendReport(KEYBOARD_REPORT_ID, &(this->hid_report), sizeof(HIDKeyboardReport));
}

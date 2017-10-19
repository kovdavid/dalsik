#include "keyboard.h"
#include "HID.h"
#include "keyboard_hid_desc.h"
#include "keymap.h"

MasterReport::MasterReport(KeyMap* keymap) {
    this->keymap = keymap;
    this->clear();
    memset(&(this->dual_key_state), 0, sizeof(DualKeyState));
}

void MasterReport::clear() {
    this->hid_report.modifiers = 0;
    this->hid_report.reserved = 0;
    memset(this->hid_report.keys, 0, 6);

    this->num_keys_pressed = 0;

    this->keymap->set_layer(0);
}

void MasterReport::handle_changed_key(ChangedKeyCoords coords) {
    if (coords.type == EVENT_NONE) {
        return;
    }

    KeyInfo key_info = this->keymap->get_key(coords.row, coords.col);

    if (coords.type == EVENT_KEY_PRESS) {
        this->press(key_info);
    }
    if (coords.type == EVENT_KEY_RELEASE) {
        this->release(key_info);
    }
}

void MasterReport::press(KeyInfo key_info) {
#if DEBUG
    Serial.print("Pressed key:");
    Serial.print(key_type_to_string(key_info));
    Serial.print("|");
    Serial.print(key_info.key, HEX);
    Serial.print("\n");
#endif

    this->num_keys_pressed++;

    this->press_hook_for_dual_keys();

    if (key_info.type == KEY_NORMAL) {
        this->press_normal_key(key_info);
    } else if (key_info.type == KEY_LAYER_PRESS) {
        this->press_layer_key(key_info);
    } else if (is_dual_key(key_info)) {
        this->press_dual_key(key_info);
    } else {
        return;
    }

    this->send_report();
}

inline void MasterReport::press_hook_for_dual_keys() {
    if (this->dual_key_state.mode == DUAL_MODE_NOT_PRESSED) {
        return;
    }

    this->dual_key_state.mode = DUAL_MODE_HOLD_MODIFIER;
    KeyInfo key_info = { KEY_NORMAL, get_dual_key_modifier(this->dual_key_state.key_info) };
    this->press_normal_key(key_info);
}

inline void MasterReport::press_normal_key(KeyInfo key_info) {
    uint8_t key = key_info.key;
    if (key >= 0xE0 && key <= 0xE7) { // modifier
        // For 'Left Shift' 0xE1 bitmask is 0x0000_0010
        uint8_t modifier_bit = key & 0x0F;
        uint8_t bitmask = 1 << modifier_bit;
        this->hid_report.modifiers |= bitmask;
    } else { // key
        for (uint8_t i = 0; i < 6; i++) {
            if (this->hid_report.keys[i] == key) {
                return; // already pressed
            }
            if (this->hid_report.keys[i] == 0x00) {
                this->hid_report.keys[i] = key;
                return;
            }
        }
    }
}

inline void MasterReport::press_layer_key(KeyInfo key_info) {
    this->keymap->set_layer(key_info.key);
}

inline void MasterReport::press_dual_key(KeyInfo key_info) {
    if (this->dual_key_state.mode != DUAL_MODE_NOT_PRESSED) {
        return;
    }

    this->dual_key_state.key_info = key_info;
    if (this->num_keys_pressed > 1) {
        this->dual_key_state.mode = DUAL_MODE_HOLD_MODIFIER;

        KeyInfo key_info = { KEY_NORMAL, get_dual_key_modifier(key_info) };
        this->press_normal_key(key_info);
    } else {
        this->dual_key_state.mode = DUAL_MODE_PENDING;
    }
}

void MasterReport::release(KeyInfo key_info) {
#if DEBUG
    Serial.print("Released key:");
    Serial.print(key_type_to_string(key_info));
    Serial.print("|");
    Serial.print(key_info.key, HEX);
    Serial.print("\n");
#endif

    this->num_keys_pressed--;

    if (key_info.type == KEY_NORMAL) {
        this->release_normal_key(key_info);
    } else if (key_info.type == KEY_LAYER_PRESS) {
        this->release_layer_key(key_info);
    } else if (is_dual_key(key_info)) {
        this->release_dual_key(key_info);
    } else {
        return;
    }

    if (this->num_keys_pressed == 0) {
        this->clear();
    }

    this->send_report();
}

inline void MasterReport::release_normal_key(KeyInfo key_info) {
    uint8_t key = key_info.key;
    if (key >= 0xE0 && key <= 0xE7) { // modifier
        // For 'Left Shift' 0xE1 bitmask is 0x1111_1101
        uint8_t modifier_bit = key & 0x0F;
        uint8_t bitmask = (1 << modifier_bit) ^ 0xFF;
        this->hid_report.modifiers &= bitmask;
    } else { // key
        for (uint8_t i = 0; i < 6; i++) {
            if (this->hid_report.keys[i] == key) {
                // shift the rest of the keys to the left
                for (uint8_t j = i; j < 5; j++) {
                    this->hid_report.keys[j] = this->hid_report.keys[j+1];
                }
                this->hid_report.keys[5] = 0x00;
                return;
            }
        }
    }
}

inline void MasterReport::release_layer_key(KeyInfo key_info) {
    this->keymap->set_layer(0);
}

inline void MasterReport::release_dual_key(KeyInfo key_info) {
    if (this->dual_key_state.mode == DUAL_MODE_HOLD_MODIFIER) {
        KeyInfo key_info = { KEY_NORMAL, get_dual_key_modifier(key_info) };
        this->release_normal_key(key_info);
    } else {
        this->dual_key_state.mode = DUAL_MODE_TAP_KEY;
        KeyInfo key_info = { KEY_NORMAL, key_info.key };
        this->press_normal_key(key_info);
        this->send_report();
        this->release_normal_key(key_info);
    }

    memset(&(this->dual_key_state), 0, sizeof(DualKeyState));
}

void MasterReport::check_special_keys() {}

void MasterReport::print_to_serial() {
    Serial.print("MasterReport:");
    Serial.print(this->hid_report.modifiers, HEX);
    Serial.print(this->hid_report.reserved, HEX);
    Serial.print(this->hid_report.keys[0], HEX);
    Serial.print(this->hid_report.keys[1], HEX);
    Serial.print(this->hid_report.keys[2], HEX);
    Serial.print(this->hid_report.keys[3], HEX);
    Serial.print(this->hid_report.keys[4], HEX);
    Serial.print(this->hid_report.keys[5], HEX);
    Serial.print("\n");
}

void MasterReport::send_report() {
#if DEBUG
    this->print_to_serial();
#endif
    HID().SendReport(KEYBOARD_REPORT_ID, &(this->hid_report), sizeof(HIDKeyboardReport));
}

#include "HID.h"
#include "keyboard_hid_desc.h"
#include "keymap.h"

KeyReport::KeyReport()
{
    this->clear();
}

void KeyReport::clear()
{
    this->hid_report.modifiers = 0;
    this->hid_report.reserved = 0;
    memset(this->hid_report.keys, 0, 6);

    this->keys_pressed = 0;
}

void KeyReport::press(uint8_t row, uint8_t col)
{
#if DEBUG
    Serial.print("Pressed key");
    Serial.print(keycode, HEX);
    Serial.print("\n");
#endif

    this->keys_pressed++;

    KeyInfo changed_key = this->keymap.get_key(row, col);

    switch (changed_key.type) {
        case KEY_NORMAL:
            this->press_normal_key(changed_key);
            break;
        case KEY_LAYER:
            this->press_layer_key(changed_key);
            break;
        default:
            return; // KEY_UNSET + other unknown types
    }

    this->send_report();
}

inline void KeyReport::press_normal_key(KeyInfo key)
{
    uint8_t keycode = key.normal.key;
    if (keycode >= 0xE0 && keycode <= 0xE7) { // modifier
        // For 'Left Shift' 0xE1 bitmask is 0x0000_0010
        uint8_t modifier_bit = keycode & 0x0F;
        uint8_t bitmask = 1 << modifier_bit;
        this->hid_report.modifiers |= bitmask;
    } else { // key
        for (uint8_t i = 0; i < 6; i++) {
            if (this->hid_report.keys[i] == keycode) {
                return; // already pressed
            }
            if (this->hid_report.keys[i] == 0x00) {
                this->hid_report.keys[i] = keycode;
                return;
            }
        }
    }
}

inline void KeyReport::press_layer_key(KeyInfo key)
{
    this->keymap.set_layer(key.layer.layer);
}

void KeyReport::release(uint8_t row, uint8_t col)
{
#if DEBUG
    Serial.print("Released key");
    Serial.print(keycode, HEX);
    Serial.print("\n");
#endif

    this->keys_pressed--;

    KeyInfo changed_key = this->keymap.get_key(row, col);

    switch (changed_key.type) {
        case KEY_NORMAL:
            this->release_normal_key(changed_key);
            break;
        case KEY_LAYER:
            this->release_layer_key(changed_key);
            break;
    }

    if (this->keys_pressed == 0) {
        this->clear();
    }

    this->send_report();
}

inline void KeyReport::release_normal_key(KeyInfo key)
{
    uint8_t keycode = key.normal.key;
    if (keycode >= 0xE0 && keycode <= 0xE7) { // modifier
        // For 'Left Shift' 0xE1 bitmask is 0x1111_1101
        uint8_t modifier_bit = keycode & 0x0F;
        uint8_t bitmask = (1 << modifier_bit) ^ 0xFF;
        this->hid_report.modifiers &= bitmask;
    } else { // key
        for (uint8_t i = 0; i < 6; i++) {
            if (this->hid_report.keys[i] == keycode) {
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

inline void KeyReport::release_layer_key(KeyInfo key)
{
    this->keymap.set_layer(0);
}

void KeyReport::check_special_keys(unsigned long now_msec)
{
}

void KeyReport::print_to_serial()
{
    Serial.print("KeyReport:");
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

void KeyReport::send_report()
{
#if DEBUG
    this->print_to_serial();
#endif
    HID().SendReport(KEYBOARD_REPORT_ID, &(this->hid_report), sizeof(HIDKeyboardReport));
}

#pragma once

#define KEYBOARD_SIDE_ADDRESS 0

#define KEYBOARD_SIDE_LEFT  0x00
#define KEYBOARD_SIDE_RIGHT 0x01

namespace EEPROM {
    uint8_t get_keyboard_side();
    void set_keyboard_side(uint8_t side);
}

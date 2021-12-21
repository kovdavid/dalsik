#pragma once

#include "dalsik.h"

namespace EEPROM {
    void clear_all();
    void clear_keymap();
    void clear_tapdance();

    uint8_t get_keyboard_side();
    void set_keyboard_side(uint8_t side);

    EEPROM_KeyInfo get_key(uint8_t layer, KeyCoords c);
    EEPROM_KeyInfo get_tapdance_key(uint8_t index, uint8_t tap);

    void set_key(uint8_t layer, KeyInfo key_info);
    void set_tapdance_key(uint8_t index, uint8_t tap, KeyInfo key_info);
}

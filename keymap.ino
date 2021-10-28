#include "EEPROM.h"
#include "dalsik.h"
#include "keymap.h"
#include "array_utils.h"
#include "dalsik_led.h"

#define KEYBOARD_SIDE_EEPROM_ADDRESS EEPROM.length() - 1

KeyMap::KeyMap() {
    this->layer_index = 0;
    this->toggled_layer_index = 0;
    this->keyboard_side = EEPROM.read(KEYBOARD_SIDE_EEPROM_ADDRESS);
    this->clear();
}

void KeyMap::clear() {
    this->layer_index = 0;
    memset(this->layer_history, 0, sizeof(uint8_t)*LAYER_HISTORY_CAPACITY);
    if (this->toggled_layer_index > 0) {
        this->set_layer(this->toggled_layer_index);
    }
}

// The right side PCB of the Let's Split is reversed, so if it sends col 0, it is actually col 5.
// Also, the keyboard is represented in EEPROM as a whole, so the reported col must be incremented
// by ONE_SIDE_COL_PIN_COUNT (col 4 becomes col 10, as there are 6 columns per side)
// The right side sends columns 0-5 and thus we offset it to 6-11
KeyInfo KeyMap::get_master_key(uint8_t row, uint8_t col) {
    if (this->keyboard_side == KEYBOARD_SIDE_RIGHT) {
        col = 2*ONE_SIDE_COL_PIN_COUNT - col - 1;
    }
    return this->get_key(row, col);
}

KeyInfo KeyMap::get_slave_key(uint8_t row, uint8_t col) {
    // KeyMap is used only on the master side; The slave is the right side if the master is the left
    if (this->keyboard_side == KEYBOARD_SIDE_LEFT) {
        col = 2*ONE_SIDE_COL_PIN_COUNT - col - 1;
    }
    return this->get_key(row, col);
}

KeyInfo KeyMap::get_key(uint8_t row, uint8_t col) {
    uint32_t eeprom_address = this->get_eeprom_address(this->layer_index, row, col);

    uint8_t type = EEPROM.read(eeprom_address);
    uint8_t key = EEPROM.read(eeprom_address + 0x01);

    KeyInfo key_info = KeyMap::init_key_info(type, key, row, col);
    if (key_info.type == KEY_TRANSPARENT) { // Get the key from lower layers
        key_info = this->get_non_transparent_key(row, col);
    }

    return key_info;
}

void KeyMap::reload_key_info_by_row_col(KeyInfo* ki) {
    if (ki->row == ROW_UNKNOWN || ki->col == COL_UNKNOWN) {
        return; // Missing coords info
    }

    KeyInfo new_ki = this->get_key(ki->row, ki->col);
    ki->type = new_ki.type;
    ki->key = new_ki.key;
}

void KeyMap::update_keyboard_side(uint8_t side) {
    if (side == KEYBOARD_SIDE_LEFT || side == KEYBOARD_SIDE_RIGHT) {
        EEPROM.update(KEYBOARD_SIDE_EEPROM_ADDRESS, side);
        this->keyboard_side = side;
    }
}

uint8_t KeyMap::get_keyboard_side() {
    return this->keyboard_side;
}

uint8_t KeyMap::get_layer() {
    return this->layer_index;
}

void KeyMap::set_layer(uint8_t layer) {
    this->layer_index = layer;
    append_uniq_to_uint8_array(this->layer_history, LAYER_HISTORY_CAPACITY, layer);
#ifdef LED_PIN
    set_led_rgb(LED_LAYER_COLORS[layer]);
#endif
}

// We could have pressed multiple layer keys, so releasing one means we switch to the other one
void KeyMap::remove_layer(uint8_t layer) {
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

void KeyMap::toggle_layer(uint8_t layer) {
    if (this->toggled_layer_index == layer) {
        this->toggled_layer_index = 0;
        this->remove_layer(layer);
    } else {
        this->toggled_layer_index = layer;
        this->set_layer(layer);
    }
}

inline uint32_t KeyMap::get_eeprom_address(uint8_t layer, uint8_t row, uint8_t col) {
    return sizeof(EEPROM_KeyInfo)*( layer*KEY_COUNT + row*2*ONE_SIDE_COL_PIN_COUNT + col );
}

inline uint32_t KeyMap::get_tapdance_eeprom_address(uint8_t index, uint8_t tap) {
    return TAPDANCE_EEPROM_OFFSET + sizeof(EEPROM_KeyInfo)*( index*MAX_TAPDANCE_TAPS + tap-1 );
}

KeyInfo KeyMap::get_key_from_layer(uint8_t layer, uint8_t row, uint8_t col) {
    uint32_t eeprom_address = this->get_eeprom_address(layer, row, col);

    uint8_t type = EEPROM.read(eeprom_address);
    uint8_t key = EEPROM.read(eeprom_address + 0x01);

    return KeyMap::init_key_info(type, key, row, col);
}

KeyInfo KeyMap::get_non_transparent_key(uint8_t row, uint8_t col) {
    for (int8_t i = LAYER_HISTORY_CAPACITY-1; i >= 0; i--) {
        uint8_t layer = this->layer_history[i];
        if (layer == 0x00) {
            continue;
        }

        KeyInfo key_info = this->get_key_from_layer(layer, row, col);
        if (key_info.type != KEY_TRANSPARENT) {
            return key_info;
        }
    }

    KeyInfo key_info = this->get_key_from_layer(0, row, col);
    if (key_info.type == KEY_TRANSPARENT) {
        return KeyMap::init_key_info(KEY_UNSET, 0x00, row, col);
    } else {
        return key_info;
    }
}

KeyInfo KeyMap::get_tapdance_key(uint8_t index, uint8_t tap) {
    uint32_t eeprom_address = this->get_tapdance_eeprom_address(index, tap);

    uint8_t type = EEPROM.read(eeprom_address);
    uint8_t key = EEPROM.read(eeprom_address + 0x01);

    return KeyMap::init_key_info_without_coords(type, key);
}

void KeyMap::set_key(uint8_t layer, KeyInfo key_info) {
    uint8_t row = key_info.row;
    uint8_t col = key_info.col;

    if (row == ROW_UNKNOWN || col == COL_UNKNOWN) {
        return; // Invalid KeyInfo
    }

    int eeprom_address = this->get_eeprom_address(layer, row, col);

    EEPROM.update(eeprom_address + 0x00, key_info.type);
    EEPROM.update(eeprom_address + 0x01, key_info.key);
}

void KeyMap::set_tapdance_key(uint8_t index, uint8_t tap, KeyInfo key_info)  {
    uint32_t eeprom_address = this->get_tapdance_eeprom_address(index, tap);

    EEPROM.update(eeprom_address + 0x00, key_info.type);
    EEPROM.update(eeprom_address + 0x01, key_info.key);
}

void KeyMap::eeprom_clear_all() {
    for (uint32_t i = 0; i < EEPROM.length(); i++) {
        EEPROM.update(i, 0x00);
    }
}

void KeyMap::eeprom_clear_keymap() {
    for (uint8_t layer = 0; layer < MAX_LAYER_COUNT; layer++) {
        for (uint8_t row = 0; row < ROW_PIN_COUNT; row++) {
            for (uint8_t col = 0; col < 2*ONE_SIDE_COL_PIN_COUNT; col++) {
                KeyInfo ki = KeyMap::init_key_info(KEY_UNSET, 0x00, row, col);
                this->set_key(layer, ki);
            }
        }
    }
}

void KeyMap::eeprom_clear_tapdance() {
    for (uint8_t index = 0; index < MAX_TAPDANCE_KEYS; index++) {
        for (uint8_t tap = 1; tap <= MAX_TAPDANCE_TAPS; tap++) {
            KeyInfo ki = KeyMap::init_key_info_without_coords(KEY_UNSET, 0x00);
            this->set_tapdance_key(index, tap, ki);
        }
    }
}

inline int KeyMap::key_info_compare(KeyInfo key_info1, KeyInfo key_info2) {
    EEPROM_KeyInfo k1 = EEPROM_KeyInfo { key_info1.type, key_info1.key };
    EEPROM_KeyInfo k2 = EEPROM_KeyInfo { key_info2.type, key_info2.key };
    return memcmp(&k1, &k2, sizeof(EEPROM_KeyInfo));
}

inline uint8_t KeyMap::get_dual_key_modifier(KeyInfo key_info) {
    switch (key_info.type) {
        case KEY_DUAL_SINGLE_LCTRL:
        case KEY_DUAL_LCTRL:
            return KC_LCTRL;
        case KEY_DUAL_SINGLE_LSHIFT:
        case KEY_DUAL_LSHIFT:
            return KC_LSHIFT;
        case KEY_DUAL_SINGLE_LALT:
        case KEY_DUAL_LALT:
            return KC_LALT;
        case KEY_DUAL_SINGLE_LGUI:
        case KEY_DUAL_LGUI:
            return KC_LGUI;
        case KEY_DUAL_SINGLE_RCTRL:
        case KEY_DUAL_RCTRL:
            return KC_RCTRL;
        case KEY_DUAL_SINGLE_RSHIFT:
        case KEY_DUAL_RSHIFT:
            return KC_RSHIFT;
        case KEY_DUAL_SINGLE_RALT:
        case KEY_DUAL_RALT:
            return KC_RALT;
        case KEY_DUAL_SINGLE_RGUI:
        case KEY_DUAL_RGUI:
            return KC_RGUI;
    }
    return 0x00;
}

inline uint8_t KeyMap::get_dual_layer_key_layer(KeyInfo key_info) {
    switch (key_info.type) {
        case KEY_DUAL_LAYER_1:
        case KEY_DUAL_LAYER_SINGLE_1:
            return 1;
        case KEY_DUAL_LAYER_2:
        case KEY_DUAL_LAYER_SINGLE_2:
            return 2;
        case KEY_DUAL_LAYER_3:
        case KEY_DUAL_LAYER_SINGLE_3:
            return 3;
        case KEY_DUAL_LAYER_4:
        case KEY_DUAL_LAYER_SINGLE_4:
            return 4;
        case KEY_DUAL_LAYER_5:
        case KEY_DUAL_LAYER_SINGLE_5:
            return 5;
        case KEY_DUAL_LAYER_6:
        case KEY_DUAL_LAYER_SINGLE_6:
            return 6;
        case KEY_DUAL_LAYER_7:
        case KEY_DUAL_LAYER_SINGLE_7:
            return 7;
    }
    return 0;
}

inline uint8_t KeyMap::get_key_with_mod_modifier(KeyInfo key_info) {
    switch (key_info.type) {
        case KEY_WITH_MOD_LCTRL:  return KC_LCTRL;
        case KEY_WITH_MOD_LSHIFT: return KC_LSHIFT;
        case KEY_WITH_MOD_LALT:   return KC_LALT;
        case KEY_WITH_MOD_LGUI:   return KC_LGUI;
        case KEY_WITH_MOD_RCTRL:  return KC_RCTRL;
        case KEY_WITH_MOD_RSHIFT: return KC_RSHIFT;
        case KEY_WITH_MOD_RALT:   return KC_RALT;
        case KEY_WITH_MOD_RGUI:   return KC_RGUI;
    }
    return 0x00;
}

inline KeyInfo KeyMap::init_key_info(uint8_t type, uint8_t key, uint8_t row, uint8_t col) {
    return KeyInfo { type, key, row, col };
}

inline KeyInfo KeyMap::init_key_info_without_coords(uint8_t type, uint8_t key) {
    return KeyInfo { type, key, ROW_UNKNOWN, COL_UNKNOWN };
}

inline uint8_t KeyMap::is_type_between(KeyInfo key_info, uint8_t type1, uint8_t type2) {
    uint8_t type = key_info.type;
    if (type >= type1 && type <= type2) {
        return 1;
    }
    return 0;
}

inline uint8_t KeyMap::is_dual_key(KeyInfo key_info) {
    return KeyMap::is_type_between(key_info, KEY_DUAL_LCTRL, KEY_DUAL_RALT);
}

inline uint8_t KeyMap::is_dual_single_key(KeyInfo key_info) {
    return KeyMap::is_type_between(key_info, KEY_DUAL_SINGLE_LCTRL, KEY_DUAL_SINGLE_RALT);
}

inline uint8_t KeyMap::is_dual_layer_key(KeyInfo key_info) {
    return KeyMap::is_type_between(key_info, KEY_DUAL_LAYER_1, KEY_DUAL_LAYER_7);
}

inline uint8_t KeyMap::is_dual_layer_single_key(KeyInfo key_info) {
    return KeyMap::is_type_between(key_info, KEY_DUAL_LAYER_SINGLE_1, KEY_DUAL_LAYER_SINGLE_7);
}

inline uint8_t KeyMap::is_multimedia_key(KeyInfo key_info) {
    return KeyMap::is_type_between(key_info, KEY_MULTIMEDIA_0, KEY_MULTIMEDIA_2);
}

inline uint8_t KeyMap::is_key_with_mod(KeyInfo key_info) {
    return KeyMap::is_type_between(key_info, KEY_WITH_MOD_LCTRL, KEY_WITH_MOD_RALT);
}

inline const __FlashStringHelper* KeyMap::key_type_to_string(KeyInfo key_info) {
         if (key_info.type == KEY_UNSET)                { return F("KEY_UNSET");                }
    else if (key_info.type == KEY_NORMAL)               { return F("KEY_NORMAL");               }
    else if (key_info.type == KEY_DUAL_LCTRL)           { return F("KEY_DUAL_LCTRL");           }
    else if (key_info.type == KEY_DUAL_RCTRL)           { return F("KEY_DUAL_RCTRL");           }
    else if (key_info.type == KEY_DUAL_LSHIFT)          { return F("KEY_DUAL_LSHIFT");          }
    else if (key_info.type == KEY_DUAL_RSHIFT)          { return F("KEY_DUAL_RSHIFT");          }
    else if (key_info.type == KEY_DUAL_LGUI)            { return F("KEY_DUAL_LGUI");            }
    else if (key_info.type == KEY_DUAL_RGUI)            { return F("KEY_DUAL_RGUI");            }
    else if (key_info.type == KEY_DUAL_LALT)            { return F("KEY_DUAL_LALT");            }
    else if (key_info.type == KEY_DUAL_RALT)            { return F("KEY_DUAL_RALT");            }
    else if (key_info.type == KEY_LAYER_PRESS)          { return F("KEY_LAYER_PRESS");          }
    else if (key_info.type == KEY_LAYER_TOGGLE)         { return F("KEY_LAYER_TOGGLE");         }
    else if (key_info.type == KEY_LAYER_HOLD_OR_TOGGLE) { return F("KEY_LAYER_HOLD_OR_TOGGLE"); }
    else if (key_info.type == KEY_WITH_MOD_LCTRL)       { return F("KEY_WITH_MOD_LCTRL");       }
    else if (key_info.type == KEY_WITH_MOD_RCTRL)       { return F("KEY_WITH_MOD_RCTRL");       }
    else if (key_info.type == KEY_WITH_MOD_LSHIFT)      { return F("KEY_WITH_MOD_LSHIFT");      }
    else if (key_info.type == KEY_WITH_MOD_RSHIFT)      { return F("KEY_WITH_MOD_RSHIFT");      }
    else if (key_info.type == KEY_WITH_MOD_LGUI)        { return F("KEY_WITH_MOD_LGUI");        }
    else if (key_info.type == KEY_WITH_MOD_RGUI)        { return F("KEY_WITH_MOD_RGUI");        }
    else if (key_info.type == KEY_WITH_MOD_LALT)        { return F("KEY_WITH_MOD_LALT");        }
    else if (key_info.type == KEY_WITH_MOD_RALT)        { return F("KEY_WITH_MOD_RALT");        }
    else if (key_info.type == KEY_SYSTEM)               { return F("KEY_SYSTEM");               }
    else if (key_info.type == KEY_MULTIMEDIA_0)         { return F("KEY_MULTIMEDIA_0");         }
    else if (key_info.type == KEY_MULTIMEDIA_1)         { return F("KEY_MULTIMEDIA_1");         }
    else if (key_info.type == KEY_MULTIMEDIA_2)         { return F("KEY_MULTIMEDIA_2");         }
    else if (key_info.type == KEY_TAPDANCE)             { return F("KEY_TAPDANCE");             }
    else if (key_info.type == KEY_DUAL_LAYER_1)         { return F("KEY_DUAL_LAYER_1");         }
    else if (key_info.type == KEY_DUAL_LAYER_2)         { return F("KEY_DUAL_LAYER_2");         }
    else if (key_info.type == KEY_DUAL_LAYER_3)         { return F("KEY_DUAL_LAYER_3");         }
    else if (key_info.type == KEY_DUAL_LAYER_4)         { return F("KEY_DUAL_LAYER_4");         }
    else if (key_info.type == KEY_DUAL_LAYER_5)         { return F("KEY_DUAL_LAYER_5");         }
    else if (key_info.type == KEY_DUAL_LAYER_6)         { return F("KEY_DUAL_LAYER_6");         }
    else if (key_info.type == KEY_DUAL_LAYER_7)         { return F("KEY_DUAL_LAYER_7");         }
    else if (key_info.type == KEY_DUAL_LAYER_SINGLE_1)  { return F("KEY_DUAL_LAYER_SINGLE_1");  }
    else if (key_info.type == KEY_DUAL_LAYER_SINGLE_2)  { return F("KEY_DUAL_LAYER_SINGLE_2");  }
    else if (key_info.type == KEY_DUAL_LAYER_SINGLE_3)  { return F("KEY_DUAL_LAYER_SINGLE_3");  }
    else if (key_info.type == KEY_DUAL_LAYER_SINGLE_4)  { return F("KEY_DUAL_LAYER_SINGLE_4");  }
    else if (key_info.type == KEY_DUAL_LAYER_SINGLE_5)  { return F("KEY_DUAL_LAYER_SINGLE_5");  }
    else if (key_info.type == KEY_DUAL_LAYER_SINGLE_6)  { return F("KEY_DUAL_LAYER_SINGLE_6");  }
    else if (key_info.type == KEY_DUAL_LAYER_SINGLE_7)  { return F("KEY_DUAL_LAYER_SINGLE_7");  }
    else if (key_info.type == KEY_DUAL_SINGLE_LCTRL)    { return F("KEY_DUAL_SINGLE_LCTRL");    }
    else if (key_info.type == KEY_DUAL_SINGLE_RCTRL)    { return F("KEY_DUAL_SINGLE_RCTRL");    }
    else if (key_info.type == KEY_DUAL_SINGLE_LSHIFT)   { return F("KEY_DUAL_SINGLE_LSHIFT");   }
    else if (key_info.type == KEY_DUAL_SINGLE_RSHIFT)   { return F("KEY_DUAL_SINGLE_RSHIFT");   }
    else if (key_info.type == KEY_DUAL_SINGLE_LGUI)     { return F("KEY_DUAL_SINGLE_LGUI");     }
    else if (key_info.type == KEY_DUAL_SINGLE_RGUI)     { return F("KEY_DUAL_SINGLE_RGUI");     }
    else if (key_info.type == KEY_DUAL_SINGLE_LALT)     { return F("KEY_DUAL_SINGLE_LALT");     }
    else if (key_info.type == KEY_DUAL_SINGLE_RALT)     { return F("KEY_DUAL_SINGLE_RALT");     }
    else if (key_info.type == KEY_TRANSPARENT)          { return F("KEY_TRANSPARENT");          }
    else                                                { return F("KEY_TYPE_UNKNOWN");         }
}

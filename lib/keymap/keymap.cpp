#include <Arduino.h>
#include <string.h>
#include "dalsik.h"
#include "dalsik_eeprom.h"
#include "keymap.h"
#include "array_utils.h"
#include "dalsik_led.h"

KeyMap::KeyMap() {
    this->layer_index = 0;
    this->toggled_layer_index = 0;
    this->keyboard_side = EEPROM::get_keyboard_side();
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
KeyInfo KeyMap::get_master_key(KeyCoords c) {
    if (this->keyboard_side == KEYBOARD_SIDE_RIGHT) {
        c.col = 2*ONE_SIDE_COL_PIN_COUNT - c.col - 1;
    }
    return this->get_key(c);
}

KeyInfo KeyMap::get_slave_key(KeyCoords c) {
    // KeyMap is used only on the master side; The slave is the right side if the master is the left
    if (this->keyboard_side == KEYBOARD_SIDE_LEFT) {
        c.col = 2*ONE_SIDE_COL_PIN_COUNT - c.col - 1;
    }
    return this->get_key(c);
}

KeyInfo KeyMap::get_key(KeyCoords c) {
    EEPROM_KeyInfo eeprom_key_info = EEPROM::get_key(this->layer_index, c);

    KeyInfo key_info = KeyMap::init_key_info(eeprom_key_info, c);
    if (key_info.type == KEY_TRANSPARENT) { // Get the key from lower layers
        key_info = this->get_non_transparent_key(c);
    }

    return key_info;
}

void KeyMap::reload_key_info_by_row_col(KeyInfo* ki) {
    if (ki->coords.row == ROW_UNKNOWN || ki->coords.col == COL_UNKNOWN) {
        return; // Missing coords info
    }

    KeyInfo new_ki = this->get_key(ki->coords);
    ki->type = new_ki.type;
    ki->key = new_ki.key;
}

void KeyMap::update_keyboard_side(uint8_t side) {
    if (side == KEYBOARD_SIDE_LEFT || side == KEYBOARD_SIDE_RIGHT) {
        EEPROM::set_keyboard_side(side);
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

KeyInfo KeyMap::get_key_from_layer(uint8_t layer, KeyCoords c) {
    EEPROM_KeyInfo eeprom_key_info = EEPROM::get_key(layer, c);
    return KeyMap::init_key_info(eeprom_key_info, c);
}

KeyInfo KeyMap::get_non_transparent_key(KeyCoords c) {
    for (int8_t i = LAYER_HISTORY_CAPACITY-1; i >= 0; i--) {
        uint8_t layer = this->layer_history[i];
        if (layer == 0x00) {
            continue;
        }

        KeyInfo key_info = this->get_key_from_layer(layer, c);
        if (key_info.type != KEY_TRANSPARENT) {
            return key_info;
        }
    }

    KeyInfo key_info = this->get_key_from_layer(0, c);
    if (key_info.type == KEY_TRANSPARENT) {
        return KeyMap::init_key_info(KEY_UNSET, 0x00, c);
    } else {
        return key_info;
    }
}

KeyInfo KeyMap::get_tapdance_key(uint8_t index, uint8_t tap) {
    EEPROM_KeyInfo key_info = EEPROM::get_tapdance_key(index, tap);
    return KeyMap::init_key_info_without_coords(key_info.type, key_info.key);
}

void KeyMap::set_key(uint8_t layer, KeyInfo key_info) {
    uint8_t row = key_info.coords.row;
    uint8_t col = key_info.coords.col;

    if (row == ROW_UNKNOWN || col == COL_UNKNOWN) {
        return; // Invalid KeyInfo
    }

    EEPROM::set_key(layer, key_info);
}

void KeyMap::set_tapdance_key(uint8_t index, uint8_t tap, KeyInfo key_info)  {
    EEPROM::set_tapdance_key(index, tap, key_info);
}

void KeyMap::eeprom_clear_all() {
    EEPROM::clear_all();
}

void KeyMap::eeprom_clear_keymap() {
    EEPROM::clear_keymap();
}

void KeyMap::eeprom_clear_tapdance() {
    EEPROM::clear_tapdance();
}

int KeyMap::key_info_compare(KeyInfo key_info1, KeyInfo key_info2) {
    EEPROM_KeyInfo k1 = EEPROM_KeyInfo { key_info1.type, key_info1.key };
    EEPROM_KeyInfo k2 = EEPROM_KeyInfo { key_info2.type, key_info2.key };
    return memcmp(&k1, &k2, sizeof(EEPROM_KeyInfo));
}

uint8_t KeyMap::get_dual_key_modifier(KeyInfo key_info) {
    switch (key_info.type) {
        case KEY_DUAL_MOD_LCTRL:
        case KEY_SINGLE_DUAL_MOD_LCTRL:
        case KEY_TIMED_DUAL_MOD_LCTRL:
            return KC_LCTRL;
        case KEY_DUAL_MOD_LSHIFT:
        case KEY_SINGLE_DUAL_MOD_LSHIFT:
        case KEY_TIMED_DUAL_MOD_LSHIFT:
            return KC_LSHIFT;
        case KEY_DUAL_MOD_LALT:
        case KEY_SINGLE_DUAL_MOD_LALT:
        case KEY_TIMED_DUAL_MOD_LALT:
            return KC_LALT;
        case KEY_DUAL_MOD_LGUI:
        case KEY_SINGLE_DUAL_MOD_LGUI:
        case KEY_TIMED_DUAL_MOD_LGUI:
            return KC_LGUI;
        case KEY_DUAL_MOD_RCTRL:
        case KEY_SINGLE_DUAL_MOD_RCTRL:
        case KEY_TIMED_DUAL_MOD_RCTRL:
            return KC_RCTRL;
        case KEY_DUAL_MOD_RSHIFT:
        case KEY_SINGLE_DUAL_MOD_RSHIFT:
        case KEY_TIMED_DUAL_MOD_RSHIFT:
            return KC_RSHIFT;
        case KEY_DUAL_MOD_RALT:
        case KEY_SINGLE_DUAL_MOD_RALT:
        case KEY_TIMED_DUAL_MOD_RALT:
            return KC_RALT;
        case KEY_DUAL_MOD_RGUI:
        case KEY_SINGLE_DUAL_MOD_RGUI:
        case KEY_TIMED_DUAL_MOD_RGUI:
            return KC_RGUI;
    }
    return 0x00;
}

uint8_t KeyMap::get_dual_layer_key_layer(KeyInfo key_info) {
    switch (key_info.type) {
        case KEY_DUAL_LAYER_1:
        case KEY_SINGLE_DUAL_LAYER_1:
        case KEY_TIMED_DUAL_LAYER_1:
            return 1;
        case KEY_DUAL_LAYER_2:
        case KEY_SINGLE_DUAL_LAYER_2:
        case KEY_TIMED_DUAL_LAYER_2:
            return 2;
        case KEY_DUAL_LAYER_3:
        case KEY_SINGLE_DUAL_LAYER_3:
        case KEY_TIMED_DUAL_LAYER_3:
            return 3;
        case KEY_DUAL_LAYER_4:
        case KEY_SINGLE_DUAL_LAYER_4:
        case KEY_TIMED_DUAL_LAYER_4:
            return 4;
        case KEY_DUAL_LAYER_5:
        case KEY_SINGLE_DUAL_LAYER_5:
        case KEY_TIMED_DUAL_LAYER_5:
            return 5;
        case KEY_DUAL_LAYER_6:
        case KEY_SINGLE_DUAL_LAYER_6:
        case KEY_TIMED_DUAL_LAYER_6:
            return 6;
        case KEY_DUAL_LAYER_7:
        case KEY_SINGLE_DUAL_LAYER_7:
        case KEY_TIMED_DUAL_LAYER_7:
            return 7;
    }
    return 0;
}

uint8_t KeyMap::get_key_with_mod_modifier(KeyInfo key_info) {
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

KeyInfo KeyMap::init_key_info(uint8_t type, uint8_t key, KeyCoords coords) {
    return KeyInfo { type, key, coords };
}

KeyInfo KeyMap::init_key_info(EEPROM_KeyInfo eeprom_key, KeyCoords c) {
    return KeyInfo { eeprom_key.type, eeprom_key.key, c };
}

KeyInfo KeyMap::init_key_info_without_coords(uint8_t type, uint8_t key) {
    return KeyInfo { type, key, KeyCoords { ROW_UNKNOWN, COL_UNKNOWN } };
}

uint8_t KeyMap::is_type_between(KeyInfo key_info, uint8_t type1, uint8_t type2) {
    uint8_t type = key_info.type;
    if (type >= type1 && type <= type2) {
        return 1;
    }
    return 0;
}

uint8_t KeyMap::is_dual_key(KeyInfo key_info) {
    return KeyMap::is_type_between(key_info, KEY_DUAL_MOD_LCTRL, KEY_DUAL_MOD_RALT);
}

uint8_t KeyMap::is_single_dual_key(KeyInfo key_info) {
    return KeyMap::is_type_between(key_info, KEY_SINGLE_DUAL_MOD_LCTRL, KEY_SINGLE_DUAL_MOD_RALT);
}

uint8_t KeyMap::is_dual_layer_key(KeyInfo key_info) {
    return KeyMap::is_type_between(key_info, KEY_DUAL_LAYER_1, KEY_DUAL_LAYER_7);
}

uint8_t KeyMap::is_single_dual_layer_key(KeyInfo key_info) {
    return KeyMap::is_type_between(key_info, KEY_SINGLE_DUAL_LAYER_1, KEY_SINGLE_DUAL_LAYER_7);
}

uint8_t KeyMap::is_multimedia_key(KeyInfo key_info) {
    return KeyMap::is_type_between(key_info, KEY_MULTIMEDIA_0, KEY_MULTIMEDIA_2);
}

uint8_t KeyMap::is_key_with_mod(KeyInfo key_info) {
    return KeyMap::is_type_between(key_info, KEY_WITH_MOD_LCTRL, KEY_WITH_MOD_RALT);
}

const __FlashStringHelper* KeyMap::key_type_to_string(KeyInfo key_info) {
         if (key_info.type == KEY_UNSET)                  { return F("KEY_UNSET");                }
    else if (key_info.type == KEY_NORMAL)                 { return F("KEY_NORMAL");               }
    else if (key_info.type == KEY_DUAL_MOD_LCTRL)             { return F("KEY_DUAL_MOD_LCTRL");           }
    else if (key_info.type == KEY_DUAL_MOD_RCTRL)             { return F("KEY_DUAL_MOD_RCTRL");           }
    else if (key_info.type == KEY_DUAL_MOD_LSHIFT)            { return F("KEY_DUAL_MOD_LSHIFT");          }
    else if (key_info.type == KEY_DUAL_MOD_RSHIFT)            { return F("KEY_DUAL_MOD_RSHIFT");          }
    else if (key_info.type == KEY_DUAL_MOD_LGUI)              { return F("KEY_DUAL_MOD_LGUI");            }
    else if (key_info.type == KEY_DUAL_MOD_RGUI)              { return F("KEY_DUAL_MOD_RGUI");            }
    else if (key_info.type == KEY_DUAL_MOD_LALT)              { return F("KEY_DUAL_MOD_LALT");            }
    else if (key_info.type == KEY_DUAL_MOD_RALT)              { return F("KEY_DUAL_MOD_RALT");            }
    else if (key_info.type == KEY_LAYER_PRESS)            { return F("KEY_LAYER_PRESS");          }
    else if (key_info.type == KEY_LAYER_TOGGLE)           { return F("KEY_LAYER_TOGGLE");         }
    else if (key_info.type == KEY_LAYER_TOGGLE_OR_HOLD)   { return F("KEY_LAYER_TOGGLE_OR_HOLD"); }
    else if (key_info.type == KEY_WITH_MOD_LCTRL)         { return F("KEY_WITH_MOD_LCTRL");       }
    else if (key_info.type == KEY_WITH_MOD_RCTRL)         { return F("KEY_WITH_MOD_RCTRL");       }
    else if (key_info.type == KEY_WITH_MOD_LSHIFT)        { return F("KEY_WITH_MOD_LSHIFT");      }
    else if (key_info.type == KEY_WITH_MOD_RSHIFT)        { return F("KEY_WITH_MOD_RSHIFT");      }
    else if (key_info.type == KEY_WITH_MOD_LGUI)          { return F("KEY_WITH_MOD_LGUI");        }
    else if (key_info.type == KEY_WITH_MOD_RGUI)          { return F("KEY_WITH_MOD_RGUI");        }
    else if (key_info.type == KEY_WITH_MOD_LALT)          { return F("KEY_WITH_MOD_LALT");        }
    else if (key_info.type == KEY_WITH_MOD_RALT)          { return F("KEY_WITH_MOD_RALT");        }
    else if (key_info.type == KEY_SYSTEM)                 { return F("KEY_SYSTEM");               }
    else if (key_info.type == KEY_MULTIMEDIA_0)           { return F("KEY_MULTIMEDIA_0");         }
    else if (key_info.type == KEY_MULTIMEDIA_1)           { return F("KEY_MULTIMEDIA_1");         }
    else if (key_info.type == KEY_MULTIMEDIA_2)           { return F("KEY_MULTIMEDIA_2");         }
    else if (key_info.type == KEY_TAPDANCE)               { return F("KEY_TAPDANCE");             }
    else if (key_info.type == KEY_DUAL_LAYER_1)           { return F("KEY_DUAL_LAYER_1");         }
    else if (key_info.type == KEY_DUAL_LAYER_2)           { return F("KEY_DUAL_LAYER_2");         }
    else if (key_info.type == KEY_DUAL_LAYER_3)           { return F("KEY_DUAL_LAYER_3");         }
    else if (key_info.type == KEY_DUAL_LAYER_4)           { return F("KEY_DUAL_LAYER_4");         }
    else if (key_info.type == KEY_DUAL_LAYER_5)           { return F("KEY_DUAL_LAYER_5");         }
    else if (key_info.type == KEY_DUAL_LAYER_6)           { return F("KEY_DUAL_LAYER_6");         }
    else if (key_info.type == KEY_DUAL_LAYER_7)           { return F("KEY_DUAL_LAYER_7");         }
    else if (key_info.type == KEY_SINGLE_DUAL_LAYER_1)    { return F("KEY_SINGLE_DUAL_LAYER_1");  }
    else if (key_info.type == KEY_SINGLE_DUAL_LAYER_2)    { return F("KEY_SINGLE_DUAL_LAYER_2");  }
    else if (key_info.type == KEY_SINGLE_DUAL_LAYER_3)    { return F("KEY_SINGLE_DUAL_LAYER_3");  }
    else if (key_info.type == KEY_SINGLE_DUAL_LAYER_4)    { return F("KEY_SINGLE_DUAL_LAYER_4");  }
    else if (key_info.type == KEY_SINGLE_DUAL_LAYER_5)    { return F("KEY_SINGLE_DUAL_LAYER_5");  }
    else if (key_info.type == KEY_SINGLE_DUAL_LAYER_6)    { return F("KEY_SINGLE_DUAL_LAYER_6");  }
    else if (key_info.type == KEY_SINGLE_DUAL_LAYER_7)    { return F("KEY_SINGLE_DUAL_LAYER_7");  }
    else if (key_info.type == KEY_SINGLE_DUAL_MOD_LCTRL)      { return F("KEY_SINGLE_DUAL_MOD_LCTRL");    }
    else if (key_info.type == KEY_SINGLE_DUAL_MOD_RCTRL)      { return F("KEY_SINGLE_DUAL_MOD_RCTRL");    }
    else if (key_info.type == KEY_SINGLE_DUAL_MOD_LSHIFT)     { return F("KEY_SINGLE_DUAL_MOD_LSHIFT");   }
    else if (key_info.type == KEY_SINGLE_DUAL_MOD_RSHIFT)     { return F("KEY_SINGLE_DUAL_MOD_RSHIFT");   }
    else if (key_info.type == KEY_SINGLE_DUAL_MOD_LGUI)       { return F("KEY_SINGLE_DUAL_MOD_LGUI");     }
    else if (key_info.type == KEY_SINGLE_DUAL_MOD_RGUI)       { return F("KEY_SINGLE_DUAL_MOD_RGUI");     }
    else if (key_info.type == KEY_SINGLE_DUAL_MOD_LALT)       { return F("KEY_SINGLE_DUAL_MOD_LALT");     }
    else if (key_info.type == KEY_SINGLE_DUAL_MOD_RALT)       { return F("KEY_SINGLE_DUAL_MOD_RALT");     }
    else if (key_info.type == KEY_TIMED_DUAL_MOD_LCTRL)       { return F("KEY_TIMED_DUAL_MOD_LCTRL");     }
    else if (key_info.type == KEY_TIMED_DUAL_MOD_RCTRL)       { return F("KEY_TIMED_DUAL_MOD_RCTRL");     }
    else if (key_info.type == KEY_TIMED_DUAL_MOD_LSHIFT)      { return F("KEY_TIMED_DUAL_MOD_LSHIFT");    }
    else if (key_info.type == KEY_TIMED_DUAL_MOD_RSHIFT)      { return F("KEY_TIMED_DUAL_MOD_RSHIFT");    }
    else if (key_info.type == KEY_TIMED_DUAL_MOD_LGUI)        { return F("KEY_TIMED_DUAL_MOD_LGUI");      }
    else if (key_info.type == KEY_TIMED_DUAL_MOD_RGUI)        { return F("KEY_TIMED_DUAL_MOD_RGUI");      }
    else if (key_info.type == KEY_TIMED_DUAL_MOD_LALT)        { return F("KEY_TIMED_DUAL_MOD_LALT");      }
    else if (key_info.type == KEY_TIMED_DUAL_MOD_RALT)        { return F("KEY_TIMED_DUAL_MOD_RALT");      }
    else if (key_info.type == KEY_TIMED_DUAL_LAYER_1)     { return F("KEY_TIMED_DUAL_LAYER_1");   }
    else if (key_info.type == KEY_TIMED_DUAL_LAYER_2)     { return F("KEY_TIMED_DUAL_LAYER_2");   }
    else if (key_info.type == KEY_TIMED_DUAL_LAYER_3)     { return F("KEY_TIMED_DUAL_LAYER_3");   }
    else if (key_info.type == KEY_TIMED_DUAL_LAYER_4)     { return F("KEY_TIMED_DUAL_LAYER_4");   }
    else if (key_info.type == KEY_TIMED_DUAL_LAYER_5)     { return F("KEY_TIMED_DUAL_LAYER_5");   }
    else if (key_info.type == KEY_TIMED_DUAL_LAYER_6)     { return F("KEY_TIMED_DUAL_LAYER_6");   }
    else if (key_info.type == KEY_TIMED_DUAL_LAYER_7)     { return F("KEY_TIMED_DUAL_LAYER_7");   }
    else if (key_info.type == KEY_TRANSPARENT)            { return F("KEY_TRANSPARENT");          }
    else                                                  { return F("KEY_TYPE_UNKNOWN");         }
}

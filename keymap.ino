#include "EEPROM.h"
#include "dalsik.h"
#include "keymap.h"
#include "array_utils.h"

KeyMap::KeyMap() {
    this->layer_index = 0;
    this->toggled_layer_index = 0;
    this->clear();
}

uint8_t KeyMap::get_layer() {
    return this->layer_index;
}

void KeyMap::set_layer(uint8_t layer) {
    this->layer_index = layer;
    append_uniq_to_uint8_array(this->layer_history, LAYER_HISTORY_CAPACITY, layer);
}

// We could have pressed multiple layer keys, so releasing one means we switch to the other one
void KeyMap::remove_layer(uint8_t layer) {
    remove_uniq_from_uint8_array(this->layer_history, LAYER_HISTORY_CAPACITY, layer);
    uint8_t prev_layer = last_nonzero_elem_of_uint8_array(
        this->layer_history, LAYER_HISTORY_CAPACITY
    );

    if (prev_layer > 0) {
        this->layer_index = prev_layer;
    } else {
        this->layer_index = this->toggled_layer_index; // toggled layer or 0
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

void KeyMap::clear() {
    this->layer_index = 0;
    memset(this->layer_history, 0, LAYER_HISTORY_CAPACITY);
    if (this->toggled_layer_index > 0) {
        this->set_layer(this->toggled_layer_index);
    }
}

inline int KeyMap::get_eeprom_address(uint8_t layer, uint8_t row, uint8_t col) {
    return sizeof(KeyInfo)*( layer*KEY_COUNT + row*BOTH_SIDE_COL_PIN_COUNT + col );
}

KeyInfo KeyMap::get_key_from_layer(uint8_t layer, uint8_t row, uint8_t col) {
    int eeprom_address = this->get_eeprom_address(layer, row, col);

    uint8_t type = EEPROM.read(eeprom_address + 0x00);
    uint8_t key = EEPROM.read(eeprom_address + 0x01);

    KeyInfo key_info = { type, key };

    return key_info;
}

KeyInfo KeyMap::get_key(uint8_t row, uint8_t col) {
    int eeprom_address = this->get_eeprom_address(this->layer_index, row, col);

    uint8_t type = EEPROM.read(eeprom_address + 0x00);
    uint8_t key = EEPROM.read(eeprom_address + 0x01);

    KeyInfo key_info = { type, key };

    return key_info;
}

KeyInfo KeyMap::get_non_transparent_key(uint8_t row, uint8_t col) {
    for (uint8_t i = LAYER_HISTORY_CAPACITY-1; i >= 0; i--) {
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
        return { KEY_UNSET, 0x00 };
    } else {
        return key_info;
    }
}

void KeyMap::set_key(uint8_t layer, uint8_t row, uint8_t col, KeyInfo key_info) {
    int eeprom_address = this->get_eeprom_address(this->layer_index, row, col);

    EEPROM.update(eeprom_address + 0x00, key_info.type);
    EEPROM.update(eeprom_address + 0x01, key_info.key);
}

void KeyMap::eeprom_clear() {
    for (uint32_t i = 0; i < EEPROM.length(); i++) {
        EEPROM.update(i, 0x00);
    }
}

inline int key_info_compare(KeyInfo key_info1, KeyInfo key_info2) {
    return memcmp(&key_info1, &key_info2, sizeof(KeyInfo));
}

inline uint8_t get_dual_key_modifier(KeyInfo key_info) {
    switch (key_info.type) {
        case KEY_DUAL_LCTRL:  return 0xE0;
        case KEY_DUAL_LSHIFT: return 0xE1;
        case KEY_DUAL_LALT:   return 0xE2;
        case KEY_DUAL_LGUI:   return 0xE3;
        case KEY_DUAL_RCTRL:  return 0xE4;
        case KEY_DUAL_RSHIFT: return 0xE5;
        case KEY_DUAL_RALT:   return 0xE6;
        case KEY_DUAL_RGUI:   return 0xE7;
    }
    return 0x00;
}

inline uint8_t get_key_with_mod_modifier(KeyInfo key_info) {
    switch (key_info.type) {
        case KEY_WITH_MOD_LCTRL:  return 0xE0;
        case KEY_WITH_MOD_LSHIFT: return 0xE1;
        case KEY_WITH_MOD_LALT:   return 0xE2;
        case KEY_WITH_MOD_LGUI:   return 0xE3;
        case KEY_WITH_MOD_RCTRL:  return 0xE4;
        case KEY_WITH_MOD_RSHIFT: return 0xE5;
        case KEY_WITH_MOD_RALT:   return 0xE6;
        case KEY_WITH_MOD_RGUI:   return 0xE7;
    }
    return 0x00;
}

inline uint8_t is_dual_key(KeyInfo key_info) {
    uint8_t type = key_info.type;
    if (type >= KEY_DUAL_LCTRL && type <= KEY_DUAL_RALT) {
        return 1;
    }
    return 0;
}

inline uint8_t is_key_with_mod(KeyInfo key_info) {
    uint8_t type = key_info.type;
    if (type >= KEY_WITH_MOD_LCTRL && type <= KEY_WITH_MOD_RALT) {
        return 1;
    }
    return 0;
}

const inline char* key_type_to_string(KeyInfo key_info) {
    switch (key_info.type) {
        case KEY_UNSET:                return "KEY_UNSET";
        case KEY_NORMAL:               return "KEY_NORMAL";
        case KEY_DUAL_LCTRL:           return "KEY_DUAL_LCTRL";
        case KEY_DUAL_RCTRL:           return "KEY_DUAL_RCTRL";
        case KEY_DUAL_LSHIFT:          return "KEY_DUAL_LSHIFT";
        case KEY_DUAL_RSHIFT:          return "KEY_DUAL_RSHIFT";
        case KEY_DUAL_LGUI:            return "KEY_DUAL_LGUI";
        case KEY_DUAL_RGUI:            return "KEY_DUAL_RGUI";
        case KEY_DUAL_LALT:            return "KEY_DUAL_LALT";
        case KEY_DUAL_RALT:            return "KEY_DUAL_RALT";
        case KEY_WITH_MOD_LCTRL:       return "KEY_DUAL_LCTRL";
        case KEY_WITH_MOD_RCTRL:       return "KEY_DUAL_RCTRL";
        case KEY_WITH_MOD_LSHIFT:      return "KEY_DUAL_LSHIFT";
        case KEY_WITH_MOD_RSHIFT:      return "KEY_DUAL_RSHIFT";
        case KEY_WITH_MOD_LGUI:        return "KEY_DUAL_LGUI";
        case KEY_WITH_MOD_RGUI:        return "KEY_DUAL_RGUI";
        case KEY_WITH_MOD_LALT:        return "KEY_DUAL_LALT";
        case KEY_WITH_MOD_RALT:        return "KEY_DUAL_RALT";
        case KEY_LAYER_PRESS:          return "KEY_LAYER_PRESS";
        case KEY_LAYER_TOGGLE:         return "KEY_LAYER_TOGGLE";
        case KEY_TRANSPARENT:          return "KEY_TRANSPARENT";
        default:                       return "KEY_TYPE_UNKNOWN";
    }
}

#pragma once

#include <Arduino.h>
#include "dalsik.h"
#include "key_definitions.h"

// EEPROM - 2B/key; 48keys; 8layers; 2*48*8 = 768B
// keymap : 0-767
// tapdance keys : 800-899

#define TAPDANCE_EEPROM_OFFSET (sizeof(EEPROM_KeyInfo)*MAX_LAYER_COUNT*KEY_COUNT)

#ifdef LED_PIN
const uint32_t LED_LAYER_COLORS[MAX_LAYER_COUNT] = {
    0x00000000, // Layer 0 - default (LED off)
    0x00FF0000,
    0x0000FF00,
    0x000000FF,
    0x00FFFF00,
    0x00FF00FF,
    0x00000000,
    0x00000000,
};
#endif

class KeyMap {
    private:
        uint8_t layer_index;
        uint8_t toggled_layer_index;
        uint8_t keyboard_side;
        uint8_t layer_history[LAYER_HISTORY_CAPACITY];

        uint32_t get_eeprom_address(uint8_t layer, KeyCoords c);
        uint32_t get_tapdance_eeprom_address(uint8_t index, uint8_t tap);
        KeyInfo get_non_transparent_key(KeyCoords c);
        KeyInfo get_key(KeyCoords c);
    public:
        KeyMap(void);

        KeyInfo get_master_key(KeyCoords c);
        KeyInfo get_slave_key(KeyCoords c);
        KeyInfo get_key_from_layer(uint8_t layer, KeyCoords c);
        KeyInfo get_tapdance_key(uint8_t index, uint8_t tap);
        void reload_key_info_by_row_col(KeyInfo* ki);

        void update_keyboard_side(uint8_t side);
        uint8_t get_keyboard_side();

        void set_key(uint8_t layer, KeyInfo key);
        void set_tapdance_key(uint8_t index, uint8_t tap, KeyInfo key_info);
        uint8_t get_layer();
        void set_layer(uint8_t layer);
        void remove_layer(uint8_t layer);
        void toggle_layer(uint8_t layer);
        void eeprom_clear_all();
        void eeprom_clear_keymap();
        void eeprom_clear_tapdance();
        void clear();

        static uint8_t is_type_between(KeyInfo key_info, uint8_t type1, uint8_t type2);
        static uint8_t is_key_with_mod(KeyInfo key_info);
        static uint8_t is_dual_key(KeyInfo key_info);
        static uint8_t is_single_dual_key(KeyInfo key_info);
        static uint8_t is_dual_layer_key(KeyInfo key_info);
        static uint8_t is_single_dual_layer_key(KeyInfo key_info);
        static uint8_t is_multimedia_key(KeyInfo key_info);
        static int key_info_compare(KeyInfo key_info1, KeyInfo key_info2);
        static uint8_t get_dual_key_modifier(KeyInfo key_info);
        static uint8_t get_dual_layer_key_layer(KeyInfo key_info);
        static uint8_t get_key_with_mod_modifier(KeyInfo key_info);

        static KeyInfo init_key_info(uint8_t type, uint8_t key, KeyCoords c);
        static KeyInfo init_key_info(EEPROM_KeyInfo eeprom_key, KeyCoords c);
        static KeyInfo init_key_info_without_coords(uint8_t type, uint8_t key);

        // For Serial.print() usage only
        static const __FlashStringHelper* key_type_to_string(KeyInfo key_info);
};

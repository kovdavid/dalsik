#ifndef KEYMAP_H
#define KEYMAP_H

#include "dalsik.h"

#define KEY_UNSET                0x00
#define KEY_NORMAL               0x01
#define KEY_DUAL_LCTRL           0x02
#define KEY_DUAL_RCTRL           0x03
#define KEY_DUAL_LSHIFT          0x04
#define KEY_DUAL_RSHIFT          0x05
#define KEY_DUAL_LGUI            0x06
#define KEY_DUAL_RGUI            0x07
#define KEY_DUAL_LALT            0x08
#define KEY_DUAL_RALT            0x09
#define KEY_LAYER_PRESS          0x0A
#define KEY_LAYER_TOGGLE         0x0B
#define KEY_LAYER_HOLD_OR_TOGGLE 0x0C
#define KEY_WITH_MOD_LCTRL       0x0D
#define KEY_WITH_MOD_RCTRL       0x0E
#define KEY_WITH_MOD_LSHIFT      0x0F
#define KEY_WITH_MOD_RSHIFT      0x10
#define KEY_WITH_MOD_LGUI        0x11
#define KEY_WITH_MOD_RGUI        0x12
#define KEY_WITH_MOD_LALT        0x13
#define KEY_WITH_MOD_RALT        0x14
#define KEY_SYSTEM               0x15
#define KEY_MULTIMEDIA_0         0x16
#define KEY_MULTIMEDIA_1         0x17
#define KEY_MULTIMEDIA_2         0x18
#define KEY_TAPDANCE             0x19
#define KEY_DUAL_LAYER_1         0x1A
#define KEY_DUAL_LAYER_2         0x1B
#define KEY_DUAL_LAYER_3         0x1C
#define KEY_DUAL_LAYER_4         0x1D
#define KEY_DUAL_LAYER_5         0x1E
#define KEY_DUAL_LAYER_6         0x1F
#define KEY_DUAL_LAYER_7         0x20
#define KEY_TRANSPARENT          0xFF

// KEY_SYSTEM
// KEY_SYSTEM_POWER_OFF = 0x81
// KEY_SYSTEM_SLEEP     = 0x82
// KEY_SYSTEM_WAKE_UP   = 0x83

// KEY_MULTIMEDIA_0 - report[1] = 0x00
// KEY_MULTIMEDIA_MUTE        -> report[0] = 0xE2
// KEY_MULTIMEDIA_VOLUME_UP   -> report[0] = 0xE9
// KEY_MULTIMEDIA_VOLUME_DOWN -> report[0] = 0xEA

// KEY_MULTIMEDIA_1 - report[1] = 0x01
// KEY_LAUNCH_CALCULATOR       -> report[0] = 0x92 (0x0192)
// KEY_LAUNCH_LOCAL_BROWSER    -> report[0] = 0x94 (0x0194)
// KEY_LAUNCH_INTERNET_BROWSER -> report[0] = 0x96 (0x0196)
// KEY_LAUNCH_SCREEN_SAVER     -> report[0] = 0x9E (0x019E)

// EEPROM - 2B/key; 48keys; 8layers; 2*48*8 = 768B
// keymap : 0-767
// tapdance keys : 800-899

typedef struct {
    uint8_t type;
    uint8_t key;
} KeyInfo;

#define MAX_LAYER_COUNT 8
#define LAYER_HISTORY_CAPACITY 5
#define MAX_TAPDANCE_KEYS 16
#define MAX_TAPDANCE_TAPS 3
#define TAPDANCE_EEPROM_OFFSET (sizeof(KeyInfo)*MAX_LAYER_COUNT*KEY_COUNT)

class KeyMap {
    private:
        uint8_t layer_index;
        uint8_t toggled_layer_index;
        uint8_t layer_history[LAYER_HISTORY_CAPACITY];

        inline uint32_t get_eeprom_address(uint8_t layer, uint8_t row, uint8_t col);
        inline uint32_t get_tapdance_eeprom_address(uint8_t index, uint8_t tap);
    public:
        KeyMap(void);

        KeyInfo get_key(uint8_t row, uint8_t col);
        KeyInfo get_key_from_layer(uint8_t layer, uint8_t row, uint8_t col);
        KeyInfo get_non_transparent_key(uint8_t row, uint8_t col);
        KeyInfo get_tapdance_key(uint8_t index, uint8_t tap);

        void set_key(uint8_t layer, uint8_t row, uint8_t col, KeyInfo key);
        void set_tapdance_key(uint8_t index, uint8_t tap, KeyInfo key_info);
        uint8_t get_layer();
        void set_layer(uint8_t layer);
        void remove_layer(uint8_t layer);
        void toggle_layer(uint8_t layer);
        void eeprom_clear_all();
        void eeprom_clear_keymap();
        void eeprom_clear_tapdance();
        void clear();

        inline static uint8_t is_type_between(KeyInfo key_info, uint8_t type1, uint8_t type2);
        inline static uint8_t is_key_with_mod(KeyInfo key_info);
        inline static uint8_t is_dual_key(KeyInfo key_info);
        inline static uint8_t is_dual_layer_key(KeyInfo key_info);
        inline static uint8_t is_multimedia_key(KeyInfo key_info);
        inline static int key_info_compare(KeyInfo key_info1, KeyInfo key_info2);
        inline static uint8_t get_dual_key_modifier(KeyInfo key_info);
        inline static uint8_t get_dual_layer_key_layer(KeyInfo key_info);
        inline static uint8_t get_key_with_mod_modifier(KeyInfo key_info);

        // For Serial.print() usage only
        inline static const __FlashStringHelper* key_type_to_string(KeyInfo key_info);
};

#endif

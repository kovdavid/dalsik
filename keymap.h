#ifndef KEYMAP_h
#define KEYMAP_h

#include "dalsik.h"

#define KEY_COUNT ROW_PIN_COUNT * COL_PIN_COUNT

#define KEY_UNSET    0x00
#define KEY_NORMAL   0x01
#define KEY_LAYER    0x02
#define KEY_DUAL     0x03
#define KEY_TAPDANCE 0x04

typedef struct {
    uint8_t key;
    uint8_t _reserved;
} KeyNormal;

typedef struct {
    uint8_t layer;
    uint8_t _reserved;
} KeyLayer;

typedef struct {
    uint8_t tap_key;
    uint8_t hold_key;
} KeyDual;

typedef struct {
    uint8_t tap_once_key;
    uint8_t tap_twice_key;
} KeyTapDance;

typedef struct {
    uint8_t byte1;
    uint8_t byte2;
} KeyGenericBytes;

// EEPROM - 3B/key; 48keys; 6layers; 3*48*6=864B
// EEPROM for persistence only; it is kept in memory at runtime?

typedef struct {
    uint8_t type;
    union {
        KeyNormal normal;
        KeyLayer layer;
        KeyDual dual;
        KeyTapDance tap_dance;
        KeyGenericBytes generic_bytes;
    };
} KeyInfo;

class KeyMap
{
    private:
        uint8_t eeprom_offset;
        uint8_t layer_index;

        int get_eeprom_address(uint8_t layer, uint8_t row, uint8_t col);
    public:
        KeyMap(void);
        KeyInfo get_key(uint8_t row, uint8_t col);
        KeyInfo get_key_from_layer(uint8_t layer, uint8_t row, uint8_t col);
        void set_layer(uint8_t layer);
        void clear();
        void set_key(uint8_t layer, uint8_t row, uint8_t col, KeyInfo key);
};

#endif

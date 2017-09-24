#ifndef KEYMAP_h
#define KEYMAP_h

#include "dalsik.h"

#define KEY_COUNT ROW_PIN_COUNT * COL_PIN_COUNT

typedef enum:uint8_t {
    NORMAL_KEY   = 0x01,
    LAYER_KEY    = 0x02,
    DUAL_KEY     = 0x03,
    TAPDANCE_KEY = 0x04,
} keymap_type;

// NORMAL_KEY
//  keymap_def   { type = NORMAL_KEY, key = 0x02 }
//  layers       { type = LAYER_KEY, layer = LAYER1 }
//  dual_key     { type = DUAL_KEY, tap_key = 0x02, hold_key = 0x03 }
//  tapdance_key { type = TAPDANCE_KEY, once_key = 0x02, twice_key = 0x03 }

// EEPROM - 3B/key; 48keys; 6layers; 3*48*6=864B
// EEPROM for persistence only; it is kept in memory at runtime?

typedef struct {
    keymap_type type;
} keymap_def;

class Keymap
{
    private:
        uint8_t eeprom_offset;
    public:
        Keymap(void);
};

#endif

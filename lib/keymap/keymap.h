#pragma once

#include "dalsik_global.h"
#include "key_info.h"
#include "key_definitions.h"
#include <avr/pgmspace.h>

extern const uint32_t keymap[][KEYBOARD_ROWS][KEYBOARD_COLS] PROGMEM;
extern const uint8_t layer_count;

namespace KeyMap {
    inline KeyInfo get_key(uint8_t layer, KeyCoords c) {
        if (layer < layer_count) {
            uint32_t progmem_data = pgm_read_dword(&keymap[layer][c.row][c.col]);
            return KeyInfo(progmem_data, c);
        }

        return KeyInfo(KEY_NO_ACTION, c);
    }

    inline uint8_t get_layer_count() {
        return layer_count;
    }
}

#define LAYOUT_4x12( \
    k00, k01, k02, k03, k04, k05, k06, k07, k08, k09, k0a, k0b, \
    k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k1a, k1b, \
    k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k2a, k2b, \
    k30, k31, k32, k33, k34, k35, k36, k37, k38, k39, k3a, k3b  \
) \
{ \
    { k00, k01, k02, k03, k04, k05, k06, k07, k08, k09, k0a, k0b }, \
    { k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k1a, k1b }, \
    { k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k2a, k2b }, \
    { k30, k31, k32, k33, k34, k35, k36, k37, k38, k39, k3a, k3b } \
}

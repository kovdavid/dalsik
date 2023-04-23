#pragma once

#include <avr/pgmspace.h>

#include "dalsik_global.h"
#include "key_info.h"

extern const uint32_t keymap[][KEYBOARD_ROWS][KEYBOARD_COLS] PROGMEM;
extern const uint8_t layer_count;

#define KEYMAP(...) __VA_ARGS__; const uint8_t layer_count = sizeof(keymap) / sizeof(keymap[0]);

class KeyMap {
    private:
        uint8_t layer_index;
        uint8_t toggled_layer_index;
        uint8_t layer_history[LAYER_HISTORY_CAPACITY];

        KeyInfo get_key_from_layer(uint8_t layer, KeyCoords c);

    public:
        KeyMap();
        KeyInfo get_key(KeyCoords c);
        KeyInfo get_non_transparent_key(KeyCoords c);
        void set_layer(uint8_t layer);
        void remove_layer(uint8_t layer);
        void toggle_layer(uint8_t layer);
        void print_internal_state();
};

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

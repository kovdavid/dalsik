#pragma once

#include "dalsik.h"
#include "key_definitions.h"

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

        KeyInfo get_non_transparent_key(KeyCoords c);
        KeyInfo get_key(KeyCoords c);
    public:
        KeyMap(void);

        KeyInfo get_master_key(KeyCoords coords);
        KeyInfo get_slave_key(KeyCoords coords);
        void reload_by_coords(KeyInfo* ki);

        void set_layer(uint8_t layer);
        void remove_layer(uint8_t layer);
        void toggle_layer(uint8_t layer);
        void clear();
};

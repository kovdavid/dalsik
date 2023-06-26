#pragma once

#include "Arduino.h"
#include "dalsik.h"

class Key {
    public:
        uint8_t type;
        uint8_t layer;
        uint8_t modifiers;
        uint8_t code;
        KeyCoords coords;

        Key(void);
        Key(KeyCoords c);
        Key(uint32_t progmem_data, KeyCoords c);
        Key(uint8_t type, uint8_t layer, uint8_t mod, uint8_t code, KeyCoords c);

        Key clone_and_keep_layer();
        Key clone_and_keep_modifiers();
        Key clone_and_keep_code();

        void print_internal_state();

        bool is_any_dual_modifiers_key();
        bool is_any_dual_layer_key();
        bool is_any_dual_key();

        bool equals(Key other);
};

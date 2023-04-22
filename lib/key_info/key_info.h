#pragma once

#include "Arduino.h"
#include "key_definitions.h"
#include "dalsik_global.h"

class KeyInfo {
    public:
        uint8_t type;
        uint8_t layer;
        uint8_t mod;
        uint8_t key;
        KeyCoords coords;

        KeyInfo(void);
        KeyInfo(KeyCoords c);
        KeyInfo(uint32_t progmem_data, KeyCoords c);
        KeyInfo(uint8_t type, uint8_t layer, uint8_t mod, uint8_t key, KeyCoords c);

        KeyInfo use_mod();
        KeyInfo use_key();

        void print_internal_state();

        bool equals(KeyInfo other);
        bool skip_layer_reload();
        bool is_any_dual_mod_key();
        bool is_any_timed_dual_key();
        bool is_any_dual_layer_key();
        bool is_any_dual_key();
        bool is_any_solo_dual_key();
        bool is_multimedia_key();
};

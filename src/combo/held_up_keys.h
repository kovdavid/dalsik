#pragma once

#include "dalsik.h"
#include "held_up_key.h"

#define HELD_UP_KEYS_SIZE 10

class HeldUpKeys {
    public:
        uint8_t count;
        HeldUpKey keys[HELD_UP_KEYS_SIZE];

        HeldUpKeys();
        HeldUpKey* add(KeyCoords coords, millisec timestamp);
        HeldUpKey* find_by_coords(KeyCoords coords);
        HeldUpKey* get_by_index(uint8_t index);
        void remove(HeldUpKey* hkey);
        void repack_keys();

        void print_internal_state(millisec now);
};

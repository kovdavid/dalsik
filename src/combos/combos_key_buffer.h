#pragma once

#include "dalsik.h"
#include "combos_buffered_key.h"

#define COMBOS_KEY_BUFFER_SIZE 10

class CombosKeyBuffer {
    public:
        uint8_t count;
        CombosBufferedKey keys[COMBOS_KEY_BUFFER_SIZE];

        CombosKeyBuffer();
        CombosBufferedKey* add(KeyCoords coords, millisec now);
        CombosBufferedKey* find(KeyCoords coords);
        void normalize();

        void print_internal_state(millisec now);

        CombosBufferedKey* get(uint8_t index);
};

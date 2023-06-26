#pragma once

#include "dalsik.h"

class HeldUpKey {
    public:
        KeyCoords coords;
        millisec timestamp;
        bool part_of_active_combo;
        uint8_t active_combo_index;
        uint8_t key_index;

        HeldUpKey();

        void activate(uint8_t combo_index);
        void clear();
};

#pragma once

#include "dalsik.h"

class CombosBufferedKey {
    public:
        KeyCoords coords;
        millisec timestamp;
        bool part_of_active_combo;
        uint8_t active_combo_index;

        CombosBufferedKey();

        void activate(uint8_t combo_index);
        void clear();
        void set_coords(KeyCoords coords, millisec now);
};

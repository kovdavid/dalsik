#pragma once

#include "dalsik.h"

#define FLAG_DISABLED      1 << 0
#define FLAG_ACTIVATED     1 << 1

class ComboState {
    private:
        KeyCoords read_coords_at(uint8_t index);

    public:
        const KeyCoords *coords;
        uint32_t target_key;
        uint8_t coords_count;
        millisec timestamp;
        uint8_t state = 0x00;
        uint8_t flags = 0x00;

        void print_internal_state(uint8_t index);

        ComboState(const KeyCoords* coords, uint32_t target_key, uint8_t coords_count);
        bool update_state_if_affected(KeyCoords coords, millisec now);
        void remove_coords_from_state(KeyCoords coords);

        bool is_activated();
        bool is_disabled();
        void set_activated();
        void set_disabled();
        void clear_state_keep_flags();
        void clear_disabled();
        void clear_state_and_flags();
        bool is_fully_pressed();
        bool is_fully_released();
};

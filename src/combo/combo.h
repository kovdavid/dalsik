#pragma once

#include "dalsik.h"
#include "matrix.h"

#define FLAG_DISABLED      1 << 0
#define FLAG_ACTIVATED     1 << 1

class Combo {
    private:
        KeyCoords read_coords_at(uint8_t index);

    public:
        const KeyCoords *coords;
        const uint32_t target_key;
        const uint8_t coords_count;
        uint8_t pressed_coords;
        uint8_t flags;

        Combo(const KeyCoords* coords, const uint32_t target_key, const uint8_t coords_count);

        bool update_state_if_affected(KeyCoords coords);
        void release_coords(KeyCoords coords);

        void print_internal_state(uint8_t index);

        bool is_fully_pressed();
        bool is_fully_released();
        bool is_activated();
        bool is_disabled();
        void set_activated_flag();
        void set_disabled_flag();
        void clear_state();
        void clear_disabled_flag();
};

extern Combo combos[];
extern const uint8_t combos_count;

#define COMBO(coords, target_key) Combo(coords, target_key, sizeof(coords)/sizeof(coords[0]))
#define COMBOS(...) __VA_ARGS__; const uint8_t combos_count = sizeof(combos)/sizeof(combos[0]);
#define COMBO_TARGET_KEY_COORDS(index) KeyCoords { COORD_COMBO, (uint8_t)index }
#define COMBO_AT(index) (&(combos[index]))

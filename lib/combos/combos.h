#pragma once

#include "dalsik.h"
#include "combo_state.h"

#ifdef COMBOS_ENABLED

extern ComboState combos[];
extern const uint8_t combos_count;

#define COMBOS_COUNT combos_count
#define COMBO_STATE(i) &(combos[i])

#else

#define COMBOS_COUNT 0
#define COMBO_STATE(i) NULL

#endif

#define COMBOS(...) __VA_ARGS__; const uint8_t combos_count = sizeof(combos) / sizeof(combos[0]);
#define COMBO(combo, key) ComboState(combo, key, sizeof(combo)/sizeof(KeyCoords))
#define COMBO_TARGET_KEY_COORDS(index) KeyCoords { COORD_COMBO, (uint8_t)index }

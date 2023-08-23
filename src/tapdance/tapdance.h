#pragma once

#include "keymap.h"
#include "key.h"

class TapDance {
    public:
        const uint32_t *target_keys;
        const uint8_t target_keys_size;

        TapDance(const uint32_t* target_keys, const uint8_t target_keys_size);

        Key get_key_for_tap(uint8_t tap_count, KeyCoords c);
};

extern const TapDance tapdances[];
extern const uint8_t tapdances_count;

#define TAPDANCE(keys) TapDance(keys, sizeof(keys)/sizeof(keys[0]))
#define TAPDANCES(...) __VA_ARGS__; const uint8_t tapdances_count = sizeof(tapdances) / sizeof(tapdances[0]);

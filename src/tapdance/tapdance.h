#pragma once

#include "keymap.h"
#include "key_info.h"

#define TAPDANCE(keys) TapDance(keys, sizeof(keys)/sizeof(keys[0]))

class TapDance {
    public:
        const uint32_t *target_keys;
        uint8_t target_keys_size;

        TapDance(const uint32_t* target_keys, uint8_t target_keys_size) {
            this->target_keys = target_keys;
            this->target_keys_size = target_keys_size;
        }

        KeyInfo get_key_for_tap(uint8_t tap_count, KeyCoords c) {
            if (tap_count == 0 || tap_count > this->target_keys_size) {
                return KeyInfo(c);
            } else {
                uint32_t progmem_data = pgm_read_dword(&this->target_keys[tap_count - 1]);
                return KeyInfo(progmem_data, c);
            }
        }
};

#ifdef TAPDANCE_ENABLED

extern TapDance tapdances[];

#define TAPDANCE_INDEX(i) &(tapdances[i])

#else

#define TAPDANCE_INDEX(i) NULL

#endif

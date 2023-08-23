#include "tapdance.h"
#include "matrix.h"
#include "key.h"
#include "dalsik.h"

#ifndef TAPDANCE_ENABLED
const TapDance tapdances[] = { TapDance(NULL, 0) };
const uint8_t tapdances_count = 0;
#endif

TapDance::TapDance(const uint32_t* target_keys, const uint8_t target_keys_size)
    : target_keys(target_keys)
    , target_keys_size(target_keys_size)
{}

Key TapDance::get_key_for_tap(uint8_t tap_count, KeyCoords c) {
    if (tap_count == 0 || tap_count > this->target_keys_size) {
        return Key(c);
    } else {
        uint32_t progmem_data = pgm_read_dword(&this->target_keys[tap_count - 1]);
        return Key(progmem_data, c);
    }
}

#pragma once

#include "dalsik.h"
#include "key_event.h"

typedef struct {
    // When did we start processing a TapDance sequence
    millisec pending_tapdance_start;
    // The number of taps for the pending TapDance sequence
    uint8_t tap_count;
    // The last press/release (not timeout) TapDance key key_event
    // Based on this value we can say if the TapDance key is pressed or released
    KeyEvent key_event;
} TapDanceHandlerInternalState;

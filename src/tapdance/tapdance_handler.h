#pragma once

#include "dalsik.h"
#include "extended_key_event.h"
#include "keymap.h"
#include "keyboard.h"

typedef struct {
    // When did we start processing a TapDance sequence
    millisec pending_tapdance_start;
    // The number of taps for the pending TapDance sequence
    uint8_t pending_tap_count;
    // The last press/release (not timeout) TapDance key key_event
    // Based on this value we can say if the TapDance key is pressed or released
    ExtendedKeyEvent last_key_event;
} TapDanceHandlerState;

class TapDanceHandler {
    TEST_FRIEND_CLASS
    private:
        Keyboard *next_event_handler;
        KeyMap *keymap;

        TapDanceHandlerState state;

        uint8_t handle_key_press(ExtendedKeyEvent e);
        uint8_t handle_key_release(ExtendedKeyEvent e);
        uint8_t handle_timeout(ExtendedKeyEvent e);
        uint8_t resume_pending_tapdance(ExtendedKeyEvent e);
        void trigger_tapdance(millisec now);

    public:
        TapDanceHandler(Keyboard *next_event_handler, KeyMap *keymap);

        void handle_key_event(ExtendedKeyEvent e);
        void print_internal_state(millisec now);
};

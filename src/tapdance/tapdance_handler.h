#pragma once

#include "dalsik.h"
#include "key_event.h"
#include "keymap.h"
#include "keyboard.h"
#include "tapdance_handler_internal_state.h"

class TapDanceHandler {
#ifdef TEST_TAPDANCE_HANDLER_FRIENDS
    TEST_TAPDANCE_HANDLER_FRIENDS
#endif
    private:
        Keyboard *next_event_handler;
        KeyMap *keymap;

        TapDanceHandlerInternalState state;

        uint8_t handle_key_press(KeyEvent e);
        uint8_t handle_key_release(KeyEvent e);
        uint8_t handle_timeout(KeyEvent e);
        void trigger_tapdance(millisec now);

    public:
        TapDanceHandler(Keyboard *next_event_handler, KeyMap *keymap);

        void handle_key_event(KeyEvent e);
        void print_internal_state(millisec now);
};

#pragma once

#include "combos_handler.h"
#include "dalsik.h"
#include "keyboard.h"
#include "keymap.h"
#include "tapdance_handler.h"

class KeyEventHandler {
#ifdef TEST_KEY_EVENT_HANDLER_FRIENDS
    TEST_KEY_EVENT_HANDLER_FRIENDS
#endif
    private:
        KeyMap keymap;
        Keyboard keyboard;
        TapDanceHandler tapdance_handler;
        CombosHandler combos_handler;
        uint8_t keyboard_side;

        uint8_t encode_slave_event(BasicKeyEvent event);
        BasicKeyEvent decode_slave_event(uint8_t data);
    public:
        KeyEventHandler();

        void send_slave_event_to_master(BasicKeyEvent event);
        void handle_key_event_from_slave(uint8_t data, millisec now);
        void handle_key_event_from_master(BasicKeyEvent event, millisec now);
        void handle_timeout(millisec now);
        void handle_key_event(BasicKeyEvent event, millisec now);

        void print_internal_state(millisec now);
};

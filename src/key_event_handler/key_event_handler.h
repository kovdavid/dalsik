#pragma once

#include "dalsik.h"
#include "combo_handler.h"
#include "keyboard.h"
#include "keymap.h"
#include "tapdance_handler.h"

class KeyEventHandler {
    TEST_FRIEND_CLASS
    private:
        KeyMap keymap;
        Keyboard keyboard;
        TapDanceHandler tapdance_handler;
        ComboHandler combo_handler;
        char keyboard_side;

        uint8_t encode_slave_event(BaseKeyEvent event);
        BaseKeyEvent decode_slave_event(uint8_t data);
    public:
        KeyEventHandler();

        void send_slave_event_to_master(BaseKeyEvent event);
        void handle_received_data_from_slave(uint8_t data, millisec now);
        void handle_key_event_from_master(BaseKeyEvent event, millisec now);
        void handle_timeout(millisec now);
        void handle_key_event(BaseKeyEvent event, millisec now);

        void print_internal_state(millisec now);
};

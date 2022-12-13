#pragma once

#include "combos_handler.h"
#include "keyboard.h"
#include "dalsik_global.h"

class KeyEventHandler {
    private:
        Keyboard keyboard;
        CombosHandler combos_handler;
        uint8_t keyboard_side;

        uint8_t encode_slave_event(ChangedKeyEvent event);
        ChangedKeyEvent decode_slave_event(uint8_t data);

        void process_slave_changed_key(ChangedKeyEvent event, millisec now);
        void process_master_changed_key(ChangedKeyEvent event, millisec now);
        inline void handle_key_event(ChangedKeyEvent event, millisec now);
    public:
        KeyEventHandler();

        void send_slave_event_to_master(ChangedKeyEvent event);
        void handle_received_data_from_slave(uint8_t data, millisec now);
        void handle_key_event_from_master(ChangedKeyEvent event, millisec now);
        void handle_timeout(millisec now);
};

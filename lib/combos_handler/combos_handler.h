#pragma once

#include "dalsik.h"
#include "keyboard.h"
#include "combos_key_buffer.h"
#include "combos_buffered_key.h"

#ifndef TEST_COMBOS_HANDLER_FRIENDS
#define TEST_COMBOS_HANDLER_FRIENDS
#endif

class CombosHandler {
    TEST_COMBOS_HANDLER_FRIENDS
    private:
        Keyboard* keyboard;
        CombosKeyBuffer key_buffer;

        // When did we start processing a pending combo
        millisec pending_combos_start;
        // The last event that was sent to the keyboard
        millisec last_passthrough_event;

        bool start_pending_combo_processing(ChangedKeyEvent e, millisec now);
        bool resume_pending_combo_processing(ChangedKeyEvent e, millisec now);
        bool resume_pending_combo_processing_press(ChangedKeyEvent e, millisec now);
        bool resume_pending_combo_processing_release(ChangedKeyEvent e, millisec now);
        bool resume_pending_combo_processing_timeout(millisec now);

        void abort_pending_combos_processing();
        void activate_combo(int8_t index);
        void release_active_combo_key(CombosBufferedKey* cbk, millisec now);
    public:
        CombosHandler(Keyboard* keyboard);

        bool handle_key_event(ChangedKeyEvent e, millisec now);
        void print_internal_state(millisec now);
};

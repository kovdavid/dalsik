#pragma once

#include "combos_buffered_key.h"
#include "combos_key_buffer.h"
#include "dalsik.h"
#include "key_event.h"
#include "tapdance_handler.h"

class CombosHandler {
    TEST_FRIEND_CLASS
    private:
        TapDanceHandler *next_event_handler;
        CombosKeyBuffer key_buffer;

        // When did we start processing a pending combo
        millisec pending_combos_start;
        // The last event that was sent to next_event_handler
        millisec last_passthrough_event;

        uint8_t start_pending_combo_processing(KeyEvent e);
        uint8_t resume_pending_combo_processing(KeyEvent e);
        uint8_t resume_pending_combo_processing_press(KeyEvent e);
        uint8_t resume_pending_combo_processing_release(KeyEvent e);
        uint8_t resume_pending_combo_processing_timeout(KeyEvent e);

        void abort_pending_combos_processing();
        void activate_combo(int8_t index);
        void release_active_combo_key(CombosBufferedKey* cbk, millisec now);
    public:
        CombosHandler(TapDanceHandler *next_event_handler);

        uint8_t handle_key_event(KeyEvent e);
        void print_internal_state(millisec now);
};

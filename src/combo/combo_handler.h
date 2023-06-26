#pragma once

#include "held_up_keys.h"
#include "held_up_key.h"
#include "dalsik.h"
#include "extended_key_event.h"
#include "tapdance_handler.h"

typedef struct {
    // When did we start processing a pending combo
    millisec pending_combo_start;
    // The last event that was sent to next_event_handler without processing here
    millisec last_non_combo_press;
    // The keys held up by the pending Combo processing
    HeldUpKeys held_up_keys;
} ComboHandlerState;

class PressScanResult {
    public:
        uint8_t affected_combos_count;
        uint8_t disabled_combos_count;
        int8_t fully_pressed_combo_index;

        PressScanResult()
            : affected_combos_count(0)
            , disabled_combos_count(0)
            , fully_pressed_combo_index(-1)
        {}
};

class ComboHandler {
    TEST_FRIEND_CLASS
    private:
        TapDanceHandler *next_event_handler;
        ComboHandlerState state;

        PressScanResult scan_combos_on_press_event(ExtendedKeyEvent event);
        int8_t find_fully_pressed_pending_combo();

        uint8_t resume_pending_combo_processing(ExtendedKeyEvent event);
        uint8_t resume_pending_combo_processing_press(ExtendedKeyEvent event);
        uint8_t resume_pending_combo_processing_release(ExtendedKeyEvent event);
        uint8_t resume_pending_combo_processing_timeout(ExtendedKeyEvent event);

        uint8_t no_pending_combo_processing(ExtendedKeyEvent event);
        uint8_t no_pending_combo_processing_release(ExtendedKeyEvent event);
        uint8_t no_pending_combo_processing_press(ExtendedKeyEvent event);

        void activate_combo(int8_t index);
        void abort_pending_combos_processing();
        void release_active_combo_key(HeldUpKey* hkey, millisec timestamp);

    public:
        ComboHandler(TapDanceHandler *next_event_handler);

        uint8_t handle_key_event(ExtendedKeyEvent e);
        void print_internal_state(millisec now);
};

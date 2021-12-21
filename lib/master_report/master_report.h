#pragma once

#include "dalsik.h"
#include "matrix.h"
#include "keymap.h"
#include "dalsik_hid.h"

class MasterReport {
    private:
        void print_base_report_to_serial();
        void print_system_report_to_serial();
        void print_multimedia_report_to_serial();

        inline void handle_key_press(KeyInfo key_info);
        inline void handle_key_release(KeyInfo key_info);
        void press(KeyInfo key_info);
        void release(KeyInfo key_info);
        void clear();

        void send_hid_report();
        void send_base_hid_report();
        void send_system_hid_report();
        void send_multimedia_hid_report();

        inline void press_hook_for_dual_keys(KeyInfo* ki);
        inline void press_hook_for_layer_toggle_or_hold(KeyInfo* i);
        inline void press_hook_for_tapdance_keys(KeyInfo key_info);
        inline void release_all_keys_hook_for_tapdance_keys();
        inline void activate_tapdance(uint8_t index);
        inline void dual_key_timeout_check(DualKeyState* state);

        inline void press_normal_key(KeyInfo key_info);
        inline void release_normal_key(KeyInfo key_info);

        inline void press_layer_key(uint8_t layer);
        inline void release_layer_key(uint8_t layer);
        inline void press_toggle_layer_key(uint8_t layer);

        inline void press_system_key(KeyInfo key_info);
        inline void release_system_key(KeyInfo key_info);

        inline void press_single_dual_key(KeyInfo key_info);
        inline void press_single_dual_layer_key(KeyInfo key_info);

        inline void press_dual_key(KeyInfo key_info);
        inline void release_dual_key(KeyInfo key_info);

        inline void press_dual_layer_key(KeyInfo key_info);
        inline void release_dual_layer_key(KeyInfo key_info);

        inline void press_multimedia_key(KeyInfo key_info);
        inline void release_multimedia_key(KeyInfo key_info);

        inline void press_key_with_mod(KeyInfo key_info);
        inline void release_key_with_mod(KeyInfo key_info);

        inline void press_layer_toggle_or_hold(KeyInfo key_info);
        inline void release_layer_toggle_or_hold(KeyInfo key_info);

        inline void press_tapdance_key(KeyInfo key_info);
        inline void release_tapdance_key(KeyInfo key_info);

        BaseHIDReport base_hid_report;
        SystemHIDReport system_hid_report;
        MultimediaHIDReport multimedia_hid_report;

        uint8_t base_hid_report_changed;
        uint8_t system_hid_report_changed;
        uint8_t multimedia_hid_report_changed;

        uint8_t num_keys_pressed;
        uint8_t base_keys_pressed;
        uint8_t system_keys_pressed;
        uint8_t multimedia_keys_pressed;

        KeyMap* keymap;
        DualKeyState dual_key_state;
        DualKeyState dual_layer_key_state;
        LayerToggleOrHoldState toggle_or_hold_state;
        TapDanceState tapdance_state[MAX_TAPDANCE_KEYS];

        uint8_t active_tapdance_key_count;
        uint8_t last_tapdance_index;
        millisec last_tapdance_press_ts;
    public:
        MasterReport(KeyMap* keymap);

        void key_timeout_check();
        void handle_master_changed_key(ChangedKeyEvent event);
        void handle_slave_changed_key(ChangedKeyEvent event);
};

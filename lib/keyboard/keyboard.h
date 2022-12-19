#pragma once

#include "matrix.h"
#include "dalsik.h"
#include "dalsik_hid.h"
#include "dalsik_global.h"
#include "key_info.h"
#include "pressed_keys.h"

#ifndef TEST_KEYBOARD_FRIENDS
#define TEST_KEYBOARD_FRIENDS
#endif

#ifdef LED_PIN
const uint32_t LED_LAYER_COLORS[MAX_LAYER_COUNT] = {
    0x00000000, // Layer 0 - default (LED off)
    0x00FF0000,
    0x0000FF00,
    0x000000FF,
    0x00FFFF00,
    0x00FF00FF,
    0x00000000,
    0x00000000,
};
#endif

class Keyboard {
    TEST_KEYBOARD_FRIENDS
    private:
        // State
        uint8_t layer_index;
        uint8_t toggled_layer_index;
        uint8_t layer_history[LAYER_HISTORY_CAPACITY];

        uint8_t one_shot_modifiers;
        HIDReports current_hid_reports;
        HIDReports last_hid_reports;

        // Does not matter, if this overflows. We need only equality check
        // This is mostly needed for timeout handlers to detect if there
        // was any keys pressed between the key we are checking timeout for
        // and `now`
        uint8_t key_press_counter;

        PressedKeys pressed_keys;

        // Functions

        void set_layer(uint8_t layer);
        void remove_layer(uint8_t layer);
        void toggle_layer(uint8_t layer);

        KeyInfo get_non_transparent_key(KeyCoords c);
        KeyInfo get_key(KeyCoords c);
        void reload_keys_on_new_layer(uint8_t key_index);

        void press(PressedKey *pk);
        void release(PressedKey *pk, millisec now);
        inline void run_press_hooks(uint8_t event_key_index);
        inline void run_press_hook(uint8_t key_index);

        inline void press_normal_key(KeyInfo key_info);
        inline void release_normal_key(KeyInfo key_info);

        inline void press_one_shot_modifier_key(PressedKey *pk);
        inline void release_one_shot_modifier_key(PressedKey *pk, millisec now);

        inline void press_layer_key(uint8_t layer);
        inline void release_layer_key(uint8_t layer);

        inline void press_toggle_layer_key(uint8_t layer);

        inline void press_system_key(KeyInfo key_info);
        inline void release_system_key(KeyInfo key_info);

        inline void press_multimedia_key(KeyInfo key_info);
        inline void release_multimedia_key(KeyInfo key_info);

        inline void press_dual_mod_key(PressedKey *pk);
        inline void release_dual_mod_key(PressedKey *pk);

        inline void press_dual_layer_key(PressedKey *pk);
        inline void release_dual_layer_key(PressedKey *pk);

        inline void press_key_with_mod(KeyInfo key_info);
        inline void release_key_with_mod(KeyInfo key_info);

        inline void press_layer_toggle_or_hold(PressedKey *pk);
        inline void release_layer_toggle_or_hold(PressedKey *pk);

        inline void send_hid_report();

        inline PressedKey* add_to_pressed_keys(KeyInfo key_info, millisec now);
        inline PressedKey* find_in_pressed_keys(KeyCoords coords);
        inline void remove_from_pressed_keys(PressedKey *pk);

        void print_base_report_to_serial();
        void print_system_report_to_serial();
        void print_multimedia_report_to_serial();

    public:
        Keyboard(void);

        void handle_key_event(ChangedKeyEvent e, millisec now);

        void handle_key_press(KeyInfo key_info, millisec now);
        void handle_key_release(KeyCoords coords, millisec now);
        void handle_timeout(millisec now);

        void print_internal_state(millisec now);
};

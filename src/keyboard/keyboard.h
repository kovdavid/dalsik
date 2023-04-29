#pragma once

#include "dalsik.h"
#include "keymap.h"
#include "key_event.h"
#include "key_info.h"
#include "pressed_keys.h"

class Keyboard {
#ifdef TEST_KEYBOARD_FRIENDS
    TEST_KEYBOARD_FRIENDS
#endif
    private:
        // State
        uint8_t one_shot_modifiers;
        bool caps_word_enabled;
        bool caps_word_apply_modifier;
        HIDReports current_hid_reports;
        HIDReports last_hid_reports;
        KeyMap *keymap;

        // Does not matter, if this overflows. We need only equality check
        // This is mostly needed for timeout handlers to detect if there
        // was any keys pressed between the key we are checking timeout for
        // and `now`
        uint8_t key_press_counter;

        PressedKeys pressed_keys;
        KeyEvent last_press_key_event;

        // Functions

        void handle_key_press(KeyEvent event);
        void handle_key_release(KeyEvent event);
        void handle_timeout(KeyEvent event);

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

        inline void press_mouse_button(PressedKey *pk);
        inline void release_mouse_button(PressedKey *pk);

        inline void press_toggle_caps_word();

        inline void press_dual_dth_key(PressedKey *pk);

        inline void caps_word_toggle();
        inline void caps_word_turn_off();
        inline void caps_word_check(KeyInfo key_info);

        inline void send_hid_report();

        inline PressedKey* add_to_pressed_keys(KeyInfo key_info, millisec now);
        inline PressedKey* find_in_pressed_keys(KeyCoords coords);
        inline void remove_from_pressed_keys(PressedKey *pk);

        void print_base_report_to_serial();
        void print_system_report_to_serial();
        void print_multimedia_report_to_serial();

    public:
        Keyboard(KeyMap *keymap);

        void handle_key_event(KeyEvent e);

        void print_internal_state(millisec now);
};

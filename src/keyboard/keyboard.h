#pragma once

#include "dalsik.h"
#include "keymap.h"
#include "extended_key_event.h"
#include "key.h"
#include "pressed_keys.h"

class Keyboard {
    TEST_FRIEND_CLASS
    private:
        // State

        // Does not matter, if this overflows. We need only equality check
        // This is mostly needed for timeout handlers to detect if there
        // was any keys pressed between the key we are checking timeout for
        // and `now`
        uint8_t key_press_counter;

        uint8_t one_shot_modifiers;
        bool caps_word_enabled;
        bool caps_word_apply_modifier;
        HIDReports current_reports;
        HIDReports last_reports;
        PressedKeys pressed_keys;
        ExtendedKeyEvent last_press_key_event;
        KeyMap *keymap;

        // Functions

        void handle_key_press(ExtendedKeyEvent event);
        void handle_key_release(ExtendedKeyEvent event);
        void handle_timeout(ExtendedKeyEvent event);

        void press(PressedKey *pk);
        void release(PressedKey *pk, millisec now);

        void press_basic_key(Key key);
        void release_basic_key(Key key);

        void press_one_shot_modifier_key(PressedKey *pk);
        void release_one_shot_modifier_key(PressedKey *pk, millisec now);

        void press_layer_hold_key(Key key);
        void release_layer_hold_key(Key key);

        void press_layer_toggle_key(Key key);

        void press_desktop_key(Key key);
        void release_desktop_key(Key key);

        void press_consumer_key(Key key);
        void release_consumer_key(Key key);

        void press_dual_key(PressedKey *pk);
        void release_dual_key(PressedKey *pk);

        void press_solo_dual_key(PressedKey *pk);

        void press_tap_hold_dual_key(PressedKey *pk);

        void press_layer_hold_or_toggle_key(PressedKey *pk);
        void release_layer_hold_or_toggle_key(PressedKey *pk);

        void press_mouse_button(PressedKey *pk);
        void release_mouse_button(PressedKey *pk);

        void press_toggle_caps_word_key();

        void caps_word_toggle();
        void caps_word_turn_off();
        void caps_word_check(Key key);

        void send_hid_report();
        void send_keyboard_hid_report();
        void send_desktop_hid_report();
        void send_consumer_hid_report();
        void send_mouse_hid_report();

    public:
        Keyboard(KeyMap *keymap);

        void handle_key_event(ExtendedKeyEvent e);
        void run_press_hooks();

        void print_internal_state(millisec now);
};

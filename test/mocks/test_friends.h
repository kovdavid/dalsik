#pragma once

#define TEST_KEYBOARD_FRIENDS \
    friend PressedKeys get_keyboard_pressed_keys(Keyboard); \
    friend bool get_caps_word_enabled(Keyboard);

#define TEST_COMBOS_HANDLER_FRIENDS \
    friend CombosKeyBuffer get_combos_key_buffer(CombosHandler);

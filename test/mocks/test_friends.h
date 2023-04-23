#pragma once

#define TEST_KEYBOARD_FRIENDS \
    friend PressedKeys* get_keyboard_pressed_keys(Keyboard*); \
    friend bool get_caps_word_enabled(Keyboard*);

#define TEST_COMBOS_HANDLER_FRIENDS \
    friend CombosKeyBuffer* get_combos_key_buffer(CombosHandler*);

#define TEST_KEY_EVENT_HANDLER_FRIENDS \
    friend PressedKeys* get_keyboard_pressed_keys(KeyEventHandler*); \
    friend CombosKeyBuffer* get_combos_key_buffer(KeyEventHandler*); \
    friend bool get_caps_word_enabled(KeyEventHandler*); \
    friend Keyboard* get_keyboard(KeyEventHandler*); \
    friend TapDanceHandlerInternalState* get_tapdance_handler_internal_state(KeyEventHandler*);

#define TEST_TAPDANCE_HANDLER_FRIENDS \
    friend TapDanceHandlerInternalState* get_tapdance_handler_internal_state(TapDanceHandler*);

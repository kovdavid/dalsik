#pragma once

#include "key_event_handler.h"
#include "keyboard.h"
#include "tapdance_handler.h"

class TestFriendClass {
    public:
        PressedKeys* get_pressed_keys(KeyEventHandler* key_event_handler) {
            return &(key_event_handler->keyboard.pressed_keys);
        }

        CombosKeyBuffer* get_combos_key_buffer(KeyEventHandler* key_event_handler) {
            return &(key_event_handler->combos_handler.key_buffer);
        }

        bool get_caps_word_enabled(KeyEventHandler* key_event_handler) {
            return key_event_handler->keyboard.caps_word_enabled;
        }

        TapDanceHandlerInternalState* get_tapdance_state(KeyEventHandler* key_event_handler) {
            return &(key_event_handler->tapdance_handler.state);
        }
};

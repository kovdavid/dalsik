#pragma once

#include "key_event_handler.h"
#include "keyboard.h"
#include "tapdance_handler.h"
#include "combo_handler.h"

class TestFriendClass {
    public:
        PressedKeys* get_pressed_keys(KeyEventHandler* key_event_handler) {
            return &(key_event_handler->keyboard.pressed_keys);
        }

        ComboHandlerState* get_combo_handler_state(KeyEventHandler* key_event_handler) {
            return &(key_event_handler->combo_handler.state);
        }

        bool get_caps_word_enabled(KeyEventHandler* key_event_handler) {
            return key_event_handler->keyboard.caps_word_enabled;
        }

        TapDanceHandlerState* get_tapdance_state(KeyEventHandler* key_event_handler) {
            return &(key_event_handler->tapdance_handler.state);
        }
};

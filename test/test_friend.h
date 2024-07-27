#pragma once

#include "key_event_handler.h"
#include "keymap.h"
#include "keyboard.h"
#include "tapdance_handler.h"
#include "combo_handler.h"
#include "mouse.h"

class TestFriendClass {
    public:
        PressedKeys* get_pressed_keys(KeyEventHandler* key_event_handler) {
            return &(key_event_handler->keyboard.pressed_keys);
        }

        MouseCursorState* get_mouse_cursor_state(KeyEventHandler* key_event_handler) {
            return &(key_event_handler->keyboard.mouse.cursor_state);
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

        uint8_t get_activated_layers_count(KeyMap keymap) {
            return keymap.activated_layers_count;
        }

        uint8_t get_activated_layer_at(KeyMap keymap, uint8_t index) {
            return keymap.activated_layers[index];
        }
};

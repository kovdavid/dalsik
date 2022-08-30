#pragma once

#include "key_definitions.h"

#define COORD_UNKNOWN 255

typedef struct {
    uint8_t row;
    uint8_t col;
} KeyCoords;

class KeyInfo {
    public:
        uint8_t type;
        uint8_t key;
        KeyCoords coords;

        KeyInfo() {
            this->type = KEY_UNSET;
            this->key = KC_NO;
            this->coords = KeyCoords { COORD_UNKNOWN, COORD_UNKNOWN };
        }
        KeyInfo(uint8_t type, uint8_t key) {
            this->type = type;
            this->key = key;
            this->coords = KeyCoords { COORD_UNKNOWN, COORD_UNKNOWN };
        }
        KeyInfo(uint8_t type, uint8_t key, KeyCoords c) {
            this->type = type;
            this->key = key;
            this->coords = c;
        }

        uint8_t has_no_coords() {
            return
                this->coords.row == COORD_UNKNOWN
                && this->coords.col == COORD_UNKNOWN;
        }

        uint8_t is_type_between(uint8_t type1, uint8_t type2) {
            if (this->type >= type1 && this->type <= type2) {
                return 1;
            }
            return 0;
        }
        uint8_t is_key_with_mod() {
            return this->is_type_between(
                KEY_WITH_MOD_LCTRL, KEY_WITH_MOD_RALT
            );
        }
        uint8_t is_dual_mod_key() {
            return this->is_type_between(
                KEY_DUAL_MOD_LCTRL, KEY_DUAL_MOD_RALT
            );
        }
        uint8_t is_solo_dual_mod_key() {
            return this->is_type_between(
                KEY_SOLO_DUAL_MOD_LCTRL, KEY_SOLO_DUAL_MOD_RALT
            );
        }
        uint8_t is_timed_dual_mod_key() {
            return this->is_type_between(
                KEY_TIMED_DUAL_MOD_LCTRL, KEY_TIMED_DUAL_MOD_RALT
            );
        }
        uint8_t is_any_dual_mod_key() {
            return
                this->is_dual_mod_key()
                || this->is_solo_dual_mod_key()
                || this->is_timed_dual_mod_key();
        }
        uint8_t is_dual_layer_key() {
            return this->is_type_between(
                KEY_DUAL_LAYER_1, KEY_DUAL_LAYER_7
            );
        }
        uint8_t is_solo_dual_layer_key() {
            return this->is_type_between(
                KEY_SOLO_DUAL_LAYER_1, KEY_SOLO_DUAL_LAYER_7
            );
        }
        uint8_t is_timed_dual_layer_key() {
            return this->is_type_between(
                KEY_TIMED_DUAL_LAYER_1, KEY_TIMED_DUAL_LAYER_7
            );
        }
        uint8_t is_any_timed_dual_key() {
            return
                this->is_timed_dual_mod_key()
                || this->is_timed_dual_layer_key();
        }
        uint8_t is_any_dual_layer_key() {
            return
                this->is_dual_layer_key()
                || this->is_solo_dual_layer_key()
                || this->is_timed_dual_layer_key();
        }
        uint8_t is_any_dual_key() {
            return
                this->is_any_dual_mod_key()
                || this->is_any_dual_layer_key();
        }
        uint8_t is_any_solo_dual_key() {
            return
                this->is_solo_dual_mod_key()
                || this->is_solo_dual_layer_key();
        }
        uint8_t is_multimedia_key() {
            return this->is_type_between(KEY_MULTIMEDIA_0, KEY_MULTIMEDIA_2);
        }

        uint8_t get_dual_key_modifier() {
            switch (this->type) {
                case KEY_DUAL_MOD_LCTRL:
                case KEY_SOLO_DUAL_MOD_LCTRL:
                case KEY_TIMED_DUAL_MOD_LCTRL:
                    return KC_LCTRL;
                case KEY_DUAL_MOD_LSHIFT:
                case KEY_SOLO_DUAL_MOD_LSHIFT:
                case KEY_TIMED_DUAL_MOD_LSHIFT:
                    return KC_LSHIFT;
                case KEY_DUAL_MOD_LALT:
                case KEY_SOLO_DUAL_MOD_LALT:
                case KEY_TIMED_DUAL_MOD_LALT:
                    return KC_LALT;
                case KEY_DUAL_MOD_LGUI:
                case KEY_SOLO_DUAL_MOD_LGUI:
                case KEY_TIMED_DUAL_MOD_LGUI:
                    return KC_LGUI;
                case KEY_DUAL_MOD_RCTRL:
                case KEY_SOLO_DUAL_MOD_RCTRL:
                case KEY_TIMED_DUAL_MOD_RCTRL:
                    return KC_RCTRL;
                case KEY_DUAL_MOD_RSHIFT:
                case KEY_SOLO_DUAL_MOD_RSHIFT:
                case KEY_TIMED_DUAL_MOD_RSHIFT:
                    return KC_RSHIFT;
                case KEY_DUAL_MOD_RALT:
                case KEY_SOLO_DUAL_MOD_RALT:
                case KEY_TIMED_DUAL_MOD_RALT:
                    return KC_RALT;
                case KEY_DUAL_MOD_RGUI:
                case KEY_SOLO_DUAL_MOD_RGUI:
                case KEY_TIMED_DUAL_MOD_RGUI:
                    return KC_RGUI;
            }
            return 0x00;
        }
        uint8_t get_dual_layer_key_layer() {
            switch (this->type) {
                case KEY_DUAL_LAYER_1:
                case KEY_SOLO_DUAL_LAYER_1:
                case KEY_TIMED_DUAL_LAYER_1:
                    return 1;
                case KEY_DUAL_LAYER_2:
                case KEY_SOLO_DUAL_LAYER_2:
                case KEY_TIMED_DUAL_LAYER_2:
                    return 2;
                case KEY_DUAL_LAYER_3:
                case KEY_SOLO_DUAL_LAYER_3:
                case KEY_TIMED_DUAL_LAYER_3:
                    return 3;
                case KEY_DUAL_LAYER_4:
                case KEY_SOLO_DUAL_LAYER_4:
                case KEY_TIMED_DUAL_LAYER_4:
                    return 4;
                case KEY_DUAL_LAYER_5:
                case KEY_SOLO_DUAL_LAYER_5:
                case KEY_TIMED_DUAL_LAYER_5:
                    return 5;
                case KEY_DUAL_LAYER_6:
                case KEY_SOLO_DUAL_LAYER_6:
                case KEY_TIMED_DUAL_LAYER_6:
                    return 6;
                case KEY_DUAL_LAYER_7:
                case KEY_SOLO_DUAL_LAYER_7:
                case KEY_TIMED_DUAL_LAYER_7:
                    return 7;
            }
            return 0;
        }
        uint8_t get_key_with_mod_modifier() {
            switch (this->type) {
                case KEY_WITH_MOD_LCTRL:  return KC_LCTRL;
                case KEY_WITH_MOD_LSHIFT: return KC_LSHIFT;
                case KEY_WITH_MOD_LALT:   return KC_LALT;
                case KEY_WITH_MOD_LGUI:   return KC_LGUI;
                case KEY_WITH_MOD_RCTRL:  return KC_RCTRL;
                case KEY_WITH_MOD_RSHIFT: return KC_RSHIFT;
                case KEY_WITH_MOD_RALT:   return KC_RALT;
                case KEY_WITH_MOD_RGUI:   return KC_RGUI;
            }
            return 0x00;
        }
};

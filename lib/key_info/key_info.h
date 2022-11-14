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
        uint8_t layer;
        uint8_t mod;
        uint8_t key;
        KeyCoords coords;

        KeyInfo() {
            this->type = KEY_NORMAL;
            this->layer = 0;
            this->mod = MOD_CLEAR;
            this->key = KC_NO;
            this->coords = KeyCoords { COORD_UNKNOWN, COORD_UNKNOWN };
        }
        KeyInfo(uint32_t progmem_data, KeyCoords c) {
            this->type  = (progmem_data >> 24) & 0xFF;
            this->layer = (progmem_data >> 16) & 0xFF;
            this->mod   = (progmem_data >> 8)  & 0xFF;
            this->key   = (progmem_data >> 0)  & 0xFF;
            this->coords = c;
        }
        KeyInfo(uint8_t type, uint8_t layer, uint8_t mod, uint8_t key, KeyCoords c) {
            this->type = type;
            this->layer = layer;
            this->mod = mod;
            this->key = key;
            this->coords = c;
        }

        KeyInfo use_mod() {
            return KeyInfo(this->type, 0, this->mod, KC_NO, this->coords);
        }

        KeyInfo use_key() {
            return KeyInfo(this->type, 0, MOD_CLEAR, this->key, this->coords);
        }

        uint8_t has_no_coords() {
            return
                this->coords.row == COORD_UNKNOWN
                && this->coords.col == COORD_UNKNOWN;
        }

        uint8_t is_any_dual_mod_key() {
            return
                this->type == KEY_DUAL_MOD
                || this->type == KEY_SOLO_DUAL_MOD
                || this->type == KEY_TIMED_DUAL_MOD;
        }
        uint8_t is_any_timed_dual_key() {
            return
                this->type == KEY_TIMED_DUAL_MOD
                || this->type == KEY_TIMED_DUAL_LAYER;
        }
        uint8_t is_any_dual_layer_key() {
            return
                this->type == KEY_DUAL_LAYER
                || this->type == KEY_SOLO_DUAL_LAYER
                || this->type == KEY_TIMED_DUAL_LAYER;
        }
        uint8_t is_any_dual_key() {
            return
                this->is_any_dual_mod_key()
                || this->is_any_dual_layer_key();
        }
        uint8_t is_any_solo_dual_key() {
            return
                this->type == KEY_SOLO_DUAL_MOD
                || this->type == KEY_SOLO_DUAL_LAYER;
        }
        uint8_t is_multimedia_key() {
            return
                this->type == KEY_MULTIMEDIA_0
                || this->type == KEY_MULTIMEDIA_1
                || this->type == KEY_MULTIMEDIA_2;
        }
};

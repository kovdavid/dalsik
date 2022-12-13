#pragma once

#include "Arduino.h"
#include "key_definitions.h"
#include "dalsik_global.h"

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
        KeyInfo(KeyCoords c) {
            this->type = KEY_NORMAL;
            this->layer = 0;
            this->mod = MOD_CLEAR;
            this->key = KC_NO;
            this->coords = c;
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

        void print_internal_state() {
            Serial.print(">> KEY_INFO ROW:");
            Serial.print(this->coords.row);
            Serial.print(" COL:");
            Serial.print(this->coords.col);
            Serial.print(" TYPE:");
            Serial.print(this->type);
            Serial.print(" LAYER:");
            Serial.print(this->layer);
            Serial.print(" MOD:");
            Serial.print(this->mod);
            Serial.print(" KEY:");
            Serial.println(this->key);
        }

        uint8_t skip_layer_reload() {
            return
                this->coords.row == COORD_COMBO
                || this->coords.row == COORD_UNKNOWN
                || this->coords.col == COORD_UNKNOWN;
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

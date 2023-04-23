#pragma once

#include "dalsik_global.h"
#include "key_info.h"

class KeyEvent {
    public:
        uint8_t type;
        KeyCoords coords;
        bool key_looked_up;
        KeyInfo key_info;
        millisec timestamp;

        KeyEvent() {
            this->type = EVENT_NONE;
            this->coords = COORDS_INVALID;
            this->key_looked_up = false;
            this->timestamp = 0;
        }

        KeyEvent(BasicKeyEvent event, millisec timestamp) {
            this->type = event.type;
            this->coords = event.coords;
            this->key_looked_up = false;
            this->timestamp = timestamp;
        }

        KeyEvent(uint8_t type, KeyCoords coords, millisec timestamp) {
            this->type = type;
            this->coords = coords;
            this->key_looked_up = false;
            this->timestamp = timestamp;
        }

        KeyEvent(uint8_t type, KeyCoords coords, KeyInfo key_info, millisec timestamp) {
            this->type = type;
            this->coords = coords;
            this->key_looked_up = true;
            this->key_info = key_info;
            this->timestamp = timestamp;
        }
};

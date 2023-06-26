#pragma once

#include "dalsik.h"
#include "key.h"
#include "keymap.h"

class ExtendedKeyEvent {
    public:
        uint8_t type;
        KeyCoords coords;
        bool key_looked_up;
        Key key;
        millisec timestamp;

        ExtendedKeyEvent();
        ExtendedKeyEvent(BaseKeyEvent event, millisec timestamp);
        ExtendedKeyEvent(uint8_t type, KeyCoords coords, millisec timestamp);
        ExtendedKeyEvent(uint8_t type, KeyCoords coords, Key key, millisec timestamp);

        void look_up_key(KeyMap *keymap);
};

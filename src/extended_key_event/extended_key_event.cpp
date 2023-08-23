#include "extended_key_event.h"
#include "matrix.h"
#include "keymap.h"

ExtendedKeyEvent::ExtendedKeyEvent()
    : ExtendedKeyEvent(EVENT_NONE, COORDS_INVALID, 0)
{}

ExtendedKeyEvent::ExtendedKeyEvent(BaseKeyEvent event, millisec timestamp)
    : ExtendedKeyEvent(event.type, event.coords, timestamp)
{}

ExtendedKeyEvent::ExtendedKeyEvent(uint8_t type, KeyCoords coords, millisec timestamp)
    : type(type)
    , coords(coords)
    , key_looked_up(false)
    , key()
    , timestamp(timestamp)
{}

ExtendedKeyEvent::ExtendedKeyEvent(uint8_t type, KeyCoords coords, Key key, millisec timestamp)
    : type(type)
    , coords(coords)
    , key_looked_up(true)
    , key(key)
    , timestamp(timestamp)
{}

void ExtendedKeyEvent::look_up_key(KeyMap *keymap) {
    if (this->key_looked_up || this->type == EVENT_TIMEOUT) {
        return;
    }

    this->key = keymap->get_key(this->coords);
    this->key_looked_up = true;
}

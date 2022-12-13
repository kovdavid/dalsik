#include "pressed_keys.h"

PressedKeys::PressedKeys() {
    this->count = 0;
    for (uint8_t i = 0; i < PRESSED_KEY_BUFFER; i++) {
        this->keys[i] = PressedKey {};
    }
}

PressedKey* PressedKeys::add(KeyInfo key_info, millisec now, uint8_t key_press_counter) {
    if (this->count >= PRESSED_KEY_BUFFER) {
        return NULL;
    }

    uint8_t key_index = this->count++;

    PressedKey *pk = &(this->keys[key_index]);
    pk->key_info = key_info;
    pk->timestamp = now;
    pk->key_press_counter = key_press_counter;
    pk->state = STATE_NOT_PROCESSED;
    pk->key_index = key_index;

    return pk;
}

PressedKey* PressedKeys::find(KeyCoords coords) {
    for (uint8_t i = 0; i < PRESSED_KEY_BUFFER; i++) {
        if (!this->keys[i].timestamp) {
            break;
        }

        PressedKey *pk = &(this->keys[i]);

        if (pk->key_info.coords.equals(coords)) {
            return pk;
        }
    }

    return NULL;
}

void PressedKeys::remove(PressedKey *pk) {
    this->count--;
    uint8_t last_index = PRESSED_KEY_BUFFER - 1;

    for (uint8_t i = pk->key_index; i < last_index; i++) {
        this->keys[i] = this->keys[i+1];
        this->keys[i].key_index = i;
    }

    this->keys[last_index] = PressedKey {};
}

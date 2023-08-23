#include "Arduino.h"
#include "pressed_keys.h"
#include "extended_key_event.h"
#include "key.h"

PressedKeys::PressedKeys()
    : count(0)
    , keys()
{}

PressedKey* PressedKeys::add(ExtendedKeyEvent event, uint8_t key_press_counter) {
    if (this->count >= PRESSED_KEY_CAPACITY) {
        return NULL;
    }

    uint8_t key_index = this->count++;

    PressedKey *pk = &(this->keys[key_index]);
    pk->key = event.key;
    pk->timestamp = event.timestamp;
    pk->key_press_counter = key_press_counter;
    pk->state = STATE_NOT_PROCESSED;
    pk->key_index = key_index;

    return pk;
}

PressedKey* PressedKeys::find(KeyCoords coords) {
    for (uint8_t i = 0; i < PRESSED_KEY_CAPACITY; i++) {
        if (!this->keys[i].timestamp) {
            break;
        }

        PressedKey *pk = &(this->keys[i]);

        if (pk->key.coords.equals(coords)) {
            return pk;
        }
    }

    return NULL;
}

void PressedKeys::remove(PressedKey *pk) {
    this->count--;
    uint8_t last_index = PRESSED_KEY_CAPACITY - 1;

    for (uint8_t i = pk->key_index; i < last_index; i++) {
        this->keys[i] = this->keys[i+1];
        this->keys[i].key_index = i;
    }

    this->keys[last_index] = PressedKey {};
}

PressedKey* PressedKeys::get_last() {
    if (this->count > 0) {
        return &(this->keys[this->count - 1]);
    } else {
        return NULL;
    }
}

void PressedKeys::print_internal_state(millisec now) {
    for (uint8_t i = 0; i < this->count; i++) {
        PressedKey pk = this->keys[i];
        Key key = pk.key;
        uint8_t state = pk.state;

        Serial.print("  -[");
        Serial.print(i);
        Serial.print("/");
        Serial.print(pk.key_index);
        Serial.print("] ");
        key.print_internal_state();
        Serial.print("       - key_press_counter:");
        Serial.print(pk.key_press_counter);
        Serial.print("\n        - timestamp:");
        Serial.print(pk.timestamp);
        Serial.print(" now-timestamp:");
        Serial.print(now - pk.timestamp);
        Serial.print("\n        - state:");
        if (state == STATE_NOT_PROCESSED) {
            Serial.print("STATE_NOT_PROCESSED");
        } else if (state == STATE_PENDING) {
            Serial.print("STATE_PENDING");
        } else if (state == STATE_ACTIVE_LAYER) {
            Serial.print("STATE_ACTIVE_LAYER");
        } else if (state == STATE_ACTIVE_MODIFIERS) {
            Serial.print("STATE_ACTIVE_MODIFIERS");
        } else if (state == STATE_ACTIVE_CODE) {
            Serial.print("STATE_ACTIVE_CODE");
        } else if (state == STATE_RELEASED) {
            Serial.print("STATE_RELEASED");
        }
        Serial.print("\n");
    }
}

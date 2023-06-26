#include "Arduino.h"
#include "held_up_keys.h"
#include "held_up_key.h"
#include "dalsik.h"

HeldUpKeys::HeldUpKeys()
    : count(0)
    , keys()
{}

HeldUpKey* HeldUpKeys::add(KeyCoords coords, millisec timestamp) {
    if (this->count >= HELD_UP_KEYS_SIZE) {
        return NULL;
    }

    uint8_t key_index = this->count++;

    HeldUpKey *hkey = &(this->keys[key_index]);
    hkey->coords = coords;
    hkey->timestamp = timestamp;
    hkey->key_index = key_index;

    return hkey;
}

HeldUpKey* HeldUpKeys::find_by_coords(KeyCoords coords) {
    for (uint8_t i = 0; i < HELD_UP_KEYS_SIZE; i++) {
        if (!this->keys[i].timestamp) {
            break;
        }

        HeldUpKey *hkey = &(this->keys[i]);

        if (hkey->coords.equals(coords)) {
            return hkey;
        }
    }

    return NULL;
}

HeldUpKey* HeldUpKeys::get_by_index(uint8_t index) {
    return &(this->keys[index]);
}

void HeldUpKeys::remove(HeldUpKey* hkey) {
    this->count--;
    uint8_t last_index = HELD_UP_KEYS_SIZE - 1;

    for (uint8_t i = hkey->key_index; i < last_index; i++) {
        this->keys[i] = this->keys[i+1];
        this->keys[i].key_index = i;
    }

    this->keys[last_index] = HeldUpKey {};
}

// Move every occupied element to the beginning of `this->keys`
void HeldUpKeys::repack_keys() {
    uint8_t last_occupied_index = 0;

    // Move occupied elements to the beginning
    for (uint8_t i = 0; i < HELD_UP_KEYS_SIZE; i++) {
        if (this->keys[i].timestamp) {
            this->keys[last_occupied_index++] = this->keys[i];
        }
    }

    this->count = last_occupied_index;

    // Clear out the rest
    while (last_occupied_index < HELD_UP_KEYS_SIZE) {
        this->keys[last_occupied_index++].clear();
    }
}

void HeldUpKeys::print_internal_state(millisec now) {
    Serial.print("\nHeldUpKeys count:");
    Serial.print(this->count);
    Serial.print("\n");
    for (uint8_t i = 0; i < this->count; i++) {
        HeldUpKey hkey = this->keys[i];
        Serial.print("\t R:");
        Serial.print(hkey.coords.row);
        Serial.print("|C:");
        Serial.print(hkey.coords.col);
        Serial.print(" timestamp:");
        Serial.print(hkey.timestamp);
        Serial.print(" now-timestamp:");
        Serial.print(now - hkey.timestamp);
        Serial.print(" part_of_active_combo:");
        Serial.print(hkey.part_of_active_combo);
        Serial.print(" active_combo_index:");
        Serial.print(hkey.active_combo_index);
        Serial.print(" key_index:");
        Serial.print(hkey.key_index);
        Serial.print("\n");
    }
    Serial.print("\n");
}

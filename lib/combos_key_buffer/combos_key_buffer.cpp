#include "Arduino.h"
#include "combos_key_buffer.h"

CombosKeyBuffer::CombosKeyBuffer() {
    this->count = 0;
    for (uint8_t i = 0; i < COMBOS_KEY_BUFFER_SIZE; i++) {
        this->keys[i] = CombosBufferedKey {};
    }
}

CombosBufferedKey* CombosKeyBuffer::add(KeyCoords coords, millisec now) {
    if (this->count >= COMBOS_KEY_BUFFER_SIZE) {
        return NULL;
    }

    uint8_t key_index = this->count++;

    CombosBufferedKey *cbk = &(this->keys[key_index]);
    cbk->set_coords(coords, now);

    return cbk;
}

CombosBufferedKey* CombosKeyBuffer::find(KeyCoords coords) {
    for (uint8_t i = 0; i < COMBOS_KEY_BUFFER_SIZE; i++) {
        if (!this->keys[i].timestamp) {
            break;
        }

        CombosBufferedKey *cbk = &(this->keys[i]);

        if (cbk->coords.equals(coords)) {
            return cbk;
        }
    }

    return NULL;
}

void CombosKeyBuffer::normalize() {
    int8_t last_index = 0;

    for (uint8_t i = 0; i < COMBOS_KEY_BUFFER_SIZE; i++) {
        if (this->keys[i].timestamp == 0) continue;

        CombosBufferedKey temp = this->keys[last_index];
        this->keys[last_index] = this->keys[i];
        this->keys[i] = temp;

        last_index++;
    }

    this->count = last_index;
}

CombosBufferedKey* CombosKeyBuffer::get(uint8_t index) {
    return &(this->keys[index]);
}

void CombosKeyBuffer::print_internal_state(millisec now) {
    Serial.print("\nCombosKeyBuffer count:");
    Serial.print(this->count);
    Serial.print("\n");
    for (uint8_t i = 0; i < this->count; i++) {
        CombosBufferedKey cbk = this->keys[i];
        Serial.print("\t R:");
        Serial.print(cbk.coords.row);
        Serial.print("|C:");
        Serial.print(cbk.coords.col);
        Serial.print(" timestamp:");
        Serial.print(cbk.timestamp);
        Serial.print(" now-timestamp:");
        Serial.print(now - cbk.timestamp);
        Serial.print(" part_of_active_combo:");
        Serial.print(cbk.part_of_active_combo);
        Serial.print(" active_combo_index:");
        Serial.print(cbk.active_combo_index);
        Serial.print("\n");
    }
    Serial.print("\n");
}

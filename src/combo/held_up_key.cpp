#include "held_up_key.h"
#include "dalsik.h"

HeldUpKey::HeldUpKey()
    : coords()
    , timestamp(0)
    , part_of_active_combo(false)
    , active_combo_index(0)
{}

void HeldUpKey::activate(uint8_t combo_index) {
    this->active_combo_index = combo_index;
    this->part_of_active_combo = true;
}

void HeldUpKey::clear() {
    this->timestamp = 0;
    this->part_of_active_combo = false;
    this->active_combo_index = 0;
}

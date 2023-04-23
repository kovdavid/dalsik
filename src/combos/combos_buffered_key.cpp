#include "combos_buffered_key.h"

CombosBufferedKey::CombosBufferedKey() {
    this->coords = COORDS_INVALID;
    this->timestamp = 0;
    this->part_of_active_combo = false;
    this->active_combo_index = 0;
}

void CombosBufferedKey::activate(uint8_t combo_index) {
    this->active_combo_index = combo_index;
    this->part_of_active_combo = true;
}

void CombosBufferedKey::clear() {
    this->active_combo_index = 0;
    this->part_of_active_combo = false;
    this->timestamp = 0;
}

void CombosBufferedKey::set_coords(KeyCoords coords, millisec now) {
    this->coords = coords;
    this->timestamp = now;
}

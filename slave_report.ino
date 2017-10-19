#include "dalsik.h"
#include "keyboard.h"
#include <Wire.h>

SlaveReport::SlaveReport() {}

void SlaveReport::handle_changed_key(ChangedKeyCoords coords) {
    if (coords.type != EVENT_NONE) {
        Wire.beginTransmission(I2C_MASTER_ADDRESS);
        Wire.write(coords.type);
        Wire.write(coords.row);
        Wire.write(coords.col);
        Wire.endTransmission();
    }
}

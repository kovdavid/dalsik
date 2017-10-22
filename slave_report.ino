#include "dalsik.h"
#include "keyboard.h"
#include <Wire.h>
#include <Arduino.h>

SlaveReport::SlaveReport() {}

void SlaveReport::handle_changed_key(ChangedKeyCoords coords) {
    if (coords.type == EVENT_NONE) {
        return;
    }

#if DEBUG
    Serial.print("Slave report<t");
    Serial.print(coords.type, HEX);
    Serial.print("-r");
    Serial.print(coords.row, HEX);
    Serial.print("-c");
    Serial.print(coords.col, HEX);
    Serial.print(">\n");
#endif

    uint8_t checksum = coords.type + coords.row + coords.col;

#if USE_I2C
    Wire.beginTransmission(I2C_MASTER_ADDRESS);
    Wire.write(coords.type);
    Wire.write(coords.row);
    Wire.write(coords.col);
    Wire.write(checksum);
    Wire.endTransmission();
#else
    Serial1.write(coords.type);
    Serial1.write(coords.row);
    Serial1.write(coords.col);
    Serial1.write(checksum);
    Serial1.flush();
#endif
}

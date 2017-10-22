#include "dalsik.h"
#include "keyboard.h"
#include <Wire.h>
#include <Arduino.h>

SlaveReport::SlaveReport() {}

void SlaveReport::handle_changed_key(ChangedKeyCoords coords) {
    if (coords.type == EVENT_NONE) {
        return;
    }

    SlaveReportData slave_data = encode_slave_report_data(coords);

#if DEBUG
    Serial.print("Slave report<t");
    Serial.print(coords.type, HEX);
    Serial.print("-r");
    Serial.print(coords.row, HEX);
    Serial.print("-c");
    Serial.print(coords.col, HEX);
    Serial.print(">\n");
#endif

#if USE_I2C
    Wire.beginTransmission(I2C_MASTER_ADDRESS);
    Wire.write(slave_data.data);
    Wire.write(slave_data.checksum);
    Wire.endTransmission();
#else
    Serial1.write(slave_data.data);
    Serial1.write(slave_data.checksum);
    Serial1.flush();
#endif
}

// We use 1B to send type, row and col from ChangedKeyCoords
// XRYYRZZZ
// X - EVENT_KEY_PRESS:1 EVENT_KEY_RELEASE:0 (EVENT_NONE is not sent by slave)
// Y - row
// Z - column
// R - reserved/unused

SlaveReportData encode_slave_report_data(ChangedKeyCoords coords) {
    uint8_t data = 0x00;

    if (coords.type == EVENT_KEY_PRESS) {
        data |= 0x80;
    }
    data |= (coords.row << 4) & 0x3F;
    data |= (coords.col & 0x07);

    return SlaveReportData { data, (uint8_t)~data };
}

ChangedKeyCoords decode_slave_report_data(SlaveReportData slave_data) {
    ChangedKeyCoords coords = { EVENT_NONE, 0, 0 };
    if (slave_data.data == (uint8_t)~slave_data.checksum) {
        coords.type = (slave_data.data & 0x80) ? EVENT_KEY_PRESS : EVENT_KEY_RELEASE;
        coords.row  = (slave_data.data >> 4) & 0x3F;
        coords.col  = (slave_data.data & 0x07);
    }
    return coords;
}

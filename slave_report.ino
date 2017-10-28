#include "dalsik.h"
#include "keyboard.h"
#include "dalsik_serial.h"
#include <Wire.h>
#include <Arduino.h>

SlaveReport::SlaveReport() {}

void SlaveReport::handle_changed_key(ChangedKeyCoords coords) {
    if (coords.type == EVENT_NONE) {
        return;
    }

    uint8_t slave_data = SlaveReport::encode_slave_report_data(coords);

#if DEBUG
    Serial.print("Slave report<t");
    Serial.print(coords.type, HEX);
    Serial.print("-r");
    Serial.print(coords.row, HEX);
    Serial.print("-c");
    Serial.print(coords.col, HEX);
    Serial.print("> slave_data:");
    Serial.print(slave_data, HEX);
    Serial.print("\n");
#endif

#if USE_I2C
    Wire.beginTransmission(I2C_MASTER_ADDRESS);
    Wire.write(slave_data);
    Wire.endTransmission();
#else
    DalsikSerial::slave_send(slave_data);
#endif
}

// We use 1B to send type, row and col from ChangedKeyCoords + parity
// ABBCCCDE
// A - EVENT_KEY_PRESS:1 EVENT_KEY_RELEASE:0 (EVENT_NONE is not sent by slave)
// B - row
// C - column
// D - parity of A+B+C (bitmask 1111_1100 - FC)
// E - value 1 (value 0x00 means "no data" - it should no be sent; this bit prevents that)

uint8_t SlaveReport::encode_slave_report_data(ChangedKeyCoords coords) {
    uint8_t data = 0x00;

    if (coords.type == EVENT_KEY_PRESS) {
        data |= 0x80;
    }
    data |= (coords.row << 5) & 0x60;
    data |= (coords.col << 2) & 0x1C;

    uint8_t p = SlaveReport::parity(data & 0xFC);

    data |= (p << 1) & 0x02;
    data |= 0x01;

    return data;
}

ChangedKeyCoords SlaveReport::decode_slave_report_data(uint8_t data) {
    ChangedKeyCoords coords;

    coords.type = (data & 0x80) ? EVENT_KEY_PRESS : EVENT_KEY_RELEASE;
    coords.row  = (data >> 5) & 0x03;
    coords.col  = (data >> 2) & 0x07;

    uint8_t p = (data >> 1) & 0x01;
    uint8_t calc_p = SlaveReport::parity(data & 0xFC);

#if DEBUG
    Serial.print("Slave report<t");
    Serial.print(coords.type, HEX);
    Serial.print("-r");
    Serial.print(coords.row, HEX);
    Serial.print("-c");
    Serial.print(coords.col, HEX);
    Serial.print("> slave_data:");
    Serial.print(slave_data, HEX);
    Serial.print("\n");
#endif

    if (p == calc_p) {
        return coords;
    } else {
        return ChangedKeyCoords { EVENT_NONE, 0, 0 };
    }
}

inline uint8_t SlaveReport::parity(uint8_t d) {
    d ^= (d >> 4);
    d ^= (d >> 2);
    d ^= (d >> 1);
    return d & 0x1;
}

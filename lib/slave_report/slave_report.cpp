#include "dalsik.h"
#include "matrix.h"
#include "dalsik_serial.h"
#include "slave_report.h"

void SlaveReport::send_changed_key(ChangedKeyEvent event) {
    if (event.type == EVENT_NONE) {
        return;
    }

    uint8_t slave_data = SlaveReport::encode_slave_report_data(event);

    DalsikSerial::slave_send(slave_data);
}

inline uint8_t parity(uint8_t d) {
    d ^= (d >> 4);
    d ^= (d >> 2);
    d ^= (d >> 1);
    return d & 0x1;
}

// We use 1B to send type, row and col from ChangedKeyEvent + 2 parity
// ABBCCCDE
// A - EVENT_KEY_PRESS:1 EVENT_KEY_RELEASE:0 (EVENT_NONE is not sent by slave)
// B - row
// C - column
// D - parity of A+B+C (bitmask 1111_1100 - FC)
// E - parity of B+C (bitmask 0111_1100 - 7C)

uint8_t SlaveReport::encode_slave_report_data(ChangedKeyEvent event) {
    uint8_t data = 0x00;

    if (event.type == EVENT_KEY_PRESS) {
        data |= 0x80;
    }
    data |= (event.coords.row << 5) & 0x60;
    data |= (event.coords.col << 2) & 0x1C;

    uint8_t p1 = parity(data & 0xFC);
    uint8_t p2 = parity(data & 0x7C);

    data |= (p1 << 1) & 0x02;
    data |= (p2 << 0) & 0x01;

    return data;
}

ChangedKeyEvent SlaveReport::decode_slave_report_data(uint8_t data) {
    ChangedKeyEvent event;

    event.type = (data & 0x80) ? EVENT_KEY_PRESS : EVENT_KEY_RELEASE;
    event.coords.row = (data >> 5) & 0x03;
    event.coords.col = (data >> 2) & 0x07;

    uint8_t p1 = (data >> 1) & 0x01;
    uint8_t p2 = (data >> 0) & 0x01;

    uint8_t calc_p1 = parity(data & 0xFC);
    uint8_t calc_p2 = parity(data & 0x7C);

    if (p1 == calc_p1 && p2 == calc_p2) {
        return event;
    } else {
        return ChangedKeyEvent { EVENT_NONE, KeyCoords { 0, 0 } };
    }
}

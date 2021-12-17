/*
 * This implementation was largely influenced by the serial.c file from QMK:
 * https://github.com/qmk/qmk_firmware/blob/master/keyboards/lets_split/serial.c
 */

#pragma once

#include "pin_utils.h"
#include "ring_buffer.h"

#define SERIAL_PIN PIN_D(0)
#define SERIAL_PIN_INTERRUPT INT0_vect
#define SERIAL_PIN_INTERRUPT_FLAG INTF0

// Serial pulse period in microseconds.
#define SERIAL_DELAY 50

namespace DalsikSerial {
    void master_init();
    void slave_init();
    void slave_send(uint8_t data);
    uint8_t has_data();
    uint8_t get_next_elem();
}

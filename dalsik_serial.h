/*
 * This implementation was largely influenced by the serial.c file from QMK:
 * https://github.com/qmk/qmk_firmware/blob/master/keyboards/lets_split/serial.c
 */

#ifndef DALSIK_SERIAL_H
#define DALSIK_SERIAL_H

#define SERIAL_PIN_DDR DDRD
#define SERIAL_PIN_PORT PORTD
#define SERIAL_PIN_INPUT PIND
#define SERIAL_PIN_MASK _BV(PD0)
#define SERIAL_PIN_INTERRUPT INT0_vect
#define SERIAL_PIN_INTERRUPT_FLAG INTF0

// Serial pulse period in microseconds.
#define SERIAL_DELAY 50

namespace DalsikSerial {
    // Buffers for master - slave communication
    extern volatile uint8_t slave_data;

    void master_init();
    void slave_init();
    void slave_send(uint8_t data);
};

#endif

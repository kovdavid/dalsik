/*
 * This implementation was largely influenced by the serial.c file from QMK:
 * https://github.com/qmk/qmk_firmware/blob/master/keyboards/lets_split/serial.c
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "pin_utils.h"
#include "dalsik_serial.h"
#include "dalsik.h"

volatile uint8_t DalsikSerial::slave_data = 0x00;
volatile uint8_t DalsikSerial::slave_data_available = 0;

void DalsikSerial::master_init(void) {
    PinUtils::pinmode_input_pullup(SERIAL_PIN);
    serial_set_interrupt();
}

void DalsikSerial::slave_init(void) {
    PinUtils::pinmode_output_high(SERIAL_PIN);
}

void DalsikSerial::slave_send(uint8_t data) {
    // Trigger the interrupt on the master & send the init LOW/HIGH
    PinUtils::set_output_low(SERIAL_PIN);
    serial_delay();
    PinUtils::set_output_high(SERIAL_PIN);
    serial_delay();

    // Send data - MSB
    for (int8_t i = 7; i >= 0; i--) {
        if (data & (1 << i)) {
            PinUtils::set_output_high(SERIAL_PIN);
        } else {
            PinUtils::set_output_low(SERIAL_PIN);
        }
        serial_delay();
    }

    // Pull the line HIGH - IDLE
    PinUtils::set_output_high(SERIAL_PIN);
    serial_delay();
}

inline static uint8_t serial_master_read() {
    serial_half_delay();
    // Sync to the end of slave LOW
    while (!PinUtils::read_pin(SERIAL_PIN));
    // Wait till the HIGH signal
    serial_delay();
    // Get to the middle of the data signal
    serial_half_delay();

    uint8_t data = 0;

    // Receive data - MSB
    for (uint8_t i = 0; i < 8; i++) {
        data = (data << 1) | PinUtils::read_pin(SERIAL_PIN);
        serial_delay();
    }

    return data;
}

//=========================//
//=======// Utils //=======//
//=========================//

inline static void serial_set_interrupt() {
    // Enable INT0
    EIMSK |= _BV(INT0);
    // Trigger on falling edge of INT0
    EICRA |=  (1 << ISC01);
    EICRA &= ~(1 << ISC00);
}

inline static void serial_delay() {
    _delay_us(SERIAL_DELAY);
}

inline static void serial_half_delay() {
    _delay_us(SERIAL_DELAY/2);
}

#if IS_MASTER
ISR(SERIAL_PIN_INTERRUPT) {
    DalsikSerial::slave_data = serial_master_read();
    DalsikSerial::slave_data_available = 1;
    // Clear pending interrupts for INTF0 (which were registered during serial_master_read)
    EIFR |= (1 << SERIAL_PIN_INTERRUPT_FLAG);
}
#endif

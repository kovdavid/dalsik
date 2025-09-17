/*
 * This implementation was largely influenced by the serial.c file from QMK:
 * https://github.com/qmk/qmk_firmware/blob/master/keyboards/lets_split/serial.c
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "pin_utils.h"
#include "ring_buffer.h"
#include "dalsik_serial.h"
#include "dalsik.h"
#include "keyboard_hardware.h"

inline static void serial_set_interrupt();
inline static void serial_delay();
inline static void serial_half_delay();
inline static uint8_t serial_master_read();

RingBuffer serial_buffer;

void DalsikSerial::master_init(void) {
    _delay_ms(100);

    PinUtils::pinmode_input_pullup(SERIAL_PIN);
    serial_set_interrupt();
}

void DalsikSerial::slave_init(void) {
    PinUtils::pinmode_output_high(SERIAL_PIN);
}

uint8_t DalsikSerial::has_data() {
    return serial_buffer.has_data();
}

uint8_t DalsikSerial::get_next_elem() {
    return serial_buffer.get_next_elem();
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
    // ATmega32U4 datasheet chapter 11.1.1 - External Interrupt Control Register A - EICRA
    EICRA &= ~(1 << ISC00);
    EICRA |=  (1 << ISC01);

    EIFR |= (1 << SERIAL_PIN_INTERRUPT_FLAG);
}

inline static void serial_delay() {
    _delay_us(SERIAL_DELAY);
}

inline static void serial_half_delay() {
    _delay_us(SERIAL_DELAY/2);
}

ISR(SERIAL_PIN_INTERRUPT) {
    uint8_t slave_data = serial_master_read();
    serial_buffer.append_elem(slave_data);
    // Clear pending interrupts for INTF0 (which were registered during serial_master_read)
    EIFR |= (1 << SERIAL_PIN_INTERRUPT_FLAG);
}

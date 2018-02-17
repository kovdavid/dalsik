#include <avr/io.h>
#include "dalsik.h"

// LED ws2812
// "0" -> HI[0.35] LO[0.80] ->  6cycle high + 13cycle low = 19cycle
// "1" -> HI[0.70] LO[0.60] -> 11cycle high + 10cycle low = 21cycle

unsigned long last_led_ts = micros();

void set_led_rgb(uint8_t red, uint8_t green, uint8_t blue) {
#ifdef LED_PIN
    while ((micros() - last_led_ts) < 50L); // wait for 50us (data latch)
    uint8_t grb_array[] = { green, red, blue };

    volatile uint8_t *port        = &(_SFR_IO8((LED_PIN >> 4) + 2));
    volatile uint8_t port_high    = *port |  (_BV(LED_PIN & 0x0F));
    volatile uint8_t port_low     = *port & ~(_BV(LED_PIN & 0x0F));
    volatile uint8_t bit_index    = 8;
    volatile uint8_t byte_index   = 3;
    volatile uint8_t *val_pointer = grb_array;
    volatile uint8_t val          = *val_pointer++;

    cli();
    asm volatile(
    "loop_bit:\n\t"
        "OUT  %[PORT], %[PORT_HIGH]\n\t"
        "DEC  %[BIT_INDEX]\n\t"
        "BREQ next_byte\n\t"
        "SBRS %[VAL], 7\n\t"
        "RJMP output_low\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
    "output_low:\n\t"
        "OUT  %[PORT], %[PORT_LOW]\n\t"
        "SBRC %[VAL], 7\n\t"
        "RJMP wait_logical_one\n\t"
        "RJMP wait_logical_zero\n\t"
    "wait_logical_zero:\n\t"
        "NOP\n\t"
    "wait_logical_one:\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "ROL %[VAL]\n\t"
        "RJMP loop_bit\n\t"
    "next_byte:\n\t"
        "NOP\n\t"
        "SBRS %[VAL], 7\n\t"
        "OUT  %[PORT], %[PORT_LOW]\n\t"
        "LDI %[BIT_INDEX], 8\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "OUT  %[PORT], %[PORT_LOW]\n\t"
        "SBRS %[VAL], 7\n\t"
        "RJMP load_next_byte\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
    "load_next_byte:\n\t"
        "LD  %[VAL], %a[VAL_POINTER]+\n\t"
        "DEC %[BYTE_INDEX]\n\t"
        "BRNE loop_bit\n\t"
    ::
        [VAL_POINTER] "e" (val_pointer),
        [PORT]        "I" (_SFR_IO_ADDR(*port)),
        [PORT_LOW]    "r" (port_low),
        [PORT_HIGH]   "r" (port_high),
        [BIT_INDEX]   "r" (bit_index),
        [BYTE_INDEX]  "r" (byte_index),
        [VAL]         "r" (val)
    );
    sei();
    last_led_ts = micros();
#endif
}

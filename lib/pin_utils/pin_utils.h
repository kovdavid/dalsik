#pragma once

#include "avr/io.h"
#include <stdint.h>

#define PIN_B(x) (0x30+x)
#define PIN_C(x) (0x60+x)
#define PIN_D(x) (0x90+x)
#define PIN_E(x) (0xC0+x)
#define PIN_F(x) (0xF0+x)

namespace PinUtils {
    inline void set_output_high(uint8_t pin) {
        _SFR_IO8((pin >> 4) + 2) |= _BV(pin & 0x0F); // HIGH
    }

    inline void set_output_low(uint8_t pin) {
        _SFR_IO8((pin >> 4) + 2) &= ~_BV(pin & 0x0F); // LOW
    }

    inline void pinmode_input_pullup(uint8_t pin) {
        _SFR_IO8((pin >> 4) + 1) &= ~_BV(pin & 0x0F); // INPUT
        _SFR_IO8((pin >> 4) + 2) |=  _BV(pin & 0x0F); // HIGH
    }

    inline void pinmode_output_low(uint8_t pin) {
        _SFR_IO8((pin >> 4) + 1) |=  _BV(pin & 0x0F); // OUTPUT
        PinUtils::set_output_low(pin);
    }

    inline void pinmode_output_high(uint8_t pin) {
        _SFR_IO8((pin >> 4) + 1) |= _BV(pin & 0x0F); // OUTPUT
        PinUtils::set_output_high(pin);
    }

    inline uint8_t read_pin(uint8_t pin) {
        return (_SFR_IO8(pin >> 4) & _BV(pin & 0x0F));
    }
}

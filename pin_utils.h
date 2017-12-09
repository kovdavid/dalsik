#ifndef PIN_UTILS_H
#define PIN_UTILS_H

#include "avr/io.h"

#define PIN_B(x) (0x30+x)
#define PIN_C(x) (0x60+x)
#define PIN_D(x) (0x90+x)
#define PIN_E(x) (0xC0+x)
#define PIN_F(x) (0xF0+x)

namespace PinUtils {
    inline static void pinmode_input_pullup(uint8_t pin);
    inline static void pinmode_output_low(uint8_t pin);
    inline static void pinmode_output_high(uint8_t pin);
    inline static void set_output_high(uint8_t pin);
    inline static void set_output_low(uint8_t pin);
    inline static uint8_t read_pin(uint8_t pin);
}

#endif

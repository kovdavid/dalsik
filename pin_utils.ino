#include "avr/io.h"

inline static void PinUtils::pinmode_input_pullup(uint8_t pin) {
    _SFR_IO8((pin >> 4) + 1) &= ~_BV(pin & 0x0F); // INPUT
    _SFR_IO8((pin >> 4) + 2) |=  _BV(pin & 0x0F); // HIGH
}

inline static void PinUtils::pinmode_output_low(uint8_t pin) {
    _SFR_IO8((pin >> 4) + 1) |=  _BV(pin & 0x0F); // OUTPUT
    set_output_low(pin);
}

inline static void PinUtils::pinmode_output_high(uint8_t pin) {
    _SFR_IO8((pin >> 4) + 1) |= _BV(pin & 0x0F); // OUTPUT
    set_output_high(pin);
}

inline static void PinUtils::set_output_high(uint8_t pin) {
    _SFR_IO8((pin >> 4) + 2) |= _BV(pin & 0x0F); // HIGH
}

inline static void PinUtils::set_output_low(uint8_t pin) {
    _SFR_IO8((pin >> 4) + 2) &= ~_BV(pin & 0x0F); // LOW
}

inline static uint8_t PinUtils::read_pin(uint8_t pin) {
    return (_SFR_IO8(pin >> 4) & _BV(pin & 0x0F));
}

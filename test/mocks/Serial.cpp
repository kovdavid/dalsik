#include "Arduino.h"

Serial_CLS Serial;

void Serial_CLS::print(const __FlashStringHelper *value) {
    const char* val = reinterpret_cast<const char*>(value);
    printf("%s", val);
}
void Serial_CLS::print(int value) {
    printf("%d", value);
}
void Serial_CLS::print(uint8_t value) {
    printf("%d", value);
}
void Serial_CLS::print(unsigned long value) {
    printf("%lu", value);
}
void Serial_CLS::print(uint8_t value, int format) {
    if (format == HEX) {
        printf("0x%02X", value);
    } else if (format == BIN) {
        printf("0b");
        for (int8_t i = 7; i >= 0; i--) {
            if (value & (1 << i)) {
                printf("1");
            } else {
                printf("0");
            }
            if (i == 4) {
                printf("_");
            }
        }
    } else {
        printf("%d", value);
    }
}
void Serial_CLS::print(unsigned char* msg, int format UNUSED) {
    printf("%s", msg);
}
void Serial_CLS::print(const char* msg) {
    printf("%s", msg);
}
void Serial_CLS::println(unsigned char* msg) {
    printf("%s\n", msg);
}

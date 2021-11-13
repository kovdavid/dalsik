#include <stdio.h>
#include "Arduino.h"
#include "misc.h"

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
void Serial_CLS::print(uint8_t value, int format UNUSED) {
    printf("%d", value);
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

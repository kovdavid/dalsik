#pragma once

#include <stdint.h>

#define PROGMEM
#define memcpy_P memcpy

inline uint32_t pgm_read_dword(uint32_t* addr) {
    return *addr;
}

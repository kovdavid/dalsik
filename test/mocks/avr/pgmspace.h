#pragma once

#define PROGMEM

inline uint32_t pgm_read_dword(uint32_t* addr) {
    return *addr;
}

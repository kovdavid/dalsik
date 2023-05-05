#pragma once

#include <stdint.h>
#include <stdbool.h>

namespace ArrayUtils {
    void append_uniq_uint8(uint8_t* array, uint8_t array_size, uint8_t elem);
    uint8_t remove_and_return_last_uint8(uint8_t* array, uint8_t array_size, uint8_t elem);
    bool contains_uint8(uint8_t* array, uint8_t array_size, uint8_t elem);
}

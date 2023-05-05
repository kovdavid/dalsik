#include <stdint.h>
#include <stdbool.h>
#include "array_utils.h"

void ArrayUtils::append_uniq_uint8(uint8_t* array, uint8_t array_size, uint8_t elem) {
    for (uint8_t i = 0; i < array_size-1; i++) {
        if (array[i] == elem) {
            return;
        }
        if (array[i] == 0x00) {
            array[i] = elem;
            return;
        }
    }
}

// Keep every non-zero element on the left after the removal
// Bad:  [0x01, 0x00, 0x03, 0x04, 0x00]
// Good: [0x01, 0x03, 0x04, 0x00, 0x00]
uint8_t ArrayUtils::remove_and_return_last_uint8(uint8_t* array, uint8_t array_size, uint8_t elem) {
    uint8_t last_nonzero_elem_index = 0;
    uint8_t last_elem = 0x00;
    for (uint8_t i = 0; i < array_size; i++) {
        if (array[i] == elem || array[i] == 0x00) {
            array[i] = 0x00;
            continue;
        }

        last_elem = array[i];

        if (last_nonzero_elem_index == i) {
            array[last_nonzero_elem_index++] = array[i];
        } else {
            array[last_nonzero_elem_index++] = array[i];
            array[i] = 0x00;
        }
    }

    return last_elem;
}

bool ArrayUtils::contains_uint8(uint8_t* array, uint8_t array_size, uint8_t elem) {
    for (uint8_t i = 0; i < array_size; i++) {
        if (array[i] == elem) {
            return true;
        }
    }

    return false;
}

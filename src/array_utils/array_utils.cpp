#include <stdint.h>
#include <stdbool.h>
#include "array_utils.h"

// Add an element to the first position of the array with the value `0x00`
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

// Remove element from an array, keep every non-zero element on the beginning
// Bad:  [0x01, 0x00, 0x03, 0x04, 0x00]
// Good: [0x01, 0x03, 0x04, 0x00, 0x00]
void ArrayUtils::remove_uint8(uint8_t* array, uint8_t array_size, uint8_t elem) {
    uint8_t last_nonzero_elem_index = 0;
    for (uint8_t i = 0; i < array_size; i++) {
        if (array[i] == elem || array[i] == 0x00) {
            array[i] = 0x00;
            continue;
        }

        if (last_nonzero_elem_index == i) {
            array[last_nonzero_elem_index++] = array[i];
        } else {
            array[last_nonzero_elem_index++] = array[i];
            array[i] = 0x00;
        }
    }
}

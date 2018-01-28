void append_uniq_to_uint8_array(uint8_t* array, uint8_t array_size, uint8_t elem) {
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
void remove_uniq_from_uint8_array(uint8_t* array, uint8_t array_size, uint8_t elem) {
    if (elem == 0x00) {
        return; // Nothing to do
    }

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

uint8_t last_nonzero_elem_of_uint8_array(uint8_t* array, uint8_t array_size) {
    for (int8_t i = array_size-1; i >= 0; i--) {
        if (array[i] != 0x00) {
            return array[i];
        }
    }

    return 0x00;
}

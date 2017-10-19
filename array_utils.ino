// From: [0x01, 0x00, 0x03, 0x04, 0x00]
// To:   [0x01, 0x03, 0x04, 0x00, 0x00]
// Shift all the values to the left
void normalize_uint8_array(uint8_t* array, uint8_t array_size) {
    uint8_t last_nonzero_index = 0;
    for (uint8_t i = 0; i < array_size-1; i++) {
        if (array[i] != 0x00) {
            array[last_nonzero_index++] = array[i];
        }
    }
}

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

void remove_uniq_from_uint8_array(uint8_t* array, uint8_t array_size, uint8_t elem) {
    for (uint8_t i = 0; i < array_size-1; i++) {
        if (array[i] == elem) {
            array[i] = 0x00;
            normalize_uint8_array(array, array_size);
            return;
        }
    }
}

uint8_t last_nonzero_elem_of_uint8_array(uint8_t* array, uint8_t array_size) {
    for (uint8_t i = array_size-1; i >= 0; i--) {
        if (array[i] != 0x00) {
            return array[i];
        }
    }

    return 0x00;
}

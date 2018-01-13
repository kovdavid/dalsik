#ifndef ARRAY_UTILS_H
#define ARRAY_UTILS_H

void append_uniq_to_uint8_array(uint8_t* array, uint8_t array_size, uint8_t elem);
void remove_uniq_from_uint8_array(uint8_t* array, uint8_t array_size, uint8_t elem);
uint8_t last_nonzero_elem_of_uint8_array(uint8_t* array, uint8_t array_size);

#endif

#include <util/atomic.h>
#include "ring_buffer.h"

// ProMicro does not do division in HW, so '%' is expensive
inline uint8_t increment_index(uint8_t current_value) {
    uint8_t new_value = current_value + 1;
    if (new_value >= BUFFER_LENGTH) {
        return 0;
    } else {
        return new_value;
    }
}

RingBuffer::RingBuffer()
    : buffer()
    , size(0)
    , read_index(0)
    , write_index(0)
{}

// Called from ISR
void RingBuffer::append_elem(uint8_t elem) {
    if (this->size >= BUFFER_LENGTH) {
        return;
    }
    this->buffer[ this->write_index ] = elem;
    this->write_index = increment_index(this->write_index);
    this->size++;
}

// Called from main loop
uint8_t RingBuffer::get_next_elem() {
    uint8_t elem = this->buffer[ this->read_index ];
    this->read_index = increment_index(this->read_index);
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        this->size--;
    }
    return elem;
}

uint8_t RingBuffer::has_data() {
    return this->size;
}

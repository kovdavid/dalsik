#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#define BUFFER_LENGTH 10

class RingBuffer {
    private:
        volatile uint8_t buffer[BUFFER_LENGTH];
        volatile uint8_t size;
        uint8_t read_index;
        uint8_t write_index;
    public:
        RingBuffer();
        void append_elem(uint8_t elem);
        uint8_t get_next_elem();
        uint8_t has_data();
};

#endif

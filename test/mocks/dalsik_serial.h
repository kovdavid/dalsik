#pragma once

namespace DalsikSerial {
    void master_init() {}
    void slave_init() {}
    void slave_send(uint8_t data) {}
    uint8_t has_data() { return 0; }
    uint8_t get_next_elem() { return 0; }
}

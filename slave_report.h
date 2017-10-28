#ifndef SLAVE_REPORT_H
#define SLAVE_REPORT_H

#include "keyboard.h"

class SlaveReport {
    public:
        SlaveReport();
        void handle_changed_key(ChangedKeyCoords coords);

        static uint8_t encode_slave_report_data(ChangedKeyCoords coords);
        static ChangedKeyCoords decode_slave_report_data(uint8_t data);
    private:
        inline static uint8_t parity(uint8_t d);
};

#endif

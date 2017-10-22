#ifndef SLAVE_REPORT_H
#define SLAVE_REPORT_H

#include "keyboard.h"

typedef struct {
    uint8_t data;
    uint8_t checksum;
} SlaveReportData;

class SlaveReport {
    public:
        SlaveReport();
        void handle_changed_key(ChangedKeyCoords coords);
};

SlaveReportData encode_slave_report_data(ChangedKeyCoords coords);
ChangedKeyCoords decode_slave_report_data(SlaveReportData slave_data);

#endif

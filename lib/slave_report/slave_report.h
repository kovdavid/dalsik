#pragma once

#include "dalsik.h"
#include "matrix.h"

namespace SlaveReport {
    void send_changed_key(ChangedKeyEvent event);
    uint8_t encode_slave_report_data(ChangedKeyEvent event);
    ChangedKeyEvent decode_slave_report_data(uint8_t data);
}

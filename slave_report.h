#ifndef SLAVE_REPORT_H
#define SLAVE_REPORT_H

#include "keyboard.h"

class SlaveReport {
    public:
        SlaveReport();
        void handle_changed_key(ChangedKeyCoords coords);
};

#endif

#ifndef KEYBOARD_h
#define KEYBOARD_h

#include "keyreport.h"

class Keyboard
{
    private:
        uint8_t keys_pressed;
        KeyReport _keyReport;
        void send_report(KeyReport* report);
        void matrix_scan(unsigned long now_msec);
        void clear_report(void);
    public:
        Keyboard(void);
        void loop(unsigned long now_msec);
};

#endif

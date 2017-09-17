#include <EEPROM.h>
#include "keyboard.h"

#include "HID.h"

unsigned long prev_loop_msec = 0;

void setup()
{
    Serial.begin(9600);
    keyboard_init();
    delay(300);
}

void loop()
{
    unsigned long now_msec = millis();

    if ((now_msec - prev_loop_msec) > 1) {
        keyboard_loop(now_msec);
    } else {
        delayMicroseconds(500);
    }

    delay(5000);

    KeyReport report = {0};
    report.keys[0] = 0x04;
    keyboard_send_report(&report); delay(2000);
    report.modifiers = 0x02; // Shift
    keyboard_send_report(&report); delay(2000);
    report.keys[1] = 0x05;
    keyboard_send_report(&report); delay(2000);
    report.keys[0] = 0x00;
    keyboard_send_report(&report); delay(2000);
    report.modifiers = 0x00;
    keyboard_send_report(&report); delay(2000);
    report.keys[1] = 0x00;
    keyboard_send_report(&report); delay(2000);

    delay(10000);
}


// report.keys[0] = key;
// sendKeyReport(&report);
// delay(10);
// report.keys[0] = 0x00;
// sendKeyReport(&report);

// if (Serial.available() > 0) {
    // int c = Serial.read();
    // key = c;
    // Serial.println("OK");
// }

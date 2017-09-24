#include "keyboard.h"

#define ONOFF_OUT_PIN 9
#define ONOFF_IN_PIN 8

unsigned long prev_loop_msec = 0;
Keyboard keyboard;

void setup()
{
    pinMode(ONOFF_IN_PIN, INPUT_PULLUP);
    pinMode(ONOFF_OUT_PIN, OUTPUT);
    digitalWrite(ONOFF_OUT_PIN, LOW);

    Serial.begin(9600);
    delay(300);
}

void loop()
{
    // Turn off the whole keyboard with a switch
    while (digitalRead(ONOFF_IN_PIN) == HIGH) {
        delayMicroseconds(500);
    }

    unsigned long now_msec = millis();

    if ((now_msec - prev_loop_msec) > 1) {
        keyboard.loop(now_msec);
    } else {
        delayMicroseconds(500);
    }
}

// // To use PORTF disable JTAG with writing JTD bit twice within four cycles.
// #if  (defined(__AVR_AT90USB1286__) || defined(__AVR_AT90USB1287__) || defined(__AVR_ATmega32U4__))
    // MCUCR |= _BV(JTD);
    // MCUCR |= _BV(JTD);
// #endif

// KeyReport report = {0};
// report.keys[0] = 0x04;
// keyboard_send_report(&report);
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

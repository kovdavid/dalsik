#include "keyboard.h"
#include "keymap.h"
#include "master_report.h"
#include <Wire.h>

#define ONOFF_OUT_PIN 9
#define ONOFF_IN_PIN 8

unsigned long prev_loop_msec = 0;
KeyMap keymap;
Keyboard keyboard(&keymap);
MasterReport master_report(&keymap);

uint8_t send_slave_report = 0;

void setup()
{
    pinMode(ONOFF_IN_PIN, INPUT_PULLUP);
    pinMode(ONOFF_OUT_PIN, OUTPUT);
    digitalWrite(ONOFF_OUT_PIN, LOW);

#if I2C_MASTER
    I2C_master_init();
#else
    I2C_slave_init();
#endif

    Serial.begin(9600);
    delay(300);
}

inline void I2C_master_init()
{
    Wire.begin(I2C_MASTER_ADDRESS);
}

inline void I2C_slave_init()
{
    Wire.begin(I2C_SLAVE_ADDRESS);
    Wire.onReceive(I2C_receive_event);
    Wire.onRequest(I2C_request_event);
}

void loop()
{
    // Turn off the whole keyboard with a switch
    while (digitalRead(ONOFF_IN_PIN) == HIGH) {
        delayMicroseconds(500);
    }

    if (Serial.available() > 0) {
        process_serial_command(&keyboard, &keymap);
    }

#if I2C_MASTER
    if (Wire.available() > 0) {
        // process slave report
    }
#endif

    unsigned long now_msec = millis();

#if I2C_SLAVE
    if (send_slave_report) {
        keyboard.send_slave_report();
        send_slave_report = 0;
    }
#endif

    if ((now_msec - prev_loop_msec) > 1) {
        master_report.check_special_keys();
        KeyChangeEvent event = keyboard.matrix_scan();
        master_report.handle_key_event(event);
    } else {
        delayMicroseconds(500);
    }
}

void I2C_receive_event(int count)
{
    while (Wire.available() > 0) {
        // uint8_t b = Wire.read();
        Wire.read();
    }
}

void I2C_request_event()
{
    send_slave_report = 1;
}

// #if I2C_MASTER
    // // Wire.beginTransmission(I2C_SLAVE_ADDRESS);
    // // Wire.write(0x07);
    // // Wire.endTransmission();
// #endif
// #if !I2C_MASTER
    // // if (i2c_received_num > 0) {
        // // Wire.beginTransmission(I2C_MASTER_ADDRESS);
        // // Wire.write(i2c_received_num+1);
        // // Wire.endTransmission();
        // // i2c_received_num = 0;
    // // }
// #endif


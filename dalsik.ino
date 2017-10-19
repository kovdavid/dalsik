#include "keyboard.h"
#include "keymap.h"
#include "master_report.h"
#include "slave_report.h"
#include <Wire.h>

#define ONOFF_OUT_PIN 9
#define ONOFF_IN_PIN 8

unsigned long prev_loop_msec = 0;
Keyboard keyboard;
KeyMap keymap;

#if I2C_MASTER
MasterReport master_report(&keymap);
#else
SlaveReport slave_report;
#endif

uint8_t I2C_read_from_slave = 0;

void setup() {
    pinMode(ONOFF_IN_PIN, INPUT_PULLUP);
    pinMode(ONOFF_OUT_PIN, OUTPUT);
    digitalWrite(ONOFF_OUT_PIN, LOW);

#if I2C_MASTER
    I2C_master_init();
#else
    I2C_slave_init();
#endif

#if I2C_MASTER
    Serial.begin(9600);
#endif

    delay(300);
}

inline void I2C_master_init() {
    Wire.begin(I2C_MASTER_ADDRESS);
    Wire.onReceive(I2C_receive_event);
}

inline void I2C_slave_init() {
    Wire.begin(I2C_SLAVE_ADDRESS);
}

void loop() {
    // Turn off the whole keyboard with a switch
    while (digitalRead(ONOFF_IN_PIN) == HIGH) {
        delayMicroseconds(500);
    }

#if I2C_MASTER
    if (Serial.available() > 0) {
        process_serial_command(&keyboard, &keymap);
    }
    if (I2C_read_from_slave) {
        I2C_read_from_slave = 0;
        read_changed_key_from_slave();
    }
#endif

    unsigned long now_msec = millis();
    if ((now_msec - prev_loop_msec) > 1) {
        prev_loop_msec = now_msec;

        ChangedKeyCoords coords = keyboard.matrix_scan();
#if I2C_MASTER
        master_report.handle_changed_key(coords);
#else
        slave_report.handle_changed_key(coords);
#endif
    } else {
        delayMicroseconds(500);
    }
}

void read_changed_key_from_slave() {
    uint8_t buffer[3] = { 0 };
    uint8_t buffer_index = 0;

    while (Wire.available() > 0) {
        buffer[buffer_index++] = Wire.read();

        if (buffer_index == 3) {
            ChangedKeyCoords coords = { buffer[0], buffer[1], buffer[2] };
            master_report.handle_changed_key(coords);

            buffer_index = 0;
            buffer[0] = 0;
            buffer[1] = 0;
            buffer[2] = 0;
        }
    }
}

void I2C_receive_event(int count) {
    I2C_read_from_slave = 1;
#if DEBUG
    Serial.print("I2C_receive_event:");
    Serial.print(count);
    Serial.print("\n");
#endif
}
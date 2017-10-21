#include "keyboard.h"
#include "keymap.h"
#include "master_report.h"
#include "slave_report.h"
#include <Wire.h>
#include <avr/io.h>

Keyboard keyboard;
KeyMap keymap;

#if I2C_MASTER
MasterReport master_report(&keymap);
#else
SlaveReport slave_report;
#endif

uint8_t I2C_read_from_slave = 0;

void setup() {
    // Disable JTAG, so we can use PORTF
    MCUCR |= _BV(JTD);
    MCUCR |= _BV(JTD);

#if ON_OFF_PIN
    pinMode(ON_OFF_PIN, INPUT_PULLUP);
#endif

#if I2C_MASTER
    Wire.begin(I2C_MASTER_ADDRESS);
    Wire.onReceive(I2C_receive_event);
    Serial.begin(115200);
#else
    Wire.begin(I2C_SLAVE_ADDRESS);
#endif
    // Wire.setClock(400000);

    while (!Serial); // USB connect

    delay(300);
}

void loop() {
#if ON_OFF_PIN
    // Turn off the whole keyboard with a switch
    while (digitalRead(ON_OFF_PIN) == LOW) {
        delayMicroseconds(500);
    }
#endif

#if I2C_MASTER
    if (Serial.available() > 0) {
        process_serial_command(&keyboard, &keymap);
    }
    if (I2C_read_from_slave) {
        I2C_read_from_slave = 0;
        read_changed_key_from_slave();
    }
#endif

    ChangedKeyCoords coords = keyboard.matrix_scan();

#if DEBUG
    if (coords.type != EVENT_NONE) {
        unsigned long now = millis();
        Serial.print("keyevent:");
        Serial.print(coords.type, HEX);
        Serial.print(" now:");
        Serial.println(now);
    }
#endif

#if I2C_MASTER
    master_report.handle_changed_key(coords);
#else
    slave_report.handle_changed_key(coords);
#endif

    delayMicroseconds(100);
}

#if I2C_MASTER
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
#endif

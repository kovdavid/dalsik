#include "keyboard.h"
#include "keymap.h"
#include "master_report.h"
#include "slave_report.h"
#include <Wire.h>
#include <avr/io.h>

Keyboard keyboard;
KeyMap keymap;

#if IS_MASTER
uint8_t slave_buffer[4] = { 0 };
uint8_t slave_buffer_index = 0;
MasterReport master_report(&keymap);
#else
SlaveReport slave_report;
#endif

uint8_t read_from_slave = 0;

void setup() {
    // Disable JTAG, so we can use PORTF
    MCUCR |= _BV(JTD);
    MCUCR |= _BV(JTD);

#if ON_OFF_PIN
    pinMode(ON_OFF_PIN, INPUT_PULLUP);
#endif

#if USE_I2C
    #if IS_MASTER
        Wire.begin(I2C_MASTER_ADDRESS);
        Wire.onReceive(I2C_receive_event);
    #else
        Wire.begin(I2C_SLAVE_ADDRESS);
    #endif
#else
    Serial1.begin(57600);
    while (!Serial1);
#endif
    // Wire.setClock(400000);

#if IS_MASERT || DEBUG
    Serial.begin(115200);
    while (!Serial);
#endif

    delay(300);
}

void loop() {
#if ON_OFF_PIN
    // Turn off the whole keyboard with a switch
    while (digitalRead(ON_OFF_PIN) == LOW) {
        delayMicroseconds(500);
        Serial.println("ON_OFF_PIN OFF");
    }
#endif

#if IS_MASTER
    if (Serial.available() > 0) {
        process_serial_command(&keyboard, &keymap);
    }
    #if USE_I2C
    if (read_from_slave) {
        read_from_slave = 0;
        read_changed_key_from_slave();
    }
    #else
    if (Serial1.available() > 0) {
        read_changed_key_from_slave();
    }
    #endif
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

#if IS_MASTER
    master_report.handle_changed_key(coords);
#else
    slave_report.handle_changed_key(coords);
#endif

    delayMicroseconds(100);
}

#if IS_MASTER
void read_changed_key_from_slave() {

    #if USE_I2C
    Stream* slave_stream = &Wire;
    #else
    Stream* slave_stream = &Serial1;
    #endif

    while (slave_stream->available() > 0) {
        slave_buffer[slave_buffer_index++] = slave_stream->read();

        if (slave_buffer_index == 4) {
            uint8_t calc_checksum = slave_buffer[0] + slave_buffer[1] + slave_buffer[2];
            uint8_t checksum = slave_buffer[3];
            if (calc_checksum == checksum) {
                ChangedKeyCoords coords = { slave_buffer[0], slave_buffer[1], slave_buffer[2] };
                master_report.handle_changed_key(coords);
            }

            slave_buffer_index = 0;
            slave_buffer[0] = 0;
            slave_buffer[1] = 0;
            slave_buffer[2] = 0;
            slave_buffer[3] = 0;
        }
    }
}

void I2C_receive_event(int count) {
    read_from_slave = 1;
}
#endif

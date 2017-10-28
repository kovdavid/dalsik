#include "keyboard.h"
#include "keymap.h"
#include "master_report.h"
#include "slave_report.h"
#include "serial.h"
#include <Wire.h>
#include <avr/io.h>

Keyboard keyboard;
KeyMap keymap;

#if IS_MASTER
MasterReport master_report(&keymap);
#else
SlaveReport slave_report;
#endif

uint8_t read_from_slave = 0;
unsigned long prev_millis = 0;

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
    // Wire.setClock(400000);
#else
    #if IS_MASTER
        serial_master_init();
    #else
        serial_slave_init();
    #endif
#endif

#if IS_MASTER || DEBUG
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
    if (slave_data != 0x00) {
        handle_slave_data(slave_data);
        slave_data = 0x00;
    }
    #endif
#endif

    // By making keyboard scanning only once every millisecond, we can make
    // the time-dependent debounce logic more predictable inside matrix_scan().
    unsigned long millis_now = millis();
    if (millis_now == prev_millis) {
        return;
    }
    prev_millis = millis_now;

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
    #if MASTER_SIDE == MASTER_SIDE_RIGHT
        coords.col += ONE_SIDE_COL_PIN_COUNT;
    #endif
    master_report.handle_changed_key(coords);
#else
    slave_report.handle_changed_key(coords);
#endif
}

#if IS_MASTER && USE_I2C
void read_changed_key_from_slave() {
    while (Wire.available() > 0) {
        handle_slave_data(Wire.read());
    }
}

void I2C_receive_event(int count) {
    read_from_slave = 1;
}
#endif

// The keymap is on the master, so the two splits should act as one keyboard
// Therefore depending on the MASTER_SIDE we need to adjust the col value
// Essentially the right half should add ONE_SIDE_COL_PIN_COUNT to col, because column numbering
// goes from left to right, no matter which side is the master (master is connected via USB)
inline void handle_slave_data(uint8_t data) {
    ChangedKeyCoords coords = decode_slave_report_data(data);

#if MASTER_SIDE == MASTER_SIDE_LEFT
    coords.col += ONE_SIDE_COL_PIN_COUNT;
#endif

    master_report.handle_changed_key(coords);
}

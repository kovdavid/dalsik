#include "matrix.h"
#include "keymap.h"
#include "master_report.h"
#include "slave_report.h"
#include "dalsik_serial.h"
#include "serialcommand.h"
#include <avr/io.h>
#if USE_I2C
    #include <Wire.h>
#endif

Matrix matrix;
KeyMap keymap;

#if IS_MASTER
MasterReport master_report(&keymap);
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
        DalsikSerial::master_init();
    #else
        DalsikSerial::slave_init();
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
        SerialCommand::process_command(&keymap);
    }
    #if USE_I2C
    if (read_from_slave != 0) {
        read_from_slave = 0;
        read_changed_key_from_slave();
    }
    #else
    if (DalsikSerial::slave_data_available != 0) {
        handle_slave_data(DalsikSerial::slave_data);
        DalsikSerial::slave_data_available = 0;
    }
    #endif
#endif

    // By making matrix scanning only once every millisecond, we can make
    // the time-dependent debounce logic more predictable
    unsigned long millis_now = millis();
    if (millis_now == prev_millis) {
        return;
    }
    prev_millis = millis_now;

    ChangedKeyCoords coords = matrix.scan();

#if DEBUG
    if (coords.type != EVENT_NONE) {
        Serial.print("keyevent:");
        Serial.print(coords.type, HEX);
        Serial.print(" now:");
        Serial.println(millis_now);
    }
#endif

#if IS_MASTER
    master_report.handle_master_changed_key(coords);
#else
    SlaveReport::send_changed_key(coords);
#endif
}

#if IS_MASTER
    #if USE_I2C
    void read_changed_key_from_slave() {
        while (Wire.available() > 0) {
            handle_slave_data(Wire.read());
        }
    }

    void I2C_receive_event(int count) {
        read_from_slave = 1;
    }
    #endif

    inline void handle_slave_data(uint8_t data) {
        ChangedKeyCoords coords = SlaveReport::decode_slave_report_data(data);

#if DEBUG
        Serial.print("\n");
        Serial.print("Handle_slave_data<T:");
        Serial.print(coords.type);
        Serial.print("|R:");
        Serial.print(coords.row);
        Serial.print("|C:");
        Serial.print(coords.col);
        Serial.print(">\n");
#endif

        master_report.handle_slave_changed_key(coords);
    }
#endif

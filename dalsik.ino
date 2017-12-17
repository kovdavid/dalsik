#include "matrix.h"
#include "keymap.h"
#include "master_report.h"
#include "slave_report.h"
#include "dalsik_serial.h"
#include "serial_command.h"
#include "pin_utils.h"
#include <avr/io.h>
#if USE_I2C
    #include <Wire.h>
#endif

Matrix matrix;
KeyMap keymap;

MasterReport master_report(&keymap);

uint8_t is_master = 0;
uint8_t read_from_slave = 0;
unsigned long prev_millis = millis();

uint8_t usb_connected() {
   USBCON |= (1 << OTGPADE); //enables VBUS pad
   delayMicroseconds(5);
   return (USBSTA & (1 << VBUS)); //checks state of VBUS
}

void setup() {
    // Disable JTAG, so we can use PORTF
    MCUCR |= _BV(JTD);
    MCUCR |= _BV(JTD);

    delay(100);
    if (usb_connected()) {
        is_master = 1;
    }
    delay(100);

    Serial.begin(115200);

#if ON_OFF_PIN
    PinUtils::pinmode_input_pullup(ON_OFF_PIN);
#endif

#if USE_I2C
    if (is_master) {
        Wire.begin(I2C_MASTER_ADDRESS);
        Wire.onReceive(I2C_receive_event);
    } else {
        Wire.begin(I2C_SLAVE_ADDRESS);
    }
#else
    if (is_master) {
        DalsikSerial::master_init();
    } else {
        DalsikSerial::slave_init();
    }
#endif

    delay(100);
}

void loop() {
#if ON_OFF_PIN
    // Turn off the whole keyboard with a switch
    while (!PinUtils::read_pin(ON_OFF_PIN));
#endif

    if (is_master) {
        if (Serial.available() > 0) {
            SerialCommand::process_command(&keymap);
        }
        #if USE_I2C
        if (read_from_slave != 0) {
            read_changed_key_from_slave();
            read_from_slave = 0;
        }
        #else
        if (DalsikSerial::slave_data_available != 0) {
            handle_slave_data(DalsikSerial::slave_data);
            DalsikSerial::slave_data_available = 0;
        }
        #endif
    }

    // By making matrix scanning only once every millisecond, we can make
    // the time-dependent debounce logic more predictable
    if (prev_millis == millis()) {
        return;
    }
    prev_millis = millis();

    master_report.key_timeout_check(); // check once every millisecond

    ChangedKeyCoords coords = matrix.scan();
    if (coords.type == EVENT_NONE) {
        return;
    }

#if DEBUG
    Serial.print("Master ChangedKeyCoords <T:");
    Serial.print(coords.type);
    Serial.print("|R:");
    Serial.print(coords.row);
    Serial.print("|C:");
    Serial.print(coords.col);
    Serial.print(">");
    Serial.print(" now:");
    Serial.println(prev_millis);
#endif

    if (is_master) {
        master_report.handle_master_changed_key(coords);
    } else {
        SlaveReport::send_changed_key(coords);
    }
}

#if USE_I2C
void read_changed_key_from_slave() {
    while (Wire.available() > 0) {
        handle_slave_data(Wire.read());
    }
}

void I2C_receive_event(int count) {
    void(count); // Supress unused parameter warning
    read_from_slave = 1;
}
#endif

inline void handle_slave_data(uint8_t data) {
    ChangedKeyCoords coords = SlaveReport::decode_slave_report_data(data);

#if DEBUG
    Serial.print("Slave ChangedKeyCoords <T:");
    Serial.print(coords.type);
    Serial.print("|R:");
    Serial.print(coords.row);
    Serial.print("|C:");
    Serial.print(coords.col);
    Serial.print(">");
    Serial.print(" now:");
    Serial.println(prev_millis);
#endif

    master_report.handle_slave_changed_key(coords);
}

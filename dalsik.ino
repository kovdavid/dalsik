#include "matrix.h"
#include "keymap.h"
#include "master_report.h"
#include "slave_report.h"
#include "dalsik_serial.h"
#include "serial_command.h"
#include "pin_utils.h"
#include <avr/io.h>

Matrix matrix;
KeyMap keymap;

MasterReport master_report(&keymap);

uint8_t is_master = 0;
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

    if (is_master) {
        DalsikSerial::master_init();
    } else {
        DalsikSerial::slave_init();
    }

    delay(100);
}

void loop() {
#if ON_OFF_PIN
    while (!PinUtils::read_pin(ON_OFF_PIN));
#endif

    if (is_master) {
        if (Serial.available() > 0) {
            SerialCommand::process_command(&keymap);
        }
        if (DalsikSerial::slave_data_available != 0) {
            handle_slave_data(DalsikSerial::slave_data);
            DalsikSerial::slave_data_available = 0;
        }
    }

    // By making matrix scanning only once every millisecond, we can make
    // the time-dependent debounce logic more predictable
    unsigned long now_ms = millis();
    if (prev_millis == now_ms) {
        return;
    }
    prev_millis = now_ms;

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

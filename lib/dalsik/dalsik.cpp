#include <Arduino.h>
#include "matrix.h"
#include "keymap.h"
#include "master_report.h"
#include "slave_report.h"
#include "dalsik_serial.h"
#include "serial_command.h"
#include "dalsik.h"
#include "pin_utils.h"
#include <avr/io.h>

Matrix matrix;
KeyMap keymap;

MasterReport master_report(&keymap);

uint8_t is_master = 0;
millis prev_millis = millis();

static inline void handle_slave_data(uint8_t data) {
    ChangedKeyEvent event = SlaveReport::decode_slave_report_data(data);

#if DEBUG
    Serial.print("Slave ChangedKeyEvent <");
    if (event.type == EVENT_KEY_PRESS) {
        Serial.print("PRE");
    } else {
        Serial.print("REL");
    }
    Serial.print("|ROW:");
    Serial.print(event.coords.row);
    Serial.print("|COL:");
    Serial.print(event.coords.col);
    Serial.print(">");
    Serial.print(" now:");
    Serial.print(prev_millis);
    Serial.print("\n");
#endif

    master_report.handle_slave_changed_key(event);
}

uint8_t usb_connected() {
   USBCON |= (1 << OTGPADE); //enables VBUS pad
   delayMicroseconds(5);
   return (USBSTA & (1 << VBUS)); //checks state of VBUS
}

void Dalsik::setup() {
    // Disable JTAG, so we can use PORTF
    MCUCR |= _BV(JTD);
    MCUCR |= _BV(JTD);

    delay(100);
    if (usb_connected()) {
        is_master = 1;
    }
    delay(100);

    Serial.begin(115200);

#ifdef ON_OFF_PIN
    PinUtils::pinmode_input_pullup(ON_OFF_PIN);
#endif
#ifdef LED_PIN
    PinUtils::pinmode_output_low(LED_PIN);
#endif

    if (is_master) {
        DalsikSerial::master_init();
    } else {
        DalsikSerial::slave_init();
    }

    TXLED0;
    RXLED0;

    delay(100);
}

void Dalsik::loop() {
#if ON_OFF_PIN
    while (!PinUtils::read_pin(ON_OFF_PIN));
#endif

    if (is_master) {
        if (Serial.available() > 0) {
            SerialCommand::process_command(&keymap);
        }

        while (DalsikSerial::has_data()) {
            uint8_t slave_data = DalsikSerial::get_next_elem();
            handle_slave_data(slave_data);
        }
    }

    // By making matrix scanning only once every millisecond, we can make
    // the time-dependent debounce logic more predictable
    millis now_ms = millis();
    if (prev_millis == now_ms) {
        return;
    }
    prev_millis = now_ms;

    master_report.key_timeout_check(); // check once every millisecond

    ChangedKeyEvent event = matrix.scan();
    if (event.type == EVENT_NONE) {
        return;
    }

#if DEBUG
    Serial.print("Master ChangedKeyEvent <");
    if (event.type == EVENT_KEY_PRESS) {
        Serial.print("PRE");
    } else {
        Serial.print("REL");
    }
    Serial.print("|ROW:");
    Serial.print(event.coords.row);
    Serial.print("|COL:");
    Serial.print(cevent.oords.col);
    Serial.print(">");
    Serial.print(" now:");
    Serial.print(prev_millis);
    Serial.print("\n");
#endif

    if (is_master) {
        master_report.handle_master_changed_key(event);
    } else {
        SlaveReport::send_changed_key(event);
    }
}

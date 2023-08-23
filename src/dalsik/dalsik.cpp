#include "Arduino.h"
#include "HID.h"

#include "matrix.h"
#include "keyboard.h"
#include "key_event_handler.h"
#include "dalsik_serial.h"
#include "dalsik_hid_descriptor.h"
#include "serial_command.h"
#include "dalsik.h"
#include "pin_utils.h"
#include <avr/io.h>

Matrix matrix;
KeyEventHandler key_event_handler;

bool is_master = false;
millisec prev_millis = 0;

uint8_t usb_connected() {
   USBCON |= (1 << OTGPADE); //enables VBUS pad
   delayMicroseconds(5);
   return (USBSTA & (1 << VBUS)); //checks state of VBUS
}

void Dalsik::setup() {
    static HIDSubDescriptor node(KEYBOARD_HID_DESCRIPTOR, sizeof(KEYBOARD_HID_DESCRIPTOR));
    HID().AppendDescriptor(&node);

    // Disable JTAG, so we can use PORTF
    MCUCR |= _BV(JTD);
    MCUCR |= _BV(JTD);

    delay(100);
    if (usb_connected()) {
        is_master = true;
    }
    delay(100);

    Serial.begin(115200);

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
    millisec now = millis();

    if (is_master) {
        if (Serial.available() > 0) {
            SerialCommand::process_command();
        }

        while (DalsikSerial::has_data()) {
            uint8_t slave_data = DalsikSerial::get_next_elem();
            key_event_handler.handle_received_data_from_slave(slave_data, now);
        }
    }

    // By making matrix scanning only once every millisecond, we can make
    // the time-dependent debounce logic more predictable
    if (prev_millis == now) {
        return;
    }
    prev_millis = now;

    key_event_handler.handle_timeout(now); // check once every millisecond

    BaseKeyEvent event = matrix.scan();
    if (event.type == EVENT_NONE) {
        return;
    }

    if (is_master) {
        key_event_handler.handle_key_event_from_master(event, now);
    } else {
        key_event_handler.send_slave_event_to_master(event);
    }
}

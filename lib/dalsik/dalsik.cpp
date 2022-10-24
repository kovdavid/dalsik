#include <Arduino.h>
#include "matrix.h"
#include "keyboard.h"
#include "key_event_handler.h"
#include "dalsik_serial.h"
#include "serial_command.h"
#include "dalsik.h"
#include "pin_utils.h"
#include <avr/io.h>

Matrix matrix;

Keyboard keyboard;
KeyEventHandler key_event_handler(&keyboard);

uint8_t is_master = 0;
millisec prev_millis = 0;

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

    keyboard.key_timeout_check(now); // check once every millisecond

    ChangedKeyEvent event = matrix.scan();
    if (event.type == EVENT_NONE) {
        return;
    }

    if (is_master) {
        key_event_handler.handle_key_event_from_master(event, now);
    } else {
        key_event_handler.send_slave_event_to_master(event);
    }
}

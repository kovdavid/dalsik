#include <Arduino.h>
#include <HID.h>
#include <stdint.h>
#include <string.h>
#include "array_utils.h"
#include "dalsik.h"
#include "dalsik_eeprom.h"
#include "dalsik_global.h"
#include "dalsik_hid.h"
#include "dalsik_led.h"
#include "dalsik_serial.h"
#include "key_definitions.h"
#include "key_event_handler.h"
#include "key_info.h"
#include "keymap.h"
#include "keyboard.h"
#include "matrix.h"
#include "pin_utils.h"
#include "ring_buffer.h"
#include "serial_command.h"

void setup() {
    Dalsik::setup();
}

void loop() {
    Dalsik::loop();
}

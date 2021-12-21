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
#include "keymap.h"
#include "master_report.h"
#include "matrix.h"
#include "pin_utils.h"
#include "ring_buffer.h"
#include "serial_command.h"
#include "slave_report.h"

void setup() {
    Dalsik::setup();
}

void loop() {
    Dalsik::loop();
}

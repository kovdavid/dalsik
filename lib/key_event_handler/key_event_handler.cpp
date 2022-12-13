#include "dalsik.h"
#include "dalsik_eeprom.h"
#include "dalsik_serial.h"
#include "matrix.h"
#include "key_event_handler.h"
#include "combos_handler.h"

inline uint8_t parity(uint8_t d) {
    d ^= (d >> 4);
    d ^= (d >> 2);
    d ^= (d >> 1);
    return d & 0x1;
}

KeyEventHandler::KeyEventHandler():
    keyboard(),
    combos_handler(&this->keyboard),
    keyboard_side(EEPROM::get_keyboard_side())
{
}

void KeyEventHandler::send_slave_event_to_master(ChangedKeyEvent event) {
    if (event.type == EVENT_NONE) {
        return;
    }

    uint8_t slave_data = this->encode_slave_event(event);
    DalsikSerial::slave_send(slave_data);
}

// We use 1B to send type, row and col from ChangedKeyEvent + 2 parity
// ABBCCCDE
// A - EVENT_KEY_PRESS:1 EVENT_KEY_RELEASE:0 (EVENT_NONE is not sent by slave)
// B - row
// C - column
// D - parity of A+B+C (bitmask 1111_1100 - FC)
// E - parity of B+C (bitmask 0111_1100 - 7C)
uint8_t KeyEventHandler::encode_slave_event(ChangedKeyEvent event) {
    uint8_t data = 0x00;

    if (event.type == EVENT_KEY_PRESS) {
        data |= 0x80;
    }
    data |= (event.coords.row << 5) & 0x60;
    data |= (event.coords.col << 2) & 0x1C;

    uint8_t p1 = parity(data & 0xFC);
    uint8_t p2 = parity(data & 0x7C);

    data |= (p1 << 1) & 0x02;
    data |= (p2 << 0) & 0x01;

    return data;
}

ChangedKeyEvent KeyEventHandler::decode_slave_event(uint8_t data) {
    ChangedKeyEvent event;

    event.type = (data & 0x80) ? EVENT_KEY_PRESS : EVENT_KEY_RELEASE;
    event.coords.row = (data >> 5) & 0x03;
    event.coords.col = (data >> 2) & 0x07;

    uint8_t p1 = (data >> 1) & 0x01;
    uint8_t p2 = (data >> 0) & 0x01;

    uint8_t calc_p1 = parity(data & 0xFC);
    uint8_t calc_p2 = parity(data & 0x7C);

    if (p1 == calc_p1 && p2 == calc_p2) {
        return event;
    } else {
        return ChangedKeyEvent { EVENT_NONE, KeyCoords { 0, 0 } };
    }
}

void KeyEventHandler::handle_received_data_from_slave(uint8_t data, millisec now) {
    ChangedKeyEvent event = this->decode_slave_event(data);

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
    Serial.print(now);
    Serial.print("\n");
#endif

    this->process_slave_changed_key(event, now);
}

void KeyEventHandler::handle_key_event_from_master(ChangedKeyEvent event, millisec now) {
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
    Serial.print(event.coords.col);
    Serial.print(">");
    Serial.print(" now:");
    Serial.print(now);
    Serial.print("\n");
#endif

    this->process_master_changed_key(event, now);
}

// The right side PCB of the Let's Split is reversed, so if it sends col 0, it is actually col 5.
// Also, the keyboard is represented in EEPROM as a whole, so the reported col must be incremented
// by ONE_SIDE_COL_PIN_COUNT (col 4 becomes col 10, as there are 6 columns per side)
// The right side sends columns 0-5 and thus we offset it to 6-11
void KeyEventHandler::process_master_changed_key(ChangedKeyEvent event, millisec now) {
    if (this->keyboard_side == KEYBOARD_SIDE_RIGHT) {
        event.coords.col = 2*ONE_SIDE_COL_PIN_COUNT - event.coords.col - 1;
    }
    this->handle_key_event(event, now);
}

// KeyMap is used only on the master side; The slave is the right side if the master is the left
void KeyEventHandler::process_slave_changed_key(ChangedKeyEvent event, millisec now) {
    if (this->keyboard_side == KEYBOARD_SIDE_LEFT) {
        event.coords.col = 2*ONE_SIDE_COL_PIN_COUNT - event.coords.col - 1;
    }
    this->handle_key_event(event, now);
}

void KeyEventHandler::handle_timeout(millisec now) {
    this->handle_key_event(ChangedKeyEvent { EVENT_TIMEOUT, COORDS_INVALID }, now);
}

inline void KeyEventHandler::handle_key_event(ChangedKeyEvent event, millisec now) {
#if COMBOS_ENABLED
    this->combos_handler.handle_key_event(event, now);
#else
    this->keyboard.handle_key_event(event, now);
#endif
}

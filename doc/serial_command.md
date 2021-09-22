# SerialCommand

* [serial_command.h](https://github.com/DavsX/dalsik/blob/master/serial_command.h)
* [serial_command.ino](https://github.com/DavsX/dalsik/blob/master/serial_command.ino)

The `SerialCommand` module is used to handle commands via the USB cable on the master side:

```c++
// dalsik.ino
KeyMap keymap;

void loop() {
    ...

    if (is_master) {
        if (Serial.available() > 0) {
            SerialCommand::process_command(&keymap);
        }
    }

    ...
}
```

Each command has a certain prefix to avoid processing "random noise":

```c++
// serial_command.h
static const char CMD_PREFIX[] = {'D','A','L','S','I','K','-'};

// +1 for the command type
// +5 is the max size of arguments (for SET_KEY)
// I can't use '\n' as the indicator of the end of a command, because that is
// the HID code for KC_G, so I made every command fixed-length
#define CMD_LENGTH sizeof(CMD_PREFIX)+1+5
```

Each command has a fixed size. I have to detect the end of a command somehow.. Using `\n` as the command separator does not work, as it is the same, as the HID code of `KC_G`. At first I was using `\n`, but the I could not configure the `g` key.

Each byte of the command is put into a static array:

```c++
// serial_command.ino
char cmd_buffer[CMD_LENGTH] = {0};
uint8_t cmd_buffer_index = 0;

void SerialCommand::process_command(KeyMap* keymap) {
    char c = Serial.read();
    cmd_buffer[cmd_buffer_index++] = c;

    if (cmd_buffer_index == CMD_LENGTH) {
        uint8_t res = execute_command(keymap);
        if (res) {
            Serial.print(F("CMD_ERROR "));
            Serial.print(res);
            Serial.print(F("\n"));
        } else {
            Serial.println(F("CMD_OK"));
        }
        memset(&cmd_buffer, 0, sizeof(char)*CMD_LENGTH);
        cmd_buffer_index = 0;
    }
}
```

In `execute_command` I first check if our `CMD_PREFIX` is present in the `cmd_buffer`:

```c++
// serial_command.ino
if (memcmp(cmd_buffer, CMD_PREFIX, sizeof(CMD_PREFIX)) != 0) {
    return 1; // Invalid command
}
```

Then based of the first byte after `CMD_PREFIX` a certain code is executed. For example setting a KeyInfo struct for a given row/column/layer works like this:

```c++
// serial_command.ino

char* buffer = &(cmd_buffer[sizeof(CMD_PREFIX)]);

if (buffer[0] == CMD_SET_KEY) {
    uint8_t layer    = buffer[1];
    uint8_t row      = buffer[2];
    uint8_t col      = buffer[3];
    uint8_t key_type = buffer[4];
    uint8_t key      = buffer[5];

    KeyInfo ki = KeyMap::init_key_info(key_type, key, row, col);
    keymap->set_key(layer, ki);
    
    return 0;
}
```

The module `KeyMap` is used to write 2 bytes corresponding to the given KeyInfo struct at the correct address:

```c++
void KeyMap::set_key(uint8_t layer, KeyInfo key_info) {
    uint8_t row = key_info.row;
    uint8_t col = key_info.col;

    if (row == ROW_UNKNOWN || col == COL_UNKNOWN) {
        return; // Invalid KeyInfo
    }

    int eeprom_address = this->get_eeprom_address(layer, row, col);

    EEPROM.update(eeprom_address + 0x00, key_info.type);
    EEPROM.update(eeprom_address + 0x01, key_info.key);
}
```


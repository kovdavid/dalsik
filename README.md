# DALSIK

**Da**vs **L**et's **S**pl**i**t **K**eyboard firmware for atmega32u4

### Why
Just for fun :) I made this to see if I can do it and to learn. The list of features is kept to minimal, so the code would be simple:

* Update the keymap without reflashing the atmega32u4 (keymap is kept in EEPROM)
* Layers
* DualKey - tap for standard key, hold with other key for modifier (like CTRL+ESC on a single key)
* Designed for the [Let's Split](https://github.com/nicinabox/lets-split-guide) keyboard by [wootpatoot](https://www.reddit.com/user/wootpatoot)
* Split keyboard support via I2C or single-wire serial

The slave is only reporting which row/column was pressed/released (1B/change). The actual key changes are determined on the master side. The advantage of this, that there is no need for 2-way communication between the master and slave (for example setting the layer).

### Flashing
To flash the atmega32u4, use the [Arduino IDE](https://www.arduino.cc/en/main/software). Set `IS_MASTER` to 1 in `dalsik.h` to flash the master and to 0 to flash the slave microcontroller.
Depending on which half you want to connect via USB, set the `MASTER_SIDE` value in `dalsik.h` to `MASTER_SIDE_LEFT` or `MASTER_SIDE_RIGHT`.

### Inspired by
* [Animus firmware](https://github.com/blahlicus/animus-family)
* [QMK](https://github.com/qmk/qmk_firmware)

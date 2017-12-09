# DALSIK

**Da**vs **L**et's **S**pl**i**t **K**eyboard firmware for atmega32u4

### 1. Motivation
Fun :) I made this to see if I can do it and to learn. The list of features is kept to minimal, so the code should be fairly simple (around 1800 lines according to cloc).

### 2. Features
* Designed for the [Let's Split](https://github.com/nicinabox/lets-split-guide) keyboard by [wootpatoot](https://www.reddit.com/user/wootpatoot)
* Up to 8 layers (press, toggle and hold-or-toggle key types)
* Update your keymap without reflashing (the keymap is kept in EEPROM)
* DualKey - tap for standard key, hold with other key for modifier (e.g. CTRL+ESC on a single key)
* TapDance - registers as different key based on the number of taps within short time
* Multimedia and system (power-off, sleep, wake-up) keys
* Split keyboard support via I2C or single-wire serial

### 3. Flashing
To flash the atmega32u4, use the [Arduino IDE](https://www.arduino.cc/en/main/software). Set `IS_MASTER` to 1 in `dalsik.h` to flash the master and to 0 to flash the slave microcontroller.
Depending on which half you want to connect via USB, set the `MASTER_SIDE` value in `dalsik.h` to `MASTER_SIDE_LEFT` or `MASTER_SIDE_RIGHT`.

### 4. Inspired by
* [Animus firmware](https://github.com/blahlicus/animus-family)
* [QMK](https://github.com/qmk/qmk_firmware)

### 5. Code architecture
Everything starts in the [dalsik.ino](https://github.com/DavsX/dalsik/blob/master/dalsik.ino) file. It contains the `void setup()` function, which initializes the hardware and the `void loop()` function, which is responsible for managing all the work that the keyboard does. Each side of the keyboard scans it's own set of keys for any change (key press or key release). This scanning is done with the [`matrix.scan()` call](#51-the-matrix-module).

### 5.1 The Matrix module

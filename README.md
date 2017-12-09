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
Everything starts in the [dalsik.ino](https://github.com/DavsX/dalsik/blob/master/dalsik.ino) file. It contains the `void setup()` function, which initializes the hardware and the `void loop()` function, which is responsible for managing all the work that the keyboard does. Each side of the keyboard scans it's own set of keys for any change (key press or key release). This scanning is done with the [`matrix.scan()`](#51-the-matrix-module) call.

### 5.1 The Matrix module
The job of this module is to detect and report any key change event (press or release). The keys are wired into a [grid](http://pcbheaven.com/wikipages/How_Key_Matrices_Works/). The `scan` function iterates over every row and for every row check every column, this scanning for changes. The debouncing method was inspired from a solution written by [Kenneth A. Kuhn](http://www.kennethkuhn.com/electronics/debounce.c). The `scan` function returns a [`ChangedKeyCoords`](https://github.com/DavsX/dalsik/blob/master/matrix.h) structure, which contains the row and column numbers of a changed key and the type of the change (press or release).

### 5.2 Master and slave differences
Both the master and slave microcontrollers runs the `matrix.scan()` function on their own key matrix. The only thing the slave microcontroller does is to send the key change event do the master. Every keymap functionality and USB communication is done by the master. The slave does not know about any keys. It knows only about rows, columns, keypresses and keyreleases. The key change event is sent from the slave to the master using the [`DalsikSerial module`](#53-the-dalsikserial-module).

### 5.3 The DalsikSerial module
The [DalsikSerial module](https://github.com/DavsX/dalsik/blob/master/dalsik_serial.ino) implements a single-wire, one-way, asynchronous serial communication protocol inspired by [QMK](https://github.com/qmk/qmk_firmware/blob/master/keyboards/lets_split/serial.c). The sender is always the slave half, while the master half only receives data. The line is kept on HIGH during idle. Before sending any data, the slave half pulls the line LOW, so the falling-edge pin interrupt on the master would trigger. After the stop bits the slave sends one byte, which the master half receives. The one byte sent by the slave contains the event type (1 bit - press or release), the row number (the Let's Split has 4 rows - that is 2 bits) and the column number (6 columns - that is 3 bits). This adds up to 6 bits. The last two are used for parity.

### 5.4 Key event overview
The translation from `ChangedKeyCoords` to actual keys (and eventually sending data to the PC) is done in the [MasterReport](https://github.com/DavsX/dalsik/blob/master/master_report.ino) module. It's two main functions are the `handle_master_changed_key` and the `handle_slave_changed_key`. The high-level overview of the keyboard goes like this:
* We scan the key-matrix once every millisecond, looking for key changes
* The key change events detected on the slave half are sent to the master half, where it is processed in the `handle_slave_changed_key` function.
* The key change events on the master half are handled in the `handle_master_changed_key` function.

### 5.5 The KeyMap module
The main responsibility of the `KeyMap` module is to translate key coordinates (row+column+layer number) into actual keys - see the [`KeyInfo` structure](https://github.com/DavsX/dalsik/blob/master/keymap.h). This module serves as an interface between the keyboard and the EEPROM, where the whole keymap is kept.

### 5.6 The MasterReport module
In the end, every key handling is done in `MasterReport`. It's main responsibility is to translate the `ChangedKeyCoords` event into actual keys (using the `KeyMap` module) and to maintain/send the [USB HID report](https://docs.mbed.com/docs/ble-hid/en/latest/api/md_doc_HID.html) structure. There is a separate function for handling a press and release event for every type of key. The HID keyreports are sent by the `send_hid_report()` function. There are currently 3 different HID reports: base keyboard, multimedia and system keyreport. The purpose of the `press_*` and `release_*` functions is to add/remove keys to/from these structures.

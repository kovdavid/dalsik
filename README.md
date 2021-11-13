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
* Split keyboard support single-wire serial
* Automatic master/slave election (based on the USB cable)

### 3. Flashing
To flash the atmega32u4, use the [Arduino IDE](https://www.arduino.cc/en/main/software).

### 4. Inspired by
* [Animus firmware](https://github.com/blahlicus/animus-family)
* [QMK](https://github.com/qmk/qmk_firmware)

### 5. Setting the keymap
There are various Perl scripts in the `utils/` directory for communicating the the keyboard. One of these commands is the `SET_KEY` command, which sets the key for a specific row/column/layer into the EEPROM. The main utility is the `utils/set_keymap.pl` script, which sets the whole keymap from a `json` file.

The master side is automatically detected, when the atmega32u4 gets power from the USB.

Example usage: `perl utils/set_keymap.pl -j /path/to/keymap.json [-s /dev/ttyACM0]`

### 5.1 Key types
Every supported key type is in [key_definitions.h](https://github.com/DavsX/dalsik/blob/master/key_definitions.h). The `KeyInfo` struct mirrors the keymap representation in the EEPROM, having 2 bytes: key type and key. KEY_UNSET and KEY_TRANSPARENT does not take 'key' argument (it is set to 0x00). The following key types are supported:

* KEY_UNSET - Has no effect
* KEY_NORMAL - Basic keys, like modifiers (CTRL,ALT,SHIFT,GUI), letters, numbers etc.
* KEY_DUAL_MOD_LCTRL - Left CTRL on hold, other KEY_NORMAL on tap
* KEY_DUAL_MOD_RCTRL - Right CTRL on hold, other KEY_NORMAL on tap
* KEY_DUAL_MOD_LSHIFT - Left SHIFT on hold, other KEY_NORMAL on tap
* KEY_DUAL_MOD_RSHIFT - Right SHIFT on hold, other KEY_NORMAL on tap
* KEY_DUAL_MOD_LGUI - Left GUI on hold, other KEY_NORMAL on tap
* KEY_DUAL_MOD_RGUI - Right GUI on hold, other KEY_NORMAL on tap
* KEY_DUAL_MOD_LALT - Left ALT on hold, other KEY_NORMAL on tap
* KEY_DUAL_MOD_RALT - Right ALT on hold, other KEY_NORMAL on tap
* KEY_LAYER_PRESS - Switch to other layer, while the key is pressed
* KEY_LAYER_TOGGLE - Switch to other layer, until this key is pressed again. Other KEY_LAYER_PRESS keys can momentarily change layers, but after those are release, the current layer stays at the toggled layer
* KEY_LAYER_TOGGLE_OR_HOLD - Switch momentarily to other layer if held. Toggle layer on tap
* KEY_WITH_MOD_LCTRL - Press some KEY_NORMAL with Left CTRL
* KEY_WITH_MOD_RCTRL - Press some KEY_NORMAL with Left CTRL
* KEY_WITH_MOD_LSHIFT - Press some KEY_NORMAL with Left SHIFT
* KEY_WITH_MOD_RSHIFT - Press some KEY_NORMAL with Right SHIFT
* KEY_WITH_MOD_LGUI - Press some KEY_NORMAL with Left GUI
* KEY_WITH_MOD_RGUI - Press some KEY_NORMAL with Right GUI
* KEY_WITH_MOD_LALT - Press some KEY_NORMAL with Left ALT
* KEY_WITH_MOD_RALT - Press some KEY_NORMAL with Right ALT
* KEY_SYSTEM - System keys (power-off, sleep, wake-up etc)
* KEY_MULTIMEDIA_0 - Multimedia keys with '0x00' prefix - mute, volume up, volume down etc
* KEY_MULTIMEDIA_1 - Multimedia keys with '0x01' prefix - application launch (calculator, browser) etc
* KEY_MULTIMEDIA_2 - Multimedia keys with '0x02' prefix - application control (Save, Exit, Open) etc.
* KEY_TAPDANCE - Tapdance key - sends one from up to 3 different keys based on number of taps
* KEY_DUAL_LAYER_1 - Switch to layer 1 on hold, other KEY_NORMAL on tap
* KEY_DUAL_LAYER_2 - Switch to layer 2 on hold, other KEY_NORMAL on tap
* KEY_DUAL_LAYER_3 - Switch to layer 3 on hold, other KEY_NORMAL on tap
* KEY_DUAL_LAYER_4 - Switch to layer 4 on hold, other KEY_NORMAL on tap
* KEY_DUAL_LAYER_5 - Switch to layer 5 on hold, other KEY_NORMAL on tap
* KEY_DUAL_LAYER_6 - Switch to layer 6 on hold, other KEY_NORMAL on tap
* KEY_DUAL_LAYER_7 - Switch to layer 7 on hold, other KEY_NORMAL on tap
* KEY_TRANSPARENT - Use key from the previous layer

The [USB HID Usage Tables](http://www.usb.org/developers/hidpage/Hut1_12v2.pdf) document contains every possible key codes used in HID communication.

### 5.2 Keymap JSON format

TODO

### 6. Code architecture
Everything starts in the [dalsik.ino](https://github.com/DavsX/dalsik/blob/master/dalsik.ino) file. It contains the `void setup()` function, which initializes the hardware and the `void loop()` function, which is responsible for managing all the work that the keyboard does. Each side of the keyboard scans it's own set of keys for any change (key press or key release). This scanning is done with the [`matrix.scan()`](#61-the-matrix-module) call.

### 6.1 The Matrix module
The job of this module is to detect and report any key change event (press or release). The keys are wired into a [grid](http://pcbheaven.com/wikipages/How_Key_Matrices_Works/). The `scan` function iterates over every row and for every row check every column, this scanning for changes. The debouncing method was inspired from a solution written by [Kenneth A. Kuhn](http://www.kennethkuhn.com/electronics/debounce.c). The `scan` function returns a [`ChangedKeyCoords`](https://github.com/DavsX/dalsik/blob/master/matrix.h) structure, which contains the row and column numbers of a changed key and the type of the change (press or release).

### 6.2 Master and slave differences
Both the master and slave microcontrollers runs the `matrix.scan()` function on their own key matrix. The only thing the slave microcontroller does is to send the key change event do the master. Every keymap functionality and USB communication is done by the master. The slave does not know about any keys. It knows only about rows, columns, keypresses and keyreleases. The key change event is sent from the slave to the master using the [`DalsikSerial module`](#63-the-dalsikserial-module).

### 6.3 The DalsikSerial module
The [DalsikSerial module](https://github.com/DavsX/dalsik/blob/master/dalsik_serial.ino) implements a single-wire, one-way, asynchronous serial communication protocol inspired by [QMK](https://github.com/qmk/qmk_firmware/blob/master/keyboards/lets_split/serial.c). The sender is always the slave half, while the master half only receives data. The line is kept on HIGH during idle. Before sending any data, the slave half pulls the line LOW, so the falling-edge pin interrupt on the master would trigger. After the stop bits the slave sends one byte, which the master half receives. The one byte sent by the slave contains the event type (1 bit - press or release), the row number (the Let's Split has 4 rows - that is 2 bits) and the column number (6 columns - that is 3 bits). This adds up to 6 bits. The last two are used for parity.

### 6.4 Key event overview
The translation from `ChangedKeyCoords` to actual keys (and eventually sending data to the PC) is done in the [MasterReport](https://github.com/DavsX/dalsik/blob/master/master_report.ino) module. It's two main functions are the `handle_master_changed_key` and the `handle_slave_changed_key`. The high-level overview of the keyboard goes like this:
* We scan the key-matrix once every millisecond, looking for key changes
* The key change events detected on the slave half are sent to the master half, where it is processed in the `handle_slave_changed_key` function.
* The key change events on the master half are handled in the `handle_master_changed_key` function.

### 6.5 The KeyMap module
The main responsibility of the `KeyMap` module is to translate key coordinates (row+column+layer number) into actual keys - see the [`KeyInfo` structure](https://github.com/DavsX/dalsik/blob/master/keymap.h). This module serves as an interface between the keyboard and the EEPROM, where the whole keymap is kept.

### 6.6 The MasterReport module
In the end, every key handling is done in `MasterReport`. It's main responsibility is to translate the `ChangedKeyCoords` event into actual keys (using the `KeyMap` module) and to maintain/send the [USB HID report](https://docs.mbed.com/docs/ble-hid/en/latest/api/md_doc_HID.html) structure. There is a separate function for handling a press and release event for every type of key. The HID keyreports are sent by the `send_hid_report()` function. There are currently 3 different HID reports: base keyboard, multimedia and system keyreport. The purpose of the `press_*` and `release_*` functions is to add/remove keys to/from these structures.

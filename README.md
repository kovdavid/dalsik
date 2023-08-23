# DALSIK

**Da**vs **L**et's **S**pl**i**t **K**eyboard firmware.

### 1. Motivation
I made this as a fun learning project. The code is working and I've been using it on my keyboard since 2017. You can think of this as "QMK lite". As the code is written for only a single keyboard and MCU, it should be easier to understand as QMK.

The code is designed for the [Let's Split](https://github.com/nicinabox/lets-split-guide) keyboard created by [wootpatoot](https://www.reddit.com/user/wootpatoot).

### 2. Inspired by
* [Animus firmware](https://github.com/blahlicus/animus-family)
* [QMK](https://github.com/qmk/qmk_firmware)
* [Arduino-Makefile](https://github.com/leka/Arduino-Makefile)

### 3. Prerequisites

The project can be compiled on Linux, if you have to following packages installed:

* Arduino (for the `HID` and `Serial` libraries and `USB` implementation)
* avr-gcc (for compilation)
* pyserial (for resetting the MCU via USB)
* avrdude (for flashing)

On Arch linux you can run:

```bash
$ pacman -S arduino avr-gcc avrdude python-pyserial
```

### 4. Keymap

You need to specify the location of you keymap file (see `sample_keymap/sample_keymap.cpp`) via the `DALSIK_KEYMAP` env variable. I recommend using [direnv](https://direnv.net/) for this:

```bash
$ cd ~/where/the/dalsik/project/is/cloned
$ cat .envrc
export DALSIK_KEYMAP=/path/to/my/keymap/file.cpp
```

### 5. Compilation & flashing

```bash
# Clean the previous build artifacts (optional)
$ make clean
# Compile the project
$ make verify
# Run the tests
$ make test
# Flash the MCU
$ make upload
```

### 6. Initial flashing

The project assumes that one side of the split keyboard is always on the left and the other is on the right. The sidedness is encoded in EEPROM. To initially set the sidedness of the keyboards, uncomment the corresponding `#define SET_KEYBOARD` line in `src/dalsik/dalsik.h` and do a `clean verify upload` flashing on both sides.

### 7. Master election

Master election is automatic based on the USB connection. When changing the keymap, only the master side needs to be reflashed. Should the USB connector break on the master keyboard, you can connect the USB cable to the other side, reflash it with your current keymap (if necessary) and it will work.

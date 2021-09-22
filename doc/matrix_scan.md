# Matrix scanning

Modules:

* [matrix.h](https://github.com/DavsX/dalsik/blob/master/matrix.h)
* [matrix.ino](https://github.com/DavsX/dalsik/blob/master/matrix.ino)



In the [Keyboard wiring](keyboard_wiring.md) document we saw how each key is wired up to the microcontroller. Scanning the keyboard matrix means checking the state (pressed or released) of each key and looking for changes. This is the main thing each half of the keyboard is doing. Both the master and the slave side are checking only their keys (which are connected to the MCU).

For tracking the state of each key I am using an array `uint8_t keystate[ROW_PIN_COUNT][ONE_SIDE_PIN_COUNT]` in [matrix.h](https://github.com/DavsX/dalsik/blob/master/matrix.h).

## Pin initialization

The pin-to-row mapping is defined in [dalsik.h](https://github.com/DavsX/dalsik/blob/master/dalsik.h) like this:

```c++
// dalsik.h
const uint8_t ROW_PINS[ROW_PIN_COUNT] = {
    PIN_D(7), PIN_E(6), PIN_B(4), PIN_B(5)
};
const uint8_t COL_PINS[ONE_SIDE_COL_PIN_COUNT] = {
    PIN_F(6), PIN_F(7), PIN_B(1), PIN_B(3), PIN_B(2), PIN_B(6)
};
```

Each pin is initialized in the `Matrix::Matrix()` constructor:

```c++
// matrix.ino
for (uint8_t row = 0; row < ROW_PIN_COUNT; row++) {
    PinUtils::pinmode_input_pullup(ROW_PINS[row]);
}
for (uint8_t col = 0; col < ONE_SIDE_COL_PIN_COUNT; col++) {
    PinUtils::pinmode_input_pullup(COL_PINS[col]);
}
```

Why are the row pins initialized as input_pullup instead of output_high? Why are they then switched to output_low and back to input_pullup? According to [arduino.cc](https://www.arduino.cc/en/Tutorial/Foundations/DigitalPins) it is safer to keep those pins idle as input_pullup, because they provide less current, than output_high (less use of power).

## Scanning

<img src="keyboard_matrix.png" alt="keyboard_matrix" style="zoom: 50%;" />

Scanning is implemented in `Matrix::scan()`. In it we loop through each row and we check each column for the given rows. The pins are all set to the _high_ signal (pull-up) - so reading from each column pin for released keys yields `1`. 

We set each row pin to _low_ signal (`PinUtils::pinmode_output_low`) and for each row we read each column pin one by one.

* If the key for that row and column is not pressed, then we read `1` from that column pin - there is no electric connection between the row pin and the column pin.
* If the key for that row and column is pressed, then the pull-up _high_ signal of the column's pin is connected to the ground (to the row's pin) and because of that we read a _low_ signal.

After checking each column we set the row's pin back to `pinmode_input_pullup` and proceed to the next row.

The `Matrix::scan()` function returns the first detected key change using the `ChangedKeyCoords` struct:

```c++
// matrix.h
typedef struct {
    uint8_t type;
    uint8_t row;
    uint8_t col;
} ChangedKeyCoords;
```

The type of the change can be:

```c++
// matrix.h
#define EVENT_NONE 0x00
#define EVENT_KEY_PRESS 0x01
#define EVENT_KEY_RELEASE 0x02
```

The column's state is read by inverting the signal on the pin:

```c++
// matrix.ino
uint8_t input = !PinUtils::read_pin(COL_PINS[col]);
uint8_t debounced_input = this->debounce_input(row, col, input);

if (debounced_input == DEBOUNCE_CHANGING) {
    continue; // Wait, till the value stabilizes
}

if (debounced_input != this->keystate[row][col]) {
    this->keystate[row][col] = debounced_input;
    PinUtils::pinmode_input_pullup(ROW_PINS[row]);

    if (debounced_input == DEBOUNCE_MAX) {
        return ChangedKeyCoords { EVENT_KEY_PRESS, row, col };
    } else {
        return ChangedKeyCoords { EVENT_KEY_RELEASE, row, col };
    }
}
```

After that we check the previous state of the key and if it's different, then we update the `keystate` value to the new one and return a `ChangedKeyCoords` struct.

## Debouncing

<img src="contact_bouncing.png" alt="contact_bouncing" style="zoom: 50%;" />

When we press a key, the two metal electrodes don't instantly form a connection, but they bounce around a bit (altering between connected and disconnected states). Without debouncing we could interpret a single key press as several press and release events in a row, which is bad for a keyboard (If I press a button once, I expect the keyboard to send a single press event).

There are multiple ways of debouncing an input. We could for example implement it using `read -> sleep -> read` (and comparing the states before and after). In Dalsik I use a separate, 6-step debounce array (each key has it's own debounce value). My solution was inspired by [Kenneth A. Kuhn](www.kennethkuhn.com/electronics/debounce.c).

It works like this:

* values in the `debounce` array are always between `DEBOUNCE_MIN` (0) and `DEBOUNCE_MAX` (5)
* during `Matrix::scan` each value for a given key (row/column) is decremented/incremented based on it's state (pressed -> increment, released -> decrement).
* if the `debounce` value is `DEBOUNCE_LOW`, then the key is considered released
* if the `debounce` value is `DEBOUNCE_MAX`, then the key is considered pressed
* if the `debounce` value is between `DEBOUNCE_LOW` and `DEBOUNCE_MAX`, then the key is considered to be in it's previous state (`DEBOUNCE_CHANGING`)

```c++
uint8_t Matrix::debounce_input(uint8_t row, uint8_t col, uint8_t input) {
    if (input) {
        if (this->debounce[row][col] < DEBOUNCE_MAX) {
            this->debounce[row][col]++;
        }
        if (this->debounce[row][col] == DEBOUNCE_MAX) {
            return DEBOUNCE_MAX;
        }
    } else {
        if (this->debounce[row][col] > DEBOUNCE_MIN) {
            this->debounce[row][col]--;
        }
        if (this->debounce[row][col] == DEBOUNCE_MIN) {
            return DEBOUNCE_MIN;
        }
    }
    return DEBOUNCE_CHANGING;
}
```


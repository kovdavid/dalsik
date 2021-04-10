# Microcontroller Basics

The heart of the keyboard is the 8-bit AVR Pro Micro (atmega32u4) microcontroller on each side of the keyboard. One of the Pro Micro's is powered via a USB cable and this microcontroller powers the other side via the TRRS cable.

<img src="atmega32u4.png" alt="Pri Micro" style="zoom: 67%;" />

## Main parameters

* 16MHz processor
* Flash Program Memory: 32kB (for the code)
* EEPROM: 1kB (keymap and other static configuration is stored here. The memory is non-volatile, which means that it remembers it's data even if powered off. The trade-offs are slower writing speed and limited number of writes before wear-off)
* SRAM: 2.5kB (for the variables. This memory is volatile, which means that all the data is lost when the device is powered off)
* Hardware USB support (no need to emulate it in soft)

## Flashing the Pro Micro

Flashing is compiling the C++ code and uploading it into the microcontroller's Flash memory. The Pro Micro has a bootloader, which supports flashing via the USB interface, which makes the whole process very easy, as there is no need for additional hardware.

For flashing the Pro Micro I use the [Arduino IDE](https://www.arduino.cc/en/software). It has a GUI for editing / flashing the code, but I don't use it. For code editing I use Vim and for flashing I use the command-line interface of the Arduino IDE ( [Makefile](https://github.com/DavsX/dalsik/blob/master/Makefile) ).

## I/O

The Pro Micro has multiple pins, which can be used for input and output. The module PinUtils ( [pin_utils.h](https://github.com/DavsX/dalsik/blob/master/pin_utils.h),  [pin_utils.ino](https://github.com/DavsX/dalsik/blob/master/pin_utils.ino) ) provides constants and helper functions for setting up pins for input/output and for reading/writing. A pin always needs to be configured first for input or output and after we can read/write from/to that pin.

Reading/writing to pins is the main thing that the Dalsik software is doing for determining which key is pressed/released and for the master-slave communication.

### Using a pin as input

When we configure a pin as an input, it is be default connected via a pull-up resistor (R1) to Vcc, so that the pin is not _floating_. That means, that we must convert the electric signal read from the pin to a logical _on_/_off_ state by inverting the value read (i.e. high electric signal means logical "0", while a low signal means "1").

Using a pull-down resistor would also work, but the atmega32u4 chip has internal pull-up resistors, so that way we don't need any additional hardware.

<img src="pull_up_resistor.jpg" alt="pull_up_resistor" style="zoom:50%;" />

```c++
#include "pin_utils.h"

uint8_t pin = PIN_B(4);

PinUtils::pinmode_input_pullup(pin); // Setup

uint8_t value = PinUtils::read_pin(pin)
```



### Using a pin as output

In output mode  we can set the pin's output value to _low_ or _high_ electric signal.

```c++
#include "pin_utils.h"

uint8_t pin = PIN_B(4);

PinUtils::pinmode_output_low(pin); // Setup + initial "low" value
PinUtils::pinmode_output_high(pin); // Setup + initial "high" value

PinUtils::set_output_low(pin);
PinUtils::set_output_high(pin);
```


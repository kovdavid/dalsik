# Master & Slave

<img src="lets_split.jpg" alt="Let's Split keyboard" style="zoom: 25%;" />

The Let's Split keyboard consists of two halves:

* master
  * The side with the attached USB cable
  * Responsible for reporting the various press/release events to the operating system
  * Scans it's own key matrix and processes the changes
  * Received the changes from the slave side
* slave
  * The side without the attached USB cable
  * Scans it's own key matrix and sends the change events to the master side (via the TRRS cable)

## Master election

Each side decides independently if it is going to run as master or slave in the `setup()` function:

```c++
// dalsik.ino

uint8_t is_master = 0;

void setup() {
    delay(100);
    if (usb_connected()) {
		is_master = 1;
    }
    delay(100);

    if (is_master) {
        DalsikSerial::master_init();
    } else {
        DalsikSerial::slave_init();
    }
}

uint8_t usb_connected() {
   USBCON |= (1 << OTGPADE); //enables VBUS pad
   delayMicroseconds(5);
   return (USBSTA & (1 << VBUS)); //checks state of VBUS
}
```



## Master-Slave communication

The communication between the two halves is one-directional. It is always the slave side, which is sending it's changes to the master side, using a single-wire serial protocol.

Therefore the master side initializes its serial communication pin for input (`DalsikSerial::master_init()`), while the slave configures for output (`DalsikSerial::slave_init()`).

Modules:

* [dalsik_serial.h](https://github.com/DavsX/dalsik/blob/master/dalsik_serial.h)
* [dalsik_serial.ino](https://github.com/DavsX/dalsik/blob/master/dalsik_serial.ino)
* [ring_buffer.h](https://github.com/DavsX/dalsik/blob/master/ring_buffer.h)
* [ring_buffer.ino](https://github.com/DavsX/dalsik/blob/master/ring_buffer.ino)

## DalsikSerial::slave_init()

```c++
// dalsik_serial.ino
void DalsikSerial::slave_init(void) {
    PinUtils::pinmode_output_high(SERIAL_PIN);
}
```

The slave side configures it's pin to output with the default value of _high_ (because of the pull-up resistor on the master side).

## DalsikSerial::master_init()

```c++
// dalsik_serial.ino
void DalsikSerial::master_init(void) {
    PinUtils::pinmode_input_pullup(SERIAL_PIN);
    serial_set_interrupt();
}

inline static void serial_set_interrupt() {
    // Enable INT0
    EIMSK |= _BV(INT0);
    // Trigger on falling edge of INT0
    // ATmega32U4 datasheet chapter 11.1.1 - External Interrupt Control Register A - EICRA
    EICRA &= ~(1 << ISC00);
    EICRA |=  (1 << ISC01);
}
```

The master side initializes it's `SERIAL_PIN` for input using a pull-up resistor. Once again during idle state the pin's state is _high_.

How is the master side able to detect, that the slave side is sending data? Using an interrupt. An interrupt stops the current code and starts executing an interrupt handler. The interrupt is set up in the `serial_set_interrupt()` function on the falling edge of the `INT0` interrupt vector, which corresponds to the `PIN_D(0)` pin:

```c++
// dalsik_serial.h
#define SERIAL_PIN PIN_D(0)
```

The serial pin is idling on _high_. Anytime drops to _low_, the interrupt handler of the master side is executed:

```c++
// dalsik_serial.ino
ISR(SERIAL_PIN_INTERRUPT) {
    uint8_t slave_data = serial_master_read();
    DalsikSerial::serial_buffer.append_elem(slave_data);
    // Clear pending interrupts for INTF0 (which were registered during serial_master_read)
    EIFR |= (1 << SERIAL_PIN_INTERRUPT_FLAG);
}
```

The master does 3 things in the interrupt handler:

* read 1 byte of information from the slave side
* add this byte to the `DalsikSerial::serial_buffer` ring buffer
* clear the "interrupt flag", which triggers the interrupt handler. This must be cleared, because it is probably set during data transfer (for example, when transmitting `0b10101010`).

## Master - Slave communication



![serial_communication](serial_communication.png)

When the slave side wants to transmit data to the master side, these things happen:

* each "bit" is transmitted for `SERIAL_DELAY` microseconds (50μs) - this value is hard-coded on both halves

* the slave side pulls the serial signal to _low_ (`PinUtils::set_output_low(SERIAL_PIN)`) (Phase 1)

  * this triggers the interrupt handler on the master side

* after 50μs the slave sets the signal back to _high_ for 50μs (Phase 2)

* when the master fires its interrupt handler, it tries to synchronize with the slave by running a loop, while the signal is _low_ (`while (!PinUtils::read_pin(SERIAL_PIN))`) - (Phase 1)

  * when the slave sets the signal to _high_ (Phase 2), then the master exits its loop

* The master waits for 50μs for the end of Phase 2, when the actual data transfer starts

* There are only 4 bits transmitted on the image above (Phase 3), but in reality there is always 1 byte (8 bits) send

* The master does not start reading from the pin right away, but instead waits for another 25μs, so that it gets to the middle of the bit, where the signal is not changing. Then it starts reading bits with 50μs delay

* The actual data transfer on the slave side is as follows:

  * The most significant bit (MSB) is transmitted first.
  * If the bit is 1, the output is set to _high_ - `PinUtils::set_output_high(SERIAL_PIN)`
  * If the bit is 0, the output is set to _low_ - `PinUtils::set_output_low(SERIAL_PIN)`
  * Each bit is transmitted for 50μs

* After sending the data the slave sets the output to _high_ (to its idle state) and waits for 50μs (Phase 4)

  * this additional delay gives time for the master to put the data into the RingBuffer and to reset its interrupt vector

  

```c++
// dalsik_serial.ino

void DalsikSerial::slave_send(uint8_t data) {
    // Trigger the interrupt on the master & send the init LOW/HIGH
    PinUtils::set_output_low(SERIAL_PIN);
    serial_delay();
    PinUtils::set_output_high(SERIAL_PIN);
    serial_delay();

    // Send data - MSB
    for (int8_t i = 7; i >= 0; i--) {
        if (data & (1 << i)) {
            PinUtils::set_output_high(SERIAL_PIN);
        } else {
            PinUtils::set_output_low(SERIAL_PIN);
        }
        serial_delay();
    }

    // Pull the line HIGH - IDLE
    PinUtils::set_output_high(SERIAL_PIN);
    serial_delay();
}

inline static uint8_t serial_master_read() {
    serial_half_delay();
    // Sync to the end of slave LOW
    while (!PinUtils::read_pin(SERIAL_PIN));
    // Wait till the HIGH signal
    serial_delay();
    // Get to the middle of the data signal
    serial_half_delay();

    uint8_t data = 0;

    // Receive data - MSB
    for (uint8_t i = 0; i < 8; i++) {
        data = (data << 1) | PinUtils::read_pin(SERIAL_PIN);
        serial_delay();
    }

    return data;
}
```

## RingBuffer

The data received from the slave side is written into a 10 bytes long RingBuffer in the `DalsikSerial` module. This data is the processed later on in the main loop.

At the beginning I was using just a single `uint8_t` as a buffer, but soon I experienced some bugs. If the slave sent too much data too fast (pressing/releasing of multiple keys with multiple fingers), then sometimes some events in the buffer were overridden by the new data before they could be processed. This cause the master side to miss the "The key E was released" event, so a key got stuck.

I chose the size of 10 bytes, because the slave side is intended to be used with 1 hand, so the press and sudden release of a key with each finger results in 10 events.

The RingBuffer is backed by an array of 10 `uint8_t`s with separate variables for the `read_index`, `write_index` and `size` (number of actual unprocessed bytes in the RingBuffer).

```c++
// ring_buffer.h

#define BUFFER_LENGTH 10

class RingBuffer {
    private:
        volatile uint8_t buffer[BUFFER_LENGTH];
        volatile uint8_t size;
        uint8_t read_index;
        uint8_t write_index;
	...
}
```

The `read_index` variable holds the index we should read from, if `size > 0`.

After reading a byte, the `read_index` is incremented and the `size` variable is atomically decreased.

```c++
// ring_buffer.ino
// Called from main loop
uint8_t RingBuffer::get_next_elem() {
    uint8_t elem = this->buffer[ this->read_index ];
    this->read_index = (this->read_index + 1) % BUFFER_LENGTH;
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        this->size--;
    }
    return elem;
}
```

The atomic block around `this->size--;` is needed, because this line of code consists of three instructions:

* fetch the current value from RAM into a register
* decrement the value of the register
* write the value to RAM

This sequence may be interrupted at any time by our interrupt handler, which causes data races, because in the interrupt handler we write to the RingBuffer, which increments the `size` variable:

```c++
// Called from ISR
void RingBuffer::append_elem(uint8_t elem) {
    if (this->size >= BUFFER_LENGTH) {
        return;
    }
    this->buffer[ this->write_index ] = elem;
    this->write_index = (this->write_index + 1) % BUFFER_LENGTH;
    this->size++;
}
```

Note, that the `this->size++;` statement is not wrapped in an atomic block, as it already runs in an interrupt handler (which can not be interrupted).

Also note, that if the RingBuffer is full, we simply drop the data (which is bad, but in practice it never happened to me).
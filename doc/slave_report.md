# SlaveReport

Modules:

* [slave_report.h](https://github.com/DavsX/dalsik/blob/master/slave_report.h)
* [slave_report.ino](https://github.com/DavsX/dalsik/blob/master/slave_report.ino)

SlaveReport is used to encode/decode events on the slave side. It is also the module, which the slave side uses to send data via the `DalsikSerial` module to the master:

```c++
// dalsik.ino
void loop() {
    ...
       
    ChangedKeyCoords coords = matrix.scan();
    if (coords.type == EVENT_NONE) {
        return;
    }
 
    if (is_master) {
        ...
    } else {
        SlaveReport::send_changed_key(coords);
    }
}
```

```c++
// slave_report.ino
void SlaveReport::send_changed_key(ChangedKeyCoords coords) {
    if (coords.type == EVENT_NONE) {
        return;
    }

    uint8_t slave_data = SlaveReport::encode_slave_report_data(coords);

    DalsikSerial::slave_send(slave_data);
}
```

On the master side, the decoding functionality is used:

```c++
// dalsik.ino
void loop() {
    ...

    if (is_master) {
        while (DalsikSerial::serial_buffer.has_data()) {
            uint8_t slave_data = DalsikSerial::serial_buffer.get_next_elem();
            handle_slave_data(slave_data);
        }
    }

    ...
}

inline void handle_slave_data(uint8_t data) {
    ChangedKeyCoords coords = SlaveReport::decode_slave_report_data(data);
	master_report.handle_slave_changed_key(coords);
}
```

##  Data format

What information does the slave side send to the master side?

* Event type - press or release
* Row index
* Column index

Initially I used 3 bytes to encode this information, but later I realized, that I could encode this into a single byte:

* Event type needs only 1 bit - PRESS=1, RELEASE=0
* Each half has 4 rows, which can be encoded into 2 bits
* Each half has 6 columns, which can be encoded into 3 bits
* The rest (2 bits) is used as parity bits

```c++
// We use 1B to send type, row and col from ChangedKeyCoords + 2 parity
// ABBCCCDE
// A - EVENT_KEY_PRESS:1 EVENT_KEY_RELEASE:0 (EVENT_NONE is not sent by slave)
// B - row
// C - column
// D - parity of A+B+C (bitmask 1111_1100 - FC)
// E - parity of B+C (bitmask 0111_1100 - 7C)

uint8_t SlaveReport::encode_slave_report_data(ChangedKeyCoords coords) {
    uint8_t data = 0x00;

    if (coords.type == EVENT_KEY_PRESS) {
        data |= 0x80;
    }
    data |= (coords.row << 5) & 0x60;
    data |= (coords.col << 2) & 0x1C;

    uint8_t p1 = parity(data & 0xFC);
    uint8_t p2 = parity(data & 0x7C);

    data |= (p1 << 1) & 0x02;
    data |= (p2 << 0) & 0x01;

    return data;
}
```


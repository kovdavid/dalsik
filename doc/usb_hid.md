# USB HID Structure

HID (Human Input Device) is part of the USB specification, which defines the communication protocol of input devices.

## Base report

The base HID report is 8 bytes long. It is a data structure which contains the keys, that are pressed at a given moment.

```c++
// dalsik_hid_desc.h
#define BASE_HID_REPORT_KEYS 6

typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[BASE_HID_REPORT_KEYS];
} BaseHIDReport;
```

`modifiers` represents the 8 modifier keys (1 - pressed, 0 - released):

```
LEFT  CONTROL          1    # 00000001
LEFT  SHIFT            2    # 00000010
LEFT  ALT              4    # 00000100
LEFT  CMD|WIN          8    # 00001000
RIGHT CONTROL          16   # 00010000
RIGHT SHIFT            32   # 00100000
RIGHT ALT              64   # 01000000
RIGHT CMD|WIN          128  # 10000000
```

The `reserved` byte is always `0x00`.

The `keys` array contains the array of pressed keys (see [key_definitions.h](https://github.com/DavsX/dalsik/blob/master/key_definitions.h)).

For example the key code for "A" is `0x04`.

If we want to press "a", we need to send:

```
#mod #res #key
0x00 0x00 0x04 0x00 0x00 0x00 0x00 0x00
```

If we want to press "A", we need to send (Shift modifier + "a"):

```
#mod #res #key
0x02 0x00 0x04 0x00 0x00 0x00 0x00 0x00
```

The base HID report (not counting the 8 modifiers) allows us to report up to 6 keys at once. Some (mainly gaming) keyboards implement `NKRO` (N-key rollover), which allows us to press more keys at once, but I never bothered to implement it.

The HID report is sent using the Arduino USB HID library (the USB protocol is quite complicated, so I never implemented it myself):

```c++
// in master_report.h -> BaseHIDReport base_hid_report
// master_report.ino
void MasterReport::send_base_hid_report() {
    void *report = &(this->base_hid_report);
    HID().SendReport(BASE_KEYBOARD_REPORT_ID, report, sizeof(BaseHIDReport));
}
```

## System and Multimedia reports

```c++
// dalsik_hid_desc.h

typedef struct {
    uint8_t key;
} SystemHIDReport;

typedef struct {
    uint8_t key;
    uint8_t prefix;
} MultimediaHIDReport;
```

Dalsik supports two additional report types: System and Multimedia.

The SystemHIDReport can be used for power_off, sleep, wake_up etc.

The MultimediaHIDReport can be used for audio mute, volume up, volume down, launch (calculator, www, file browser, screen saver).
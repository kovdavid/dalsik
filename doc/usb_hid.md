# USB HID Structure

HID (Human Input Device) is part of the USB specification, which defines the way input devices use the tell the operating system which keys were pressed or released.

## Base report

The base HID report is 8 bytes long:

```c++
// dalsik_hid_desc.h
#define BASE_HID_REPORT_KEYS 6

typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[BASE_HID_REPORT_KEYS];
} BaseHIDReport;
```

The `modifiers` represents the 8 modifier keys (1 - pressed, 0 - released):

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

The `keys` array contains the array of pressed keys.

For example the key code for "A" is `0x04`.

If we want to press "a", we need to send:

```
#mod #res #key
0x00 0x00 0x04 0x00 0x00 0x00 0x00 0x00
```

If we want to press "A", we need to send:

```
#mod #res #key
0x02 0x00 0x04 0x00 0x00 0x00 0x00 0x00
```

There is no separate code for "a" and "A". In the latter case we need to set one of the Shift modifiers.

The base HID report (not counting the 8 modifiers) allows for the simultaneous pressing of 6 keys. Some (mainly gaming) keyboards implement the `NKRO` (N-key rollover), which allows for the simultaneous pressing of more, than 6 keys, but I never bothered to implement it.

The HID report is sent using the Arduino's USB HID library (the USB protocol is quite complicated, so I never wrote it myself):

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
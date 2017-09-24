#ifndef KEYREPORT_h
#define KEYREPORT_h

typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
} KeyReport;

#endif

// bit number
// 76543210
// DEADBEEF

// bit 0: left control
// bit 1: left shift
// bit 2: left alt
// bit 3: left GUI (Win/Apple/Meta key)
// bit 4: right control
// bit 5: right shift
// bit 6: right alt
// bit 7: right GUI
//
// E0 Left control
// E1 Left shift
// E2 Left alt
// E3 Left GUI
// E4 Right control
// E5 Right shift
// E6 Right alt
// E7 Right GUI

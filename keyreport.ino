void keyreport_clear(KeyReport* report)
{
    report->modifiers = 0;
    report->reserved = 0;
    memset(report->keys, 0, 6);
}

void keyreport_press(KeyReport* report, uint8_t keycode)
{
    if (keycode >= 0xE0 && keycode <= 0xE7) {
        keyreport_press_modifier(report, keycode);
    } else {
        keyreport_press_key(report, keycode);
    }
}

void keyreport_release(KeyReport* report, uint8_t keycode)
{
    if (keycode >= 0xE0 && keycode <= 0xE7) {
        keyreport_release_modifier(report, keycode);
    } else {
        keyreport_release_key(report, keycode);
    }
}

void keyreport_press_modifier(KeyReport* report, uint8_t keycode)
{
    // For 'Left Shift' 0xE1 bitmask is 0x0000_0010
    uint8_t modifier_bit = keycode & 0x0F;
    uint8_t bitmask = 1 << modifier_bit;
    report->modifiers |= bitmask;
}

void keyreport_release_modifier(KeyReport* report, uint8_t keycode)
{
    // For 'Left Shift' 0xE1 bitmask is 0x1111_1101
    uint8_t modifier_bit = keycode & 0x0F;
    uint8_t bitmask = (1 << modifier_bit) ^ 0xFF;
    report->modifiers &= bitmask;
}

void keyreport_press_key(KeyReport* report, uint8_t keycode)
{
    for (uint8_t i = 0; i < 6; i++) {
        if (report->keys[i] == keycode) {
            return; // already pressed
        }
        if (report->keys[i] == 0x00) {
            report->keys[i] = keycode;
            return;
        }
    }
}

void keyreport_release_key(KeyReport* report, uint8_t keycode)
{
    for (uint8_t i = 0; i < 6; i++) {
        if (report->keys[i] == keycode) {
            // shift the rest of the keys to the left
            for (uint8_t j = i; j < 5; j++) {
                report->keys[j] = report->keys[j+1];
            }
            report->keys[5] = 0x00;
            return;
        }
    }
}

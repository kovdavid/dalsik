#ifndef KEYDEFINITIONS_H
#define KEYDEFINITIONS_H

// INTERNAL_NAME BYTE_VALUE /* SHORTCUT */
// Keys with arguments have SHORTCUT ending with '(' or ','

#define KEY_UNSET                0x00 /* KC_NO      */
#define KEY_NORMAL               0x01 /* KC(        */
#define KEY_DUAL_LCTRL           0x02 /* D(LCTRL,   */
#define KEY_DUAL_RCTRL           0x03 /* D(RCTRL,   */
#define KEY_DUAL_LSHIFT          0x04 /* D(LSHIFT,  */
#define KEY_DUAL_RSHIFT          0x05 /* D(RSHIFT,  */
#define KEY_DUAL_LGUI            0x06 /* D(LGUI,    */
#define KEY_DUAL_RGUI            0x07 /* D(RGUI,    */
#define KEY_DUAL_LALT            0x08 /* D(LALT,    */
#define KEY_DUAL_RALT            0x09 /* D(RALT,    */
#define KEY_LAYER_PRESS          0x0A /* LP(        */
#define KEY_LAYER_TOGGLE         0x0B /* LT(        */
#define KEY_LAYER_HOLD_OR_TOGGLE 0x0C /* LHT(       */
#define KEY_WITH_MOD_LCTRL       0x0D /* LCTRL(     */
#define KEY_WITH_MOD_RCTRL       0x0E /* RCTRL(     */
#define KEY_WITH_MOD_LSHIFT      0x0F /* LSHIFT(    */
#define KEY_WITH_MOD_RSHIFT      0x10 /* RSHIFT(    */
#define KEY_WITH_MOD_LGUI        0x11 /* LGUI(      */
#define KEY_WITH_MOD_RGUI        0x12 /* RGUI(      */
#define KEY_WITH_MOD_LALT        0x13 /* LALT(      */
#define KEY_WITH_MOD_RALT        0x14 /* RALT(      */
#define KEY_SYSTEM               0x15 /* KC_SYSTEM( */
#define KEY_MULTIMEDIA_0         0x16 /* KC_M0(     */
#define KEY_MULTIMEDIA_1         0x17 /* KC_M1(     */
#define KEY_MULTIMEDIA_2         0x18 /* KC_M2(     */
#define KEY_TAPDANCE             0x19 /* TD(        */
#define KEY_DUAL_LAYER_1         0x1A /* D(LAYER_1, */
#define KEY_DUAL_LAYER_2         0x1B /* D(LAYER_2, */
#define KEY_DUAL_LAYER_3         0x1C /* D(LAYER_3, */
#define KEY_DUAL_LAYER_4         0x1D /* D(LAYER_4, */
#define KEY_DUAL_LAYER_5         0x1E /* D(LAYER_5, */
#define KEY_DUAL_LAYER_6         0x1F /* D(LAYER_6, */
#define KEY_DUAL_LAYER_7         0x20 /* D(LAYER_7, */
#define KEY_TRANSPARENT          0xFF /* TRANS      */

#define ALIAS_SYS_POWER_OFF KC_SYSTEM(0x81)
#define ALIAS_SYS_SLEEP     KC_SYSTEM(0x82)
#define ALIAS_SYS_WAKE_UP   KC_SYSTEM(0x83)

#define ALIAS_AUDIO_MUTE    KC_M0(0xE2)
#define ALIAS_AUDIO_VOLUP   KC_M0(0xE9)
#define ALIAS_AUDIO_VOLDOWN KC_M0(0xEA)

#define ALIAS_LAUNCH_CALCULATOR    KC_M1(0x92)
#define ALIAS_LAUNCH_WWW           KC_M1(0x94)
#define ALIAS_LAUNCH_FILE_BROWSER  KC_M1(0x96)
#define ALIAS_LAUNCH_SCREEN_SAVER  KC_M1(0x9E)

#define KC_NO               0x00
#define KC_ROLL_OVER        0x01
#define KC_POST_FAIL        0x02
#define KC_UNDEFINED        0x03
#define KC_A                0x04
#define KC_B                0x05
#define KC_C                0x06
#define KC_D                0x07
#define KC_E                0x08
#define KC_F                0x09
#define KC_G                0x0A
#define KC_H                0x0B
#define KC_I                0x0C
#define KC_J                0x0D
#define KC_K                0x0E
#define KC_L                0x0F
#define KC_M                0x10
#define KC_N                0x11
#define KC_O                0x12
#define KC_P                0x13
#define KC_Q                0x14
#define KC_R                0x15
#define KC_S                0x16
#define KC_T                0x17
#define KC_U                0x18
#define KC_V                0x19
#define KC_W                0x1A
#define KC_X                0x1B
#define KC_Y                0x1C
#define KC_Z                0x1D
#define KC_1                0x1E
#define KC_2                0x1F
#define KC_3                0x20
#define KC_4                0x21
#define KC_5                0x22
#define KC_6                0x23
#define KC_7                0x24
#define KC_8                0x25
#define KC_9                0x26
#define KC_0                0x27
#define KC_ENT              0x28
#define KC_ENTER            0x28
#define KC_ESC              0x29
#define KC_ESCAPE           0x29
#define KC_BSPACE           0x2A
#define KC_BSPC             0x2A
#define KC_TAB              0x2B
#define KC_SPACE            0x2C
#define KC_SPC              0x2C
#define KC_MINS             0x2D
#define KC_MINUS            0x2D
#define KC_EQL              0x2E
#define KC_EQUAL            0x2E
#define KC_LBRACKET         0x2F
#define KC_LBRC             0x2F
#define KC_RBRACKET         0x30
#define KC_RBRC             0x30
#define KC_BSLASH           0x31
#define KC_BSLS             0x31
#define KC_NONUS_HASH       0x32
#define KC_NUHS             0x32
#define KC_SCLN             0x33
#define KC_SCOLON           0x33
#define KC_QUOT             0x34
#define KC_QUOTE            0x34
#define KC_GRAVE            0x35
#define KC_GRV              0x35
#define KC_COMM             0x36
#define KC_COMMA            0x36
#define KC_DOT              0x37
#define KC_SLASH            0x38
#define KC_SLSH             0x38
#define KC_CAPS             0x39
#define KC_CAPSLOCK         0x39
#define KC_CLCK             0x39
#define KC_F1               0x3A
#define KC_F2               0x3B
#define KC_F3               0x3C
#define KC_F4               0x3D
#define KC_F5               0x3E
#define KC_F6               0x3F
#define KC_F7               0x40
#define KC_F8               0x41
#define KC_F9               0x42
#define KC_F10              0x43
#define KC_F11              0x44
#define KC_F12              0x45
#define KC_PSCR             0x46
#define KC_PSCREEN          0x46
#define KC_SCROLLLOCK       0x47
#define KC_SLCK             0x47
#define KC_BRK              0x48
#define KC_PAUS             0x48
#define KC_PAUSE            0x48
#define KC_INS              0x49
#define KC_INSERT           0x49
#define KC_HOME             0x4A
#define KC_PGUP             0x4B
#define KC_DEL              0x4C
#define KC_DELETE           0x4C
#define KC_END              0x4D
#define KC_PGDN             0x4E
#define KC_PGDOWN           0x4E
#define KC_RGHT             0x4F
#define KC_RIGHT            0x4F
#define KC_LEFT             0x50
#define KC_DOWN             0x51
#define KC_UP               0x52
#define KC_NLCK             0x53
#define KC_NUMLOCK          0x53
#define KC_KP_SLASH         0x54
#define KC_PSLS             0x54
#define KC_KP_ASTERISK      0x55
#define KC_PAST             0x55
#define KC_KP_MINUS         0x56
#define KC_PMNS             0x56
#define KC_KP_PLUS          0x57
#define KC_PPLS             0x57
#define KC_KP_ENTER         0x58
#define KC_PENT             0x58
#define KC_KP_1             0x59
#define KC_P1               0x59
#define KC_KP_2             0x5A
#define KC_P2               0x5A
#define KC_KP_3             0x5B
#define KC_P3               0x5B
#define KC_KP_4             0x5C
#define KC_P4               0x5C
#define KC_KP_5             0x5D
#define KC_P5               0x5D
#define KC_KP_6             0x5E
#define KC_P6               0x5E
#define KC_KP_7             0x5F
#define KC_P7               0x5F
#define KC_KP_8             0x60
#define KC_P8               0x60
#define KC_KP_9             0x61
#define KC_P9               0x61
#define KC_KP_0             0x62
#define KC_P0               0x62
#define KC_KP_DOT           0x63
#define KC_PDOT             0x63
#define KC_NONUS_BSLASH     0x64
#define KC_NUBS             0x64
#define KC_APP              0x65
#define KC_APPLICATION      0x65
#define KC_POWER            0x66
#define KC_KP_EQUAL         0x67
#define KC_PEQL             0x67
#define KC_F13              0x68
#define KC_F14              0x69
#define KC_F15              0x6A
#define KC_F16              0x6B
#define KC_F17              0x6C
#define KC_F18              0x6D
#define KC_F19              0x6E
#define KC_F20              0x6F
#define KC_F21              0x70
#define KC_F22              0x71
#define KC_F23              0x72
#define KC_F24              0x73
#define KC_EXEC             0x74
#define KC_EXECUTE          0x74
#define KC_HELP             0x75
#define KC_MENU             0x76
#define KC_SELECT           0x77
#define KC_SLCT             0x77
#define KC_STOP             0x78
#define KC_AGAIN            0x79
#define KC_AGIN             0x79
#define KC_UNDO             0x7A
#define KC_CUT              0x7B
#define KC_COPY             0x7C
#define KC_PASTE            0x7D
#define KC_PSTE             0x7D
#define KC_FIND             0x7E
#define KC_MUTE             0x7F
#define KC_VOLUP            0x80
#define KC_VOLDOWN          0x81
#define KC_LCAP             0x82
#define KC_LOCKING_CAPS     0x82
#define KC_LNUM             0x83
#define KC_LOCKING_NUM      0x83
#define KC_LOCKING_SCROLL   0x84
#define KC_LSCR             0x84
#define KC_KP_COMMA         0x85
#define KC_PCMM             0x85
#define KC_KP_EQUAL_AS400   0x86
#define KC_INT1             0x87
#define KC_INT2             0x88
#define KC_INT3             0x89
#define KC_INT4             0x8A
#define KC_INT5             0x8B
#define KC_INT6             0x8C
#define KC_INT7             0x8D
#define KC_INT8             0x8E
#define KC_INT9             0x8F
#define KC_LANG1            0x90
#define KC_LANG2            0x91
#define KC_LANG3            0x92
#define KC_LANG4            0x93
#define KC_LANG5            0x94
#define KC_LANG6            0x95
#define KC_LANG7            0x96
#define KC_LANG8            0x97
#define KC_LANG9            0x98
#define KC_ALT_ERASE        0x99
#define KC_ERAS             0x99
#define KC_SYSREQ           0x9A
#define KC_CANCEL           0x9B
#define KC_CLEAR            0x9C
#define KC_CLR              0x9C
#define KC_PRIOR            0x9D
#define KC_RETURN           0x9E
#define KC_SEPARATOR        0x9F
#define KC_OUT              0xA0
#define KC_OPER             0xA1
#define KC_CLEAR_AGAIN      0xA2
#define KC_CRSEL            0xA3
#define KC_EXSEL            0xA4
#define KC_LCTL             0xE0
#define KC_LCTRL            0xE0
#define KC_LSFT             0xE1
#define KC_LSHIFT           0xE1
#define KC_LALT             0xE2
#define KC_LGUI             0xE3
#define KC_RCTL             0xE4
#define KC_RCTRL            0xE4
#define KC_RSFT             0xE5
#define KC_RSHIFT           0xE5
#define KC_RALT             0xE6
#define KC_RGUI             0xE7

#endif

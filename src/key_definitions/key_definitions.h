#pragma once

enum key_types {
    KEY_BASIC = 0x00,
    KEY_LAYER_HOLD,
    KEY_LAYER_TOGGLE,
    KEY_LAYER_HOLD_OR_TOGGLE,
    KEY_DESKTOP,
    KEY_CONSUMER,
    KEY_TAPDANCE,
    KEY_ONE_SHOT_MODIFIER,
    KEY_DUAL_MODIFIERS,
    KEY_SOLO_DUAL_MODIFIERS,
    KEY_TAP_HOLD_DUAL_MODIFIERS,
    KEY_DUAL_LAYER,
    KEY_SOLO_DUAL_LAYER,
    KEY_TAP_HOLD_DUAL_LAYER,
    KEY_TOGGLE_CAPS_WORD,
    KEY_MOUSE_BUTTON,
    KEY_MOUSE_CURSOR,
    KEY_TRANSPARENT = 0xFF,
};

// Copied from QMK
enum hid_keyboard_keys {
    KC_NO = 0x00,
    KC_ROLL_OVER,
    KC_POST_FAIL,
    KC_UNDEFINED,
    KC_A,
    KC_B,
    KC_C,
    KC_D,
    KC_E,
    KC_F,
    KC_G,
    KC_H,
    KC_I,
    KC_J,
    KC_K,
    KC_L,
    KC_M, // 0x10
    KC_N,
    KC_O,
    KC_P,
    KC_Q,
    KC_R,
    KC_S,
    KC_T,
    KC_U,
    KC_V,
    KC_W,
    KC_X,
    KC_Y,
    KC_Z,
    KC_1,
    KC_2,
    KC_3, // 0x20
    KC_4,
    KC_5,
    KC_6,
    KC_7,
    KC_8,
    KC_9,
    KC_0,
    KC_ENTER,
    KC_ESCAPE,
    KC_BACKSPACE,
    KC_TAB,
    KC_SPACE,
    KC_MINUS,
    KC_EQUAL,
    KC_LEFT_BRACKET,
    KC_RIGHT_BRACKET, // 0x30
    KC_BACKSLASH,
    KC_NONUS_HASH,
    KC_SEMICOLON,
    KC_QUOTE,
    KC_GRAVE,
    KC_COMMA,
    KC_DOT,
    KC_SLASH,
    KC_CAPS_LOCK,
    KC_F1,
    KC_F2,
    KC_F3,
    KC_F4,
    KC_F5,
    KC_F6,
    KC_F7, // 0x40
    KC_F8,
    KC_F9,
    KC_F10,
    KC_F11,
    KC_F12,
    KC_PRINT_SCREEN,
    KC_SCROLL_LOCK,
    KC_PAUSE,
    KC_INSERT,
    KC_HOME,
    KC_PAGE_UP,
    KC_DELETE,
    KC_END,
    KC_PAGE_DOWN,
    KC_RIGHT,
    KC_LEFT, // 0x50
    KC_DOWN,
    KC_UP,
    KC_NUM_LOCK,
    KC_KP_SLASH,
    KC_KP_ASTERISK,
    KC_KP_MINUS,
    KC_KP_PLUS,
    KC_KP_ENTER,
    KC_KP_1,
    KC_KP_2,
    KC_KP_3,
    KC_KP_4,
    KC_KP_5,
    KC_KP_6,
    KC_KP_7,
    KC_KP_8, // 0x60
    KC_KP_9,
    KC_KP_0,
    KC_KP_DOT,
    KC_NONUS_BACKSLASH,
    KC_APPLICATION,
    KC_KB_POWER,
    KC_KP_EQUAL,
    KC_F13,
    KC_F14,
    KC_F15,
    KC_F16,
    KC_F17,
    KC_F18,
    KC_F19,
    KC_F20,
    KC_F21, // 0x70
    KC_F22,
    KC_F23,
    KC_F24,
    KC_EXECUTE,
    KC_HELP,
    KC_MENU,
    KC_SELECT,
    KC_STOP,
    KC_AGAIN,
    KC_UNDO,
    KC_CUT,
    KC_COPY,
    KC_PASTE,
    KC_FIND,
    KC_KB_MUTE,
    KC_KB_VOLUME_UP, // 0x80
    KC_KB_VOLUME_DOWN,
    KC_LOCKING_CAPS_LOCK,
    KC_LOCKING_NUM_LOCK,
    KC_LOCKING_SCROLL_LOCK,
    KC_KP_COMMA,
    KC_KP_EQUAL_AS400,
    KC_INTERNATIONAL_1,
    KC_INTERNATIONAL_2,
    KC_INTERNATIONAL_3,
    KC_INTERNATIONAL_4,
    KC_INTERNATIONAL_5,
    KC_INTERNATIONAL_6,
    KC_INTERNATIONAL_7,
    KC_INTERNATIONAL_8,
    KC_INTERNATIONAL_9,
    KC_LANGUAGE_1, // 0x90
    KC_LANGUAGE_2,
    KC_LANGUAGE_3,
    KC_LANGUAGE_4,
    KC_LANGUAGE_5,
    KC_LANGUAGE_6,
    KC_LANGUAGE_7,
    KC_LANGUAGE_8,
    KC_LANGUAGE_9,
    KC_ALTERNATE_ERASE,
    KC_SYSTEM_REQUEST,
    KC_CANCEL,
    KC_CLEAR,
    KC_PRIOR,
    KC_RETURN,
    KC_SEPARATOR,
    KC_OUT, // 0xA0
    KC_OPER,
    KC_CLEAR_AGAIN,
    KC_CRSEL,
    KC_EXSEL,
};

// Copied from https://github.com/qmk/qmk_firmware/blob/master/tmk_core/protocol/report.h
enum hid_desktop_keys {
    // 4.5.1 System Controls - Power Controls
    SYSTEM_POWER_DOWN             = 0x81,
    SYSTEM_SLEEP                  = 0x82,
    SYSTEM_WAKE_UP                = 0x83,
    SYSTEM_RESTART                = 0x8F,
};

// Copied from https://github.com/qmk/qmk_firmware/blob/master/tmk_core/protocol/report.h
enum hid_consumer_keys {
    // 15.5 Display Controls
    SNAPSHOT               = 0x065,
    BRIGHTNESS_UP          = 0x06F,
    BRIGHTNESS_DOWN        = 0x070,
    // 15.7 Transport Controls
    TRANSPORT_RECORD       = 0x0B2,
    TRANSPORT_FAST_FORWARD = 0x0B3,
    TRANSPORT_REWIND       = 0x0B4,
    TRANSPORT_NEXT_TRACK   = 0x0B5,
    TRANSPORT_PREV_TRACK   = 0x0B6,
    TRANSPORT_STOP         = 0x0B7,
    TRANSPORT_EJECT        = 0x0B8,
    TRANSPORT_RANDOM_PLAY  = 0x0B9,
    TRANSPORT_STOP_EJECT   = 0x0CC,
    TRANSPORT_PLAY_PAUSE   = 0x0CD,
    // 15.9.1 Audio Controls - Volume
    AUDIO_MUTE             = 0x0E2,
    AUDIO_VOL_UP           = 0x0E9,
    AUDIO_VOL_DOWN         = 0x0EA,
    // 15.15 Application Launch Buttons
    AL_CC_CONFIG           = 0x183,
    AL_EMAIL               = 0x18A,
    AL_CALCULATOR          = 0x192,
    AL_LOCAL_BROWSER       = 0x194,
    AL_LOCK                = 0x19E,
    AL_CONTROL_PANEL       = 0x19F,
    AL_ASSISTANT           = 0x1CB,
    AL_KEYBOARD_LAYOUT     = 0x1AE,
    // 15.16 Generic GUI Application Controls
    AC_NEW                 = 0x201,
    AC_OPEN                = 0x202,
    AC_CLOSE               = 0x203,
    AC_EXIT                = 0x204,
    AC_MAXIMIZE            = 0x205,
    AC_MINIMIZE            = 0x206,
    AC_SAVE                = 0x207,
    AC_PRINT               = 0x208,
    AC_PROPERTIES          = 0x209,
    AC_UNDO                = 0x21A,
    AC_COPY                = 0x21B,
    AC_CUT                 = 0x21C,
    AC_PASTE               = 0x21D,
    AC_SELECT_ALL          = 0x21E,
    AC_FIND                = 0x21F,
    AC_SEARCH              = 0x221,
    AC_HOME                = 0x223,
    AC_BACK                = 0x224,
    AC_FORWARD             = 0x225,
    AC_STOP                = 0x226,
    AC_REFRESH             = 0x227,
    AC_BOOKMARKS           = 0x22A,
    AC_MISSION_CONTROL     = 0x29F,
    AC_LAUNCHPAD           = 0x2A0
};

// https://en.wikipedia.org/wiki/Mouse_keys
enum hid_mouse_keys {
    MOUSE_BUTTON1 = (1 << 0),
    MOUSE_BUTTON2 = (1 << 1),
    MOUSE_BUTTON3 = (1 << 2),
    MOUSE_BUTTON4 = (1 << 3),
    MOUSE_BUTTON5 = (1 << 4),
    MOUSE_BUTTON6 = (1 << 5),
    MOUSE_BUTTON7 = (1 << 6),
    MOUSE_BUTTON8 = (1 << 7),
};

#define MOUSE_LEFT_BUTTON   MOUSE_BUTTON1
#define MOUSE_RIGHT_BUTTON  MOUSE_BUTTON2
#define MOUSE_MIDDLE_BUTTON MOUSE_BUTTON3

#define BTN1 MOUSE_BUTTON1
#define BTN2 MOUSE_BUTTON2
#define BTN3 MOUSE_BUTTON3
#define BTN4 MOUSE_BUTTON4
#define BTN5 MOUSE_BUTTON5
#define BTN6 MOUSE_BUTTON6
#define BTN7 MOUSE_BUTTON7
#define BTN8 MOUSE_BUTTON8

enum hid_mouse_cursor {
    MOUSE_CURSOR_UP    = (1 << 0),
    MOUSE_CURSOR_DOWN  = (1 << 1),
    MOUSE_CURSOR_LEFT  = (1 << 2),
    MOUSE_CURSOR_RIGHT = (1 << 3),
};

#define MOD_CLEAR  0x00

#define MOD_RAW_LCTRL  (1 << 0)
#define MOD_RAW_LSHIFT (1 << 1)
#define MOD_RAW_LALT   (1 << 2)
#define MOD_RAW_LGUI   (1 << 3)
#define MOD_RAW_RCTRL  (1 << 4)
#define MOD_RAW_RSHIFT (1 << 5)
#define MOD_RAW_RALT   (1 << 6)
#define MOD_RAW_RGUI   (1 << 7)

#define MOD_LCTRL  (MOD_RAW_LCTRL  << 8)
#define MOD_LSHIFT (MOD_RAW_LSHIFT << 8)
#define MOD_LALT   (MOD_RAW_LALT   << 8)
#define MOD_LGUI   (MOD_RAW_LGUI   << 8)
#define MOD_RCTRL  (MOD_RAW_RCTRL  << 8)
#define MOD_RSHIFT (MOD_RAW_RSHIFT << 8)
#define MOD_RALT   (MOD_RAW_RALT   << 8)
#define MOD_RGUI   (MOD_RAW_RGUI   << 8)

#define KEY_TYPE(t) (((uint32_t)t) << 24)
#define LAYER(l) (((uint32_t)l) << 16)

// Key constructor macros

#define KC(code)        ( KEY_TYPE(KEY_BASIC) | code )
#define KCM(mod)        ( KEY_TYPE(KEY_BASIC) | mod  )
#define LCTRL(key)      ( KEY_TYPE(KEY_BASIC) | MOD_LCTRL  | key )
#define RCTRL(key)      ( KEY_TYPE(KEY_BASIC) | MOD_RCTRL  | key )
#define LSHIFT(key)     ( KEY_TYPE(KEY_BASIC) | MOD_LSHIFT | key )
#define RSHIFT(key)     ( KEY_TYPE(KEY_BASIC) | MOD_RSHIFT | key )
#define LGUI(key)       ( KEY_TYPE(KEY_BASIC) | MOD_LGUI   | key )
#define RGUI(key)       ( KEY_TYPE(KEY_BASIC) | MOD_LGUI   | key )
#define LALT(key)       ( KEY_TYPE(KEY_BASIC) | MOD_LALT   | key )
#define RALT(key)       ( KEY_TYPE(KEY_BASIC) | MOD_LALT   | key )

#define LH(layer)       ( KEY_TYPE(KEY_LAYER_HOLD)           | LAYER(layer) )
#define LT(layer)       ( KEY_TYPE(KEY_LAYER_TOGGLE)         | LAYER(layer) )
#define LHT(layer)      ( KEY_TYPE(KEY_LAYER_HOLD_OR_TOGGLE) | LAYER(layer) )

#define DESKTOP(code)   ( KEY_TYPE(KEY_DESKTOP)  | code )
#define CONSUMER(key)   ( KEY_TYPE(KEY_CONSUMER) | key )

#define TD(index)       ( KEY_TYPE(KEY_TAPDANCE)          | index )
#define OSM(mod)        ( KEY_TYPE(KEY_ONE_SHOT_MODIFIER) | mod )

#define DM(mod, key)    ( KEY_TYPE(KEY_DUAL_MODIFIERS)          | mod | key )
#define DS(mod, key)    ( KEY_TYPE(KEY_SOLO_DUAL_MODIFIERS)     | mod | key )
#define THDM(mod, key)  ( KEY_TYPE(KEY_TAP_HOLD_DUAL_MODIFIERS) | mod | key )

#define DL(layer, key)   ( KEY_TYPE(KEY_DUAL_LAYER)          | LAYER(layer) | key )
#define DSL(layer, key)  ( KEY_TYPE(KEY_SOLO_DUAL_LAYER)     | LAYER(layer) | key )
#define THDL(layer, key) ( KEY_TYPE(KEY_TAP_HOLD_DUAL_LAYER) | LAYER(layer) | key )

#define MOUSE_BUTTON(button)    ( KEY_TYPE(KEY_MOUSE_BUTTON) | button )
#define MOUSE_CURSOR(direction) ( KEY_TYPE(KEY_MOUSE_CURSOR) | direction )

#define CAPS_WORD       ( KEY_TYPE(KEY_TOGGLE_CAPS_WORD) | KC_NO )

#define XXXXXXX         ( KEY_TYPE(KEY_BASIC) | KC_NO )
#define KEY_NO_ACTION   XXXXXXX
#define _______         KEY_TYPE(KEY_TRANSPARENT)

// Modifiers

#define KC_LCTL    KCM(MOD_LCTRL)
#define KC_LCTRL   KCM(MOD_LCTRL)
#define KC_LSFT    KCM(MOD_LSHIFT)
#define KC_LSHIFT  KCM(MOD_LSHIFT)
#define KC_LALT    KCM(MOD_LALT)
#define KC_LGUI    KCM(MOD_LGUI)
#define KC_RCTL    KCM(MOD_RCTRL)
#define KC_RCTRL   KCM(MOD_RCTRL)
#define KC_RSFT    KCM(MOD_RSHIFT)
#define KC_RSHIFT  KCM(MOD_RSHIFT)
#define KC_RALT    KCM(MOD_RALT)
#define KC_RGUI    KCM(MOD_RGUI)

#define KC_SCR_SVR CONSUMER(AL_LOCK)
#define KC_CALC    CONSUMER(AL_CALCULATOR)
#define KC_VOLUP   CONSUMER(AUDIO_VOL_UP)
#define KC_VOLDN   CONSUMER(AUDIO_VOL_DOWN)
#define KC_MUTE    CONSUMER(AUDIO_MUTE)

#define KC_TILDE LSHIFT(KC_GRAVE) // ~
#define KC_TILD KC_TILDE

#define KC_EXCLAIM LSHIFT(KC_1) // !
#define KC_EXLM KC_EXCLAIM

#define KC_AT LSHIFT(KC_2) // @

#define KC_HASH LSHIFT(KC_3) // #

#define KC_DOLLAR LSHIFT(KC_4) // $
#define KC_DLR KC_DOLLAR

#define KC_PERCENT LSHIFT(KC_5) // %
#define KC_PERC KC_PERCENT

#define KC_CIRCUMFLEX LSHIFT(KC_6) // ^
#define KC_CIRC KC_CIRCUMFLEX

#define KC_AMPERSAND LSHIFT(KC_7) // &
#define KC_AMPR KC_AMPERSAND

#define KC_ASTERISK LSHIFT(KC_8) // *
#define KC_ASTR KC_ASTERISK

#define KC_LEFT_PAREN LSHIFT(KC_9) // (
#define KC_LPRN KC_LEFT_PAREN

#define KC_RIGHT_PAREN LSHIFT(KC_0) // )
#define KC_RPRN KC_RIGHT_PAREN

#define KC_UNDERSCORE LSHIFT(KC_MINUS) // _
#define KC_UNDS KC_UNDERSCORE

#define KC_PLUS LSHIFT(KC_EQUAL) // +

#define KC_LEFT_CURLY_BRACE LSHIFT(KC_LEFT_BRACKET) // {
#define KC_LCBR KC_LEFT_CURLY_BRACE

#define KC_RIGHT_CURLY_BRACE LSHIFT(KC_RIGHT_BRACKET) // {
#define KC_RCBR KC_RIGHT_CURLY_BRACE

#define KC_LEFT_ANGLE_BRACKET LSHIFT(KC_COMMA) // <
#define KC_LABK KC_LEFT_ANGLE_BRACKET

#define KC_RIGHT_ANGLE_BRACKET LSHIFT(KC_DOT) // >
#define KC_RABK KC_RIGHT_ANGLE_BRACKET

#define KC_COLON LSHIFT(KC_SEMICOLON) // :
#define KC_COLN KC_COLON

#define KC_PIPE LSHIFT(KC_BACKSLASH) // |

#define KC_QUESTION LSHIFT(KC_SLASH) // ?
#define KC_QUES KC_QUESTION

#define KC_DOUBLE_QUOTE LSHIFT(KC_QUOTE) // "
#define KC_DQUO KC_DOUBLE_QUOTE

#define KC_ENT  KC_ENTER
#define KC_ESC  KC_ESCAPE
#define KC_BSPC KC_BACKSPACE
#define KC_SPC  KC_SPACE
#define KC_MINS KC_MINUS
#define KC_EQL  KC_EQUAL
#define KC_LBRC KC_LEFT_BRACKET
#define KC_RBRC KC_RIGHT_BRACKET
#define KC_BSLS KC_BACKSLASH
#define KC_NUHS KC_NONUS_HASH
#define KC_SCLN KC_SEMICOLON
#define KC_QUOT KC_QUOTE
#define KC_GRV  KC_GRAVE
#define KC_COMM KC_COMMA
#define KC_SLSH KC_SLASH
#define KC_NUBS KC_NONUS_BACKSLASH

#define KC_PSCR KC_PRINT_SCREEN
#define KC_PAUS KC_PAUSE
#define KC_BRK  KC_PAUSE
#define KC_INS  KC_INSERT
#define KC_PGUP KC_PAGE_UP
#define KC_DEL  KC_DELETE
#define KC_PGDN KC_PAGE_DOWN
#define KC_RGHT KC_RIGHT
#define KC_APP  KC_APPLICATION
#define KC_EXEC KC_EXECUTE
#define KC_SLCT KC_SELECT
#define KC_AGIN KC_AGAIN
#define KC_PSTE KC_PASTE
#define KC_ERAS KC_ALTERNATE_ERASE
#define KC_SYRQ KC_SYSTEM_REQUEST
#define KC_CNCL KC_CANCEL
#define KC_CLR  KC_CLEAR
#define KC_PRIR KC_PRIOR
#define KC_RETN KC_RETURN
#define KC_SEPR KC_SEPARATOR
#define KC_CLAG KC_CLEAR_AGAIN
#define KC_CRSL KC_CRSEL
#define KC_EXSL KC_EXSEL

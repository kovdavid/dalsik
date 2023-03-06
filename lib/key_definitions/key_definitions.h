#pragma once

// Key types
enum key_types {
    KEY_NORMAL = 0x00,
    KEY_LAYER_PRESS,
    KEY_LAYER_TOGGLE,
    KEY_LAYER_TOGGLE_OR_HOLD,
    KEY_SYSTEM,
    KEY_MULTIMEDIA_0,
    KEY_MULTIMEDIA_1,
    KEY_MULTIMEDIA_2,
    KEY_TAPDANCE,
    KEY_ONE_SHOT_MODIFIER,
    KEY_DUAL_MOD,
    KEY_SOLO_DUAL_MOD,
    KEY_TIMED_DUAL_MOD,
    KEY_DUAL_LAYER,
    KEY_SOLO_DUAL_LAYER,
    KEY_TIMED_DUAL_LAYER,
    KEY_TOGGLE_CAPS_WORD,
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

#define KC(code)        ( KEY_TYPE(KEY_NORMAL) | code )
#define KCM(mod)        ( KEY_TYPE(KEY_NORMAL) | mod  )
#define LCTRL(key)      ( KEY_TYPE(KEY_NORMAL) | MOD_LCTRL  | key )
#define RCTRL(key)      ( KEY_TYPE(KEY_NORMAL) | MOD_RCTRL  | key )
#define LSHIFT(key)     ( KEY_TYPE(KEY_NORMAL) | MOD_LSHIFT | key )
#define RSHIFT(key)     ( KEY_TYPE(KEY_NORMAL) | MOD_RSHIFT | key )
#define LGUI(key)       ( KEY_TYPE(KEY_NORMAL) | MOD_LGUI   | key )
#define RGUI(key)       ( KEY_TYPE(KEY_NORMAL) | MOD_LGUI   | key )
#define LALT(key)       ( KEY_TYPE(KEY_NORMAL) | MOD_LALT   | key )
#define RALT(key)       ( KEY_TYPE(KEY_NORMAL) | MOD_LALT   | key )

#define LP(layer)       ( KEY_TYPE(KEY_LAYER_PRESS)          | LAYER(layer) )
#define LT(layer)       ( KEY_TYPE(KEY_LAYER_TOGGLE)         | LAYER(layer) )
#define LHT(layer)      ( KEY_TYPE(KEY_LAYER_TOGGLE_OR_HOLD) | LAYER(layer) )

#define KC_SYSTEM(code) ( KEY_TYPE(KEY_SYSTEM)       | code )
#define KC_M0(code)     ( KEY_TYPE(KEY_MULTIMEDIA_0) | code )
#define KC_M1(code)     ( KEY_TYPE(KEY_MULTIMEDIA_1) | code )
#define KC_M2(code)     ( KEY_TYPE(KEY_MULTIMEDIA_2) | code )

#define TD(index)       ( KEY_TYPE(KEY_TAPDANCE)          | index )
#define OSM(mod)        ( KEY_TYPE(KEY_ONE_SHOT_MODIFIER) | mod )

#define D(mod, key)     ( KEY_TYPE(KEY_DUAL_MOD)         | mod | key )
#define DS(mod, key)    ( KEY_TYPE(KEY_SOLO_DUAL_MOD)    | mod | key )
#define DT(mod, key)    ( KEY_TYPE(KEY_TIMED_DUAL_MOD)   | mod | key )

#define DL(layer, key)  ( KEY_TYPE(KEY_DUAL_LAYER)       | LAYER(layer) | key )
#define DSL(layer, key) ( KEY_TYPE(KEY_SOLO_DUAL_LAYER)  | LAYER(layer) | key )
#define DTL(layer, key) ( KEY_TYPE(KEY_TIMED_DUAL_LAYER) | LAYER(layer) | key )

#define CAPS_WORD       ( KEY_TYPE(KEY_TOGGLE_CAPS_WORD) )

#define XXXXXXX         ( KEY_TYPE(KEY_NORMAL) | KC_NO )
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

// Aliases

#define KC_SYS_POWER_OFF KC_SYSTEM(0x81)
#define KC_SYS_SLEEP     KC_SYSTEM(0x82)
#define KC_SYS_WAKE_UP   KC_SYSTEM(0x83)

#define KC_AUDIO_MUTE    KC_M0(0xE2)
#define KC_AUDIO_VOLUP   KC_M0(0xE9)
#define KC_AUDIO_VOLDOWN KC_M0(0xEA)

#define KC_LAUNCH_CALCULATOR    KC_M1(0x92)
#define KC_LAUNCH_WWW           KC_M1(0x94)
#define KC_LAUNCH_FILE_BROWSER  KC_M1(0x96)
#define KC_LAUNCH_SCREEN_SAVER  KC_M1(0x9E)

#define KC_SCR_SVR KC_LAUNCH_SCREEN_SAVER
#define KC_CALC KC_LAUNCH_CALCULATOR
#define KC_VOLUP KC_AUDIO_VOLUP
#define KC_VOLDN KC_AUDIO_VOLDOWN
#define KC_MUTE KC_AUDIO_MUTE

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

#include "keymap.h"

#define KEY0 KC_LCTRL
#define KEY1 D(MOD_LCTRL, KC_C)
#define KEY2 D(MOD_LSHIFT, KC_D)
#define KEY3 DT(MOD_LSHIFT, KC_E)
#define KEY4 DT(MOD_LCTRL, KC_F)
#define KEY5 DL(1, KC_G)
#define KEY6 DSL(1, KC_H)
#define KEY7 OSM(MOD_LCTRL)
#define KEY8 LHT(1)
#define KEY9 LP(1)

const uint32_t keymap[][KEYBOARD_ROWS][KEYBOARD_COLS] = KEYMAP({
    LAYOUT_4x12(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
         KEY0  ,  KC_A  ,  KC_B  ,  KEY1  ,  KEY2  ,  KEY3  ,  KEY4  ,  KEY5  ,  KEY6  ,  KEY7  ,  KEY8  ,  KEY9  ,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
    ),
    LAYOUT_4x12(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______,  KC_E  , _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
    )
});

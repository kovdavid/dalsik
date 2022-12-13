#include "keymap.h"
#include "combos.h"

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

#define KEY10 DL(1, KC_H)
#define KEY11 DL(1, KC_I)

const KeyCoords combo1[] PROGMEM = { { 3, 0 }, { 3, 7 } };
const KeyCoords combo2[] PROGMEM = { { 3, 0 }, { 3, 1 }, { 3, 2 } };
const KeyCoords combo3[] PROGMEM = { { 3, 0 }, { 3, 1 }, { 3, 3 } };
const KeyCoords combo4[] PROGMEM = { { 3, 0 }, { 3, 1 }, { 3, 3 }, { 3, 4 } };
const KeyCoords combo5[] PROGMEM = { { 3, 5 }, { 3, 6 } };

ComboState combos[] = COMBOS({
    COMBO(combo1, KC_A),
    COMBO(combo2, KC_B),
    COMBO(combo3, KC_C),
    COMBO(combo4, KC_D),
    COMBO(combo5, KC_LCTRL),
});

const uint32_t keymap[][KEYBOARD_ROWS][KEYBOARD_COLS] = KEYMAP({
    LAYOUT_4x12(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
         KEY0  ,  KC_A  ,  KC_B  ,  KEY1  ,  KEY2  ,  KEY3  ,  KEY4  ,  KEY5  ,  KEY6  ,  KEY7  ,  KEY8  ,  KEY9  ,
         KEY10 ,  KEY11 , _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        // Combo row
          KC_Q ,   KC_W ,   KC_R , _______, _______, _______, _______, _______, _______, _______, _______, _______
    ),
    LAYOUT_4x12(
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______,  KC_E  , _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
         KEY10 ,  KEY11 , _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        // Combo row
          KC_Q ,   KC_W ,   KC_R , _______, _______, _______, _______, _______, _______, _______, _______, _______
    )
});

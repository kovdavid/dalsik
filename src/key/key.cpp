#include "key.h"
#include "key_definitions.h"
#include "dalsik.h"

Key::Key(void)
    : Key(COORDS_INVALID)
{}

Key::Key(KeyCoords coords)
    : type(KEY_BASIC)
    , layer(0)
    , modifiers(MOD_CLEAR)
    , code(KC_NO)
    , coords(coords)
{}

Key::Key(uint32_t progmem_data, KeyCoords coords)
    : coords(coords)
{
    this->type      = (progmem_data >> 24) & 0xFF;
    this->layer     = (progmem_data >> 16) & 0xFF;
    this->modifiers = (progmem_data >> 8)  & 0xFF;
    this->code      = (progmem_data >> 0)  & 0xFF;
}

Key::Key(uint8_t type, uint8_t layer, uint8_t modifiers, uint8_t code, KeyCoords coords)
    : type(type)
    , layer(layer)
    , modifiers(modifiers)
    , code(code)
    , coords(coords)
{}

Key Key::clone_and_keep_layer() {
    return Key(this->type, this->layer, MOD_CLEAR, KC_NO, this->coords);
}

Key Key::clone_and_keep_modifiers() {
    return Key(this->type, 0, this->modifiers, KC_NO, this->coords);
}

Key Key::clone_and_keep_code() {
    return Key(this->type, 0, MOD_CLEAR, this->code, this->coords);
}

void Key::print_internal_state() {
    Serial.print(">> KEY ROW:");
    Serial.print(this->coords.row);
    Serial.print(" COL:");
    Serial.print(this->coords.col);
    Serial.print(" TYPE:");
    Serial.print(this->type);
    Serial.print(" LAYER:");
    Serial.print(this->layer);
    Serial.print(" MOD:");
    Serial.print(this->modifiers);
    Serial.print(" CODE:");
    Serial.print(this->code);
    Serial.print("\n");
}

bool Key::is_any_dual_modifiers_key() {
    return
        this->type == KEY_DUAL_MODIFIERS
        || this->type == KEY_SOLO_DUAL_MODIFIERS
        || this->type == KEY_TAP_HOLD_DUAL_MODIFIERS;
}

bool Key::is_any_dual_layer_key() {
    return
        this->type == KEY_DUAL_LAYER
        || this->type == KEY_SOLO_DUAL_LAYER
        || this->type == KEY_TAP_HOLD_DUAL_LAYER;
}

bool Key::is_any_dual_key() {
    return
        this->is_any_dual_modifiers_key()
        || this->is_any_dual_layer_key();
}

bool Key::equals(Key other) {
    return
        this->type == other.type
        && this->layer == other.layer
        && this->modifiers == other.modifiers
        && this->code == other.code
        && this->coords.equals(other.coords);
}

# Key Types and features

The definition of different key types is in [key_definitions.h](https://github.com/DavsX/dalsik/blob/master/key_definitions.h) . These codes are used to give different functionality for keys. Each key is encoded as 2 bytes in the [EEPROM](eeprom_layout.md). The first byte is the type, the second is the value (which can differ for each type).

# Layers

Each key (for a given row + column coordinate) have it's function (e.g. "key a", "key 2" etc.). Layering is one of the biggest reason I made this project. It enables us to define a different function for the same key for different layers.

For example a key on row 1 and column 1 can act as "key a" on the default (0) layer, "key 2" on layer 1, "key @" on layer 2 and "volume up" on layer 3. This enables me to have all the keys I need on just 48 physical keys.

Layers can be switched by pressing keys, which have a layer changing key type assigned to them.

# Tapdance



# Key types

The supported key types of Dalsik are the following:

* `#define KEY_UNSET 0x00`

  * the key is not mapped to anything; pressing the key does nothing

* `#define KEY_NORMAL 0x01`

  * basic keys, like modifiers (CTRL, ALT, SHIFT, GUI), letters, numbers etc.

  * e.g. "a" is `KEY_NORMAL - KC_A`, so it will be encoded in memory as `0x01 0x04`

    ```
    #define KC_A 0x04
    ```

* `#define KEY_DUAL_LCTRL 0x02`

  * If the key is pressed and a different key is pressed before releasing this one, then this key acts as left CTRL
    * Pressing `KEY_DUAL_LCTRL` -> Pressing "a" -> Releasing "a" -> Releasing  `KEY_DUAL_LCTRL` sends "CTRL+a"
  * If the key is pressed on its own, then it acts as `KEY_NORMAL` with this keys value.
    * So a key `0x02 0x04` act as `KEY_NORMAL - KC_A` when pressed on its own

* `#define KEY_DUAL_RCTRL 0x03`

* `#define KEY_DUAL_LSHIFT 0x04`

* `#define KEY_DUAL_RSHIFT 0x05`

* `#define KEY_DUAL_LGUI 0x06`

* `#define KEY_DUAL_RGUI 0x07`

* `#define KEY_DUAL_LALT 0x08`

* `#define KEY_DUAL_RALT 0x09`

* `#define KEY_LAYER_PRESS 0x0A`

  * Changes the keyboard's layer, while this key is held down
  * `0x0A 0x01` changes layer to 1, `0x0A 0x02` changes layer to 2 etc.

* `#define KEY_LAYER_TOGGLE 0x0B`

  * Toggles the keyboard's layer when tapped. The changed layer remains active even after releasing this key. The layer is deactivated, when this key is tapped again.

* `#define KEY_LAYER_HOLD_OR_TOGGLE 0x0C`

  * Combines `KEY_LAYER_PRESS` and `KEY_LAYER_TOGGLE`.
  * When tapped on its own, this key acts like `KEY_LAYER_TOGGLE`
  * When pressed with a different key, then this key acts like `KEY_LAYER_PRESS`

* `#define KEY_WITH_MOD_LCTRL 0x0D`

  * This key sends its value as `KEY_NORMAL` but also activates `LCTRL`
  * The key `0x0D 0x04` sends "CTRL-a" using a single key

* `#define KEY_WITH_MOD_RCTRL 0x0E`

* `#define KEY_WITH_MOD_LSHIFT 0x0F`

* `#define KEY_WITH_MOD_RSHIFT 0x10`

* `#define KEY_WITH_MOD_LGUI 0x011`

* `#define KEY_WITH_MOD_RGUI 0x12`

* `#define KEY_WITH_MODL_LALT 0x13`

* `#define KEY_WITH_MOD_RALT 0x14`

* `#define KEY_SYSTEM 0x15`

  * System keys (power-off, sleep, wake-up etc)

* `#define KEY_MULTIMEDIA_0 0x16`

  * Multimedia keys with '0x00' prefix - mute, volume up, volume down etc.

* `#define KEY_MULTIMEDIA_0 0x17`

  * Multimedia keys with '0x01' prefix - application launch (calculator, browser) etc.

* `#define KEY_MULTIMEDIA_0 0x18`

  * Multimedia keys with '0x02' prefix - application control (Save, Exit, Open) etc.

* `#define KEY_TAPDANCE 0x19`

  * Key `0x19 0x01` activates tapdance 1, `0x19 0x02` tapdance 2 etc.


# Key Types and features

The different supported key types and key codes are defined in [key_definitions.h](https://github.com/DavsX/dalsik/blob/master/key_definitions.h). The USB HID standard uses 1 byte for key codes, but Dalsik uses 2 bytes to encode each key to support additional features. The first byte is the type, the second is the value (which can differ for each type - can be a HID key code, or something special).

# Layers

Each key (for a given row + column coordinate) have it's function (e.g. "key a", "key 2" etc.). Layering is one of the biggest reason I made this project. It enables us to define a different function for the same key for different layers.

For example a key on row 1 and column 1 can act as "key a" on the default (0) layer, "key 2" on layer 1, "key @" on layer 2 and "volume up" on layer 3. This enables me to have all the keys I need on just 48 physical keys.

Layers can be switched by pressing keys, which have a layer changing key type assigned to them.

# TapDance

Tapdance is a functionality, that allows to define multiple keys/functionality for a single key. It is similar to layers, but while layers require to press an additional key (for switching layers) to get a different behavior for a given key, TapDance works by tapping a given key multiple times in a given time period.

```c++
// dalsik.h
#define TAPDANCE_TIMEOUT_MS 300
```

If I tap a TapDance key only once, then the first functionality is triggered. If I tap a TapDance key twice, then the second one is triggered. Up to three different functionality can be mapped to a single key. (e.g. single tap - "CTRL+c", double tab - "CTRL+v")

As each key takes 2 bytes in the EEPROM and a TapDance key can be configured for 3 different keys, the TapDance key mapping is located at a specific offset in the EEPROM, while each group is indexed, starting from 0.

# Dual keys

Several types of dual keys are supported, which can do 2 different things. They can act as a modifier or layer press on some occasions and as a normal key as well.

Basically when such a dual key is pressed and released on its own, then it acts as a normal key. The same is true, when this given key is pressed for more, than `DUAL_MODE_TIMEOUT_MS` milliseconds (see `dalsik.h`). These two conditions handle taps and long presses (which then trigger repeated "press" of the key by the operating system).

The secondary functionality (modifier, layer press) is activated only when a different key is pressed after pressing the dual key before `DUAL_MODE_TIMEOUT_MS` milliseconds.

# Key types

The supported key types of Dalsik are the following:

* `#define KEY_UNSET 0x00`

  * the key is not mapped to anything; pressing the key does nothing

* `#define KEY_NORMAL 0x01`

  * basic keys, like modifiers (CTRL, ALT, SHIFT, GUI), letters, numbers etc.

  * The [USB HID Usage tables](http://www.usb.org/developers/hidpage/Hut1_12v2.pdf) document contains all the HID keycodes

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

  * Key `0x19 0x01` activates tapdance 0, `0x19 0x02` tapdance 1 etc.

* `#define KEY_DUAL_LAYER_1 0x1A`

  * When tapped on its own, this key acts like `KEY_NORMAL`
  * When pressed with a different key, then this key acts like `KEY_LAYER_PRESS`
  * e.g. `0x1A 0x04` is "a" on its own and "Layer press 1" when held down with a different key

* `#define KEY_DUAL_LAYER_2 0x1B`

* `#define KEY_DUAL_LAYER_3 0x1C`

* `#define KEY_DUAL_LAYER_4 0x1D`

* `#define KEY_DUAL_LAYER_5 0x1E`

* `#define KEY_DUAL_LAYER_6 0x1F`

* `#define KEY_DUAL_LAYER_7 0x20`

* `#define KEY_DUAL_SINGLE_LCTRL 0x28`

  * `DUAL_SINGLE` keys can act as a modifier (in this case as a `KEY_DUAL_LCTRL`) only, when they are the first keys to be pressed. If such a key is pressed after another key, then the primary key is activated right away.

* `#define KEY_DUAL_SINGLE_RCTRL 0x29`

* `#define KEY_DUAL_SINGLE_LSHIFT 0x2A`

* `#define KEY_DUAL_SINGLE_RSHIFT 0x2B`

* `#define KEY_DUAL_SINGLE_LGUI 0x2C`

* `#define KEY_DUAL_SINGLE_RGUI 0x2D`

* `#define KEY_DUAL_SINGLE_LALT 0x2E`

* `#define KEY_DUAL_SINGLE_RALT 0x2F`

* `#define KEY_DUAL_LAYER_SINGLE_1 0x21`

  * `DUAL_SINGLE_LAYER` keys can act as a layer press (in this case as a `KEY_DUAL_LAYER_1`) only, when they are the first keys to be pressed. If such a key is pressed after another key, then the primary key is activated right away.

* `#define KEY_DUAL_LAYER_SINGLE_2 0x22`

* `#define KEY_DUAL_LAYER_SINGLE_3 0x23`

* `#define KEY_DUAL_LAYER_SINGLE_4 0x24`

* `#define KEY_DUAL_LAYER_SINGLE_5 0x25`

* `#define KEY_DUAL_LAYER_SINGLE_6 0x26`

* `#define KEY_DUAL_LAYER_SINGLE_7 0x27`

* `#define KEY_TRANSPARENT 0xFF`

  * This makes the current key act like the key from a previous layer
  * If I set the key at layer 0, row 1, column 1 to be `KC_A` and the same key on layer 1 to be `KEY_TRANSPARENT`, then the key on layer 1 will act as `KC_A`
  * Useful, if I don't want to change a given key on the next layer, but also don't want to duplicate its value


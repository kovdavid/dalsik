#pragma once

#define BIT_SET(base, mask) (base |= mask)
#define BIT_CLEAR(base, mask) (base &= ~(mask))
#define BIT_TOGGLE(base, mask) (base ^= mask)

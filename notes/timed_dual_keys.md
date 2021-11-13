# Glossary

* Timed dual keys TDK
* Dual keys DK
* 'A' key - TDK(CTRL, A)
    * primary function - KC_A
    * secondary function - KC_CTRL
* 'S' key - KC(S)
* 'D' key - TDK(SHIFT, D)

# Intro

Different from DK in that pressing another button while the DK is pressed does not automatically activate
the secondary function of DK. The motivation for TDK is to use them as home-row modifiers, which keys are
frequently pressed, so quickly pressing 'A', then 'S', then releasing them should write 'AS' and the 'S'
should not trigger the secondary function of 'A'.

# 'AS' problem

When quickly pressing 'A', then 'S' then releasing 'A', then 'S' writes 'sa' instead of 'as', as the press
event of the primary function of the 'A' key has a delay.

So when a TDK key is pressed and it's function is not yet determined (primary/secondary) then we need to
buffer other key presses and process them later.

# 'ADS' problem

When pressing 'A', then 'D', then 'S' - what to do? send CTRL+SHIFT+S, or CTRL+D+S?

# Brainstorming

What to do, when we tap a TDK key? send the primary function (e.g. `KC(A)`).
What to do, when we hold it long by itself? Send the primary or secondary function?
Theoretically it could be like this: when you hold the key and press a different special key, we could force the secondary function (i.e. hold TDK + tap space).
Or we could move the secondary keys on a layer? I.e. hold backspace and asdf turn to TDK. Hold down A+S, release backspace and you are in CTRL+SHIFT mode. Then press P and you get CTRL+SHIFT+P

## Use cases

* `min_delay` = 20ms
    * if the key after TDK is pressed in less than this value, then we trigger the primary function on the TDK key
* `timeout` = 50ms
    * if the last pending TDK key is held for this long, we trigger the secondary function on it

### Use case 1

In order for the secondary function to trigger there must be at least `min_delay` on KC(D)

QUEUE=[ TDK(CTRL,A)               ] SENT=[  ]
QUEUE=[ TDK(CTRL,A) =10ms=> KC(D) ] SENT=[ A => A+D ]

### Use case 2

KC(D) was pressed after `min_delay`, so the TDK function was resolved based on
the release event of KC(D)

QUEUE=[ TDK(CTRL,A)               ] SENT=[  ]
QUEUE=[ TDK(CTRL,A) =30ms=> KC(D) ] SENT=[  ]
QUEUE=[ TDK(CTRL,A) =30ms=> XRELX ] SENT=[ CTRL => CTRL+D => CTRL ]

### Use case 3

Similar to `EX1 US2`, but the TDK key was release first, so we don't trigger the secondary function on TDK

QUEUE=[ TDK(CTRL,A)               ] SENT=[  ]
QUEUE=[ TDK(CTRL,A) =30ms=> KC(D) ] SENT=[  ]
QUEUE=[ XXRELEASEXX =30ms=> KC(D) ] SENT=[ A => A+D => D ]

### Use case 4

KC(D) was pressed after `timeout`, so the secondary function on TDK was used

QUEUE=[ TDK(CTRL,A)               ] SENT=[  ]
QUEUE=[ TDK(CTRL,A) =90ms=> KC(D) ] SENT=[ CTRL => CTRL+D ]

### Use case 5

TDK timeout in this case should be counted from the last pending TDK key.
Since KC(D) was pressed before `min_delay`, we send the primary keys.

QUEUE=[ TDK(CTRL,A)                                   ] SENT=[  ]
QUEUE=[ TDK(CTRL,A) =10ms=> TDK(SHFT,S)               ] SENT=[  ]
QUEUE=[ TDK(CTRL,A) =10ms=> TDK(SHFT,S) =10ms=> KC(D) ] SENT=[ A => A+S => A+S+D ]

### Use case 6

Multiple TDK from the left set of modifiers were hold for more than `timeout`,
so activate the secondary function

QUEUE=[ TDK(LCTRL,A)                                 ] SENT=[  ]
QUEUE=[ TDK(LCTRL,A) =10ms=> TDK(LSHFT,S) =timeout=> ] SENT=[ CTRL => CTRL+SHIFT ]











QUEUE=[ TDK(CTRL,A)                                   ] SENT=[  ]
QUEUE=[ TDK(CTRL,A) =10ms=> TDK(SHFT,S)               ] SENT=[  ]
QUEUE=[ TDK(CTRL,A) =10ms=> TDK(SHFT,S) =30ms=> KC(D) ] SENT=[  ]







releasing D (the last pressed key - at the end of the buffer) should trigger the secondary function
of A and S

QUEUE=[ TDK(CTRL,A) =10ms=> TDK(SHFT,S) , KC(D) ] SENT=[ ]
QUEUE=[ TDK(CTRL,_) =10ms=> TDK(SHFT,_) , XRELX ] SENT=[ CTRL => CTRL+SHIFT => CTRL+SHFT+D => CTRL+SHFT ]
QUEUE=[ TDK(CTRL,_)  , XXRELEASEXX ,       ] SENT=[ CTRL ]
QUEUE=[ XXRELEASEXX ,             ,       ] SENT=[ ]

*Releasing a button affects the buttons before it in the queue*

The CTRL/SHFT keys should not switch back to the primary key anymore

### Use case 2

keys are being released in the same order as they were pressed. Releasing A should send 'A';
releasing S should send 'D'

QUEUE=[ TDK(CTRL,A) , TDK(SHFT,S) , KC(D) ] SENT=[ ]
QUEUE=[ XXRELEASEXX , TDK(SHFT,S) , KC(D) ] SENT=[ A ]
QUEUE=[             , XXRELEASEXX , KC(D) ] SENT=[ A => S ]
QUEUE=[             ,             , XRELX ] SENT=[ A => S => D ]

### Use case 3

Releasing A should send 'A'. Releasing D should trigger the secondary function of S

*Releasing a button affects the buttons before it in the queue*

QUEUE=[ TDK(CTRL,A) , TDK(SHFT,S) , KC(D) ] SENT=[ ]
QUEUE=[ XXRELEASEXX , TDK(SHFT,S) , KC(D) ] SENT=[ A ]
QUEUE=[             , TDK(SHFT,S) , XRELX ] SENT=[ A => SHFT+D ]
QUEUE=[             , XXRELEASEXX ,       ] SENT=[ A => SHFT+D ]

### Use case 4

By releasing S we need to decide what to do with it. As another TDK is being pressed (A),
we activate the primary function of S and activate secondary function on A

QUEUE=[ TDK(CTRL,A) , TDK(SHFT,S) , KC(D) ] SENT=[ ]
QUEUE=[ TDK(CTRL,A) , XXRELEASEXX , KC(D) ] SENT=[ CTRL+S ]
QUEUE=[ TDK(CTRL,A) ,             , XRELX ] SENT=[ CTRL+S => CTRL+D ]
QUEUE=[ XXRELEASEXX ,             ,       ] SENT=[ CTRL+S => CTRL+D ]

### Use case 5

QUEUE=[ TDK(CTRL,A) , TDK(SHFT,S) , KC(D) ] SENT=[ ]
QUEUE=[ TDK(CTRL,A) , XXRELEASEXX , KC(D) ] SENT=[ CTRL+S ]
QUEUE=[ XXRELEASEXX ,             , KC(D) ] SENT=[ CTRL+S => D ]
QUEUE=[             ,             , XRELX ] SENT=[ CTRL+S => D ]

### Use case 6

QUEUE=[ TDK(CTRL,A) , TDK(SHFT,S) , KC(D) ] SENT=[ ]
QUEUE=[ TDK(CTRL, ) , TDK(SHFT,S) , KC(D) ] SENT=[ ] // Timeout for TDK(CTRL,A)
QUEUE=[ XXRELEASEXX , TDK(SHFT, ) , KC(D) ] SENT=[ ] // Timeout for TDK(SHFT,S)
QUEUE=[ XXRELEASEXX , XXRELEASEXX , KC(D) ] SENT=[ ]

### Use case 7

QUEUE=[ TDK(CTRL,A) , TDK(SHFT,S) , KC(D), KC(E) ] SENT=[ ]
QUEUE=[ TDK(CTRL,A) , TDK(SHFT,S) , XRELX, KC(E) ] SENT=[ CTRL+SHIFT+D => CTRL+SHIFT+E ]

### Use case 8

QUEUE=[ TDK(CTRL,A), KC(D)         ] SENT=[]
QUEUE=[ TDK(CTRL,A), KC(D) , KC(E) ] SENT=[]
QUEUE=[ TDK(CTRL,A), KC(D) , XRELX ] SENT=[ CTRL+D => CTRL+D+E ] // Press CTRL+D, then CTRL+D+E, then release E and send CTRL+D
QUEUE=[ TDK(CTRL,A), XRELX ,       ] SENT=[ release CTRL+D ]

### Use case 9

QUEUE=[ TDK(CTRL,A), KC(D)         ] SENT=[]
QUEUE=[ TDK(CTRL,A), KC(D) , KC(E) ] SENT=[]
QUEUE=[ TDK(CTRL,A), XRELX , KC(E) ] SENT=[]
QUEUE=[ TDK(CTRL,A),       , XRELX ] SENT=[]

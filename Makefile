DEVICE=$(shell ls /dev/ttyACM*)

# crt-t + q => quit tio session
tio:
	tio -m INLCRNL -l /tmp/tio.log $(DEVICE)

upload:
	arduino --board SparkFun:avr:promicro:cpu=16MHzatmega32U4 --upload dalsik.ino --verbose --port $(DEVICE)

verify:
	arduino --board SparkFun:avr:promicro:cpu=16MHzatmega32U4 --verify dalsik.ino --verbose --port $(DEVICE)

clear_keymap:
	./utils/cmd_clear_keymap.pl

set_keymap:
	./utils/set_keymap.pl -j ~/dotfiles/dalsik_layout/dalsik_layout_V3.json

render_keymap:
	./utils/cmd_get_keymap.pl | ./utils/render_keymap.pl

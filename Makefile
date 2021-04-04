tio:
	tio -l /tmp/tio.log /dev/ttyACM0

upload:
	arduino --board sparkfun:avr:promicro:cpu=16MHzatmega32U4 --upload dalsik.ino --verbose --preserve-temp-files --port /dev/ttyACM0

verify:
	arduino --board sparkfun:avr:promicro:cpu=16MHzatmega32U4 --verify dalsik.ino --verbose --preserve-temp-files --port /dev/ttyACM0

clear_keymap:
	./utils/cmd_clear_keymap.pl

set_keymap: clear_keymap
	./utils/set_keymap.pl -j ~/dotfiles/dalsik_layout/dalsik_layout_V3.json

render_keymap:
	./utils/cmd_get_keymap.pl | ./utils/render_keymap.pl

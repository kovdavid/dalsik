DEVICE=$(shell ls /dev/ttyACM*)
BOARD=SparkFun:avr:promicro:cpu=16MHzatmega32U4

verify:
	make -C src

upload:
	make -C src upload

Arduino-Makefile:
	git submodule add --force https://github.com/WeAreLeka/Arduino-Makefile Arduino-Makefile

acutest:
	mkdir -p test/acutest
	wget -O test/acutest/acutest.h https://raw.githubusercontent.com/mity/acutest/master/include/acutest.h

# crt-t + q => quit tio session
tio:
	tio -m INLCRNL -l /tmp/tio.log $(DEVICE)

clean:
	rm -rf build/

old_verify:
	arduino --board $(BOARD) --verify dalsik.ino --verbose --port $(DEVICE)

old_upload:
	arduino --board $(BOARD) --upload dalsik.ino --verbose --port $(DEVICE)

clear_keymap:
	./utils/cmd_clear_keymap.pl

set_keymap:
	./utils/set_keymap.pl -j ~/dotfiles/dalsik_layout/dalsik_layout_V3.json

render_keymap:
	./utils/cmd_get_keymap.pl | ./utils/render_keymap.pl

ctags:
	ctags -f tags -R --extra=q .

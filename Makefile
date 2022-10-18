DEVICE=$(shell ls /dev/ttyACM*)

clean_verify: clean verify

verify:
	make -C src

upload:
	make -C src upload

Arduino-Makefile:
	git submodule add --force https://github.com/WeAreLeka/Arduino-Makefile Arduino-Makefile

acutest:
	mkdir -p test/acutest
	wget -O test/acutest/acutest.h https://raw.githubusercontent.com/mity/acutest/master/include/acutest.h

# ctrl-t + q => quit tio session
tio:
	tio --map INLCRNL --log-file /tmp/tio.log $(DEVICE)

clean:
	rm -rf build/

test:
	make -f Makefile.unittest test

clear_keymap:
	./utils/cmd_clear_keymap.pl

set_keymap:
	./utils/set_keymap.pl -j ~/dotfiles/dalsik_layout/dalsik_layout.json

render_keymap:
	./utils/cmd_get_keymap.pl | ./utils/render_keymap.pl

ctags:
	ctags -f tags -R --extras=+q .

.PHONY: test

quick_verify: partial_clean verify

clean_verify: clean verify

clean_upload: clean verify upload clean

verify_checksum:
	@echo -n "Calculated checksum: "
	@./utils/checksum.sh
	@echo -n "Received checksum: "
	@./utils/cmd_get_checksum.pl

verify:
	make -f Makefile.release verify

upload:
	make -f Makefile.release upload

# ctrl-t + q => quit tio session
tio:
	tio --map INLCRNL --log-file /tmp/tio.log $(shell ls /dev/ttyACM*)

partial_clean:
	rm -rf build/release/dalsik/
	rm -rf build/release/dalsik.*
	rm -rf build/release/libcore.a

clean:
	rm -rf build/

test:
	make -f Makefile.unittest test

ctags:
	ctags -f tags -R --extras=+q --languages=C,C++ src/

.PHONY: test

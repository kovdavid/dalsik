DEVICE=$(shell ls /dev/ttyACM*)

clean_verify: clean verify

clean_upload: clean verify upload clean

verify_checksum:
	@echo -n "Calculated checksum: "
	@./utils/checksum.sh
	@echo -n "Received checksum: "
	@./utils/cmd_get_checksum.pl

verify:
	DALSIK_CHECKSUM=$(shell ./utils/checksum.sh) make -f Makefile.compile verify

upload:
	$(PRE_UPLOAD_COMMAND)
	make -f Makefile.compile upload
	$(POST_UPLOAD_COMMAND)

acutest:
	mkdir -p test/acutest
	wget -O test/acutest/acutest.h https://raw.githubusercontent.com/mity/acutest/master/include/acutest.h

# ctrl-t + q => quit tio session
tio:
	tio --map INLCRNL --log-file /tmp/tio.log $(DEVICE)

clean:
	rm -rf build/
	rm -f tags

test:
	make -f Makefile.unittest test

ctags:
	ctags -f tags -R --extras=+q --languages=C,C++ src/

.PHONY: test

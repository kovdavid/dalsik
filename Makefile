DEVICE=$(shell ls /dev/ttyACM*)
CHECKSUM_FILE := "lib/dalsik/dalsik_checksum.h"

clean_verify: clean verify

clean_upload: clean verify upload clean

$(CHECKSUM_FILE):
	./utils/checksum.sh --header > $@

verify_checksum:
	@echo -n "Calculated checksum: "
	@./utils/checksum.sh
	@echo -n "Received checksum: "
	@./utils/cmd_get_checksum.pl

verify: $(CHECKSUM_FILE)
	make -C src

upload:
	$(PRE_UPLOAD_COMMAND)
	make -C src upload
	$(POST_UPLOAD_COMMAND)

Arduino-Makefile:
	git submodule add --force https://github.com/DavsX/Arduino-Makefile Arduino-Makefile

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

ctags:
	ctags -f tags -R --extras=+q --languages=C,C++ .

.PHONY: test $(CHECKSUM_FILE)

tio:
	tio -l /tmp/tio.log /dev/ttyACM0

upload:
	arduino --board sparkfun:avr:promicro:cpu=16MHzatmega32U4 --upload dalsik.ino --verbose --preserve-temp-files --port /dev/ttyACM0

verify:
	arduino --board sparkfun:avr:promicro:cpu=16MHzatmega32U4 --verify dalsik.ino --verbose --preserve-temp-files --port /dev/ttyACM0

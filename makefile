.PHONY: all test monitor

all: test

monitor:
	pio device monitor

test:
	pio test -e native

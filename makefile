.PHONY: all test monitor examples shapshot ss

all: test

monitor:
	pio device monitor

test:
	pio test -e native

examples:
	./build-examples.sh

shapshot:
	python shapshot.py

ss: shapshot

.PHONY: all test monitor examples shapshot ss lint

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

lint:
	./lint.sh

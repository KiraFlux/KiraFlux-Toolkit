.PHONY: all monitor test build-examples be shapshot ss lint new-example ne

all: test

monitor:
	pio device monitor

test:
	pio test -e native

build-examples:
	./build-examples.sh

be: build-examples

shapshot:
	python shapshot.py

ss: shapshot

lint:
	./lint.sh

name ?= new-example

new-example:
	python new-example.py $(name)

ne: new-example
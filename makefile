.PHONY: all monitor test build-examples be shapshot ss lint new-example ne new-test nt

name ?= unnamed # new example/test name
group ?= common # test group

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

new-example:
	python new-template.py example $(name)

ne: new-example

new-test:
	python new-template.py test $(group) $(name)

nt: new-test

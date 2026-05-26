.PHONY: all monitor test build-examples be snapshot s lint new-example ne new-test nt changelog

name ?= unnamed # target example/test name
group ?= common # target example/test group

all: test

monitor:
	pio device monitor

test:
	pio test -e native

build-examples:
	./build-examples.sh

be: build-examples

snapshot:
	python snapshot.py

s: snapshot

format:
	./clang-format-all.sh

f: format

lint:
	./lint.sh

new-example:
	python new-template.py example $(group) $(name)

ne: new-example

new-test:
	python new-template.py test $(group) $(name)

nt: new-test

changelog:
	git log main..dev > log.txt && git diff main..dev > diff.txt
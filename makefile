.PHONY: all monitor test lint new-example ne new-test nt diff

name ?= unnamed # target example/test name
group ?= common # target example/test group

all: test

monitor:
	pio device monitor

test:
	pio test -e native

lint:
	./lint.sh

new-example:
	python new-template.py example $(group) $(name)

ne: new-example

new-test:
	python new-template.py test $(group) $(name)

nt: new-test

diff:
	git log main..dev > log.txt && git diff main..dev > diff.txt
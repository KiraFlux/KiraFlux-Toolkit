all: build_esp32

build_native:
	pio run -e native

run_native: build_native
	./.pio/build/native/program	

n: run_native

build_esp32:
	pio run -e esp32dev

b: build_esp32

upload:
	pio run -e esp32dev --target upload

u: upload

monitor:
	pio device monitor

m: monitor
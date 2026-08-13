# Copyright (c) 2026 KiraFlux
# SPDX-License-Identifier: MIT
#
# Makefile for PlatformIO projects (ESP32 / native)
# Provides shortcuts for build, run, upload, monitor, clean, and port detection.
# See 'make help' for usage.

ENV_NATIVE := native
ENV_ESP32  := esp32dev
BUILD_DIR  := .pio/build
PROG_NATIVE := $(BUILD_DIR)/$(ENV_NATIVE)/program

BOLD   := \033[1m
YELLOW := \033[33m
CYAN   := \033[36m
RESET  := \033[0m

ENV ?= $(ENV_ESP32)
PORT ?= $(shell ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null | head -1)

.PHONY: all build b run r upload u monitor m clean c help h list l

all: build

build:
	pio run -e $(ENV)

b: build

run:
	pio run -e $(ENV_NATIVE) --target upload

r: run

upload: 
	pio run -e $(ENV_ESP32) --target upload --upload-port $(PORT) 

u: upload

monitor:
	pio device monitor --port $(PORT) --no-reconnect

m: monitor

clean:
	pio run --target clean -e $(ENV)

c: clean

list:
	@pio device list --serial | grep -E '^/dev/tty(USB|ACM)' || printf "No USB/ACM serial ports found\n"

l: list

help:
	@printf "$(BOLD)Usage: make [target] [ENV=name] [PORT=port]\n\n"
	@printf "Targets:\n$(RESET)"
	@printf "  $(CYAN)all / build$(RESET)   - Build for $(YELLOW)ENV\n"
	@printf "  $(CYAN)r / run$(RESET)       - Build and run native binary\n"
	@printf "  $(CYAN)u / upload$(RESET)    - Upload firmware on $(YELLOW)PORT\n"
	@printf "  $(CYAN)m / monitor$(RESET)   - Open serial monitor on $(YELLOW)PORT\n"
	@printf "  $(CYAN)c / clean$(RESET)     - Remove build artifacts for $(YELLOW)ENV\n"
	@printf "  $(CYAN)l / list$(RESET)      - Show available USB/ACM ports\n"
	@printf "  $(CYAN)h / help$(RESET)      - Show help\n\n"
	@printf "$(BOLD)Variables:$(RESET)\n"
	@printf "  $(CYAN)ENV$(RESET)   - environment (default: $(YELLOW)$(ENV_ESP32)$(RESET))\n"
	@printf "  $(CYAN)PORT$(RESET)  - serial port (default: $(YELLOW)auto-detected first USB/ACM$(RESET))\n"

h: help
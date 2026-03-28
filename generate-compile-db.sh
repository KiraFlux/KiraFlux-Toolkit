#!/bin/bash
set -e

COMPILE_COMMANDS="compile_commands.json"

echo "Generating compile_commands.json..."
pio run --target compiledb > /dev/null 2>&1

echo "Cleaning up compile_commands.json..."
sed -i \
    -e 's/-mlongcalls/-mlong-calls/g' \
    -e 's/-fno-tree-switch-conversion//g' \
    -e 's/-fstrict-volatile-bitfields//g' \
    -e 's/-Wno-unknown-warning-option//g' \
    "$COMPILE_COMMANDS"
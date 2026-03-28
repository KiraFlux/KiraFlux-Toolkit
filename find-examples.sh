#!/bin/bash
# Outputs a JSON array of directories containing platformio.ini

set -e

paths=$(find examples -maxdepth 3 -name platformio.ini -printf '%h\n' | sort)

if [ -z "$paths" ]; then
    echo "[]"
    exit 0
fi

echo "$paths" | sed 's/.*/"&"/' | paste -sd, | sed 's/.*/[&]/'
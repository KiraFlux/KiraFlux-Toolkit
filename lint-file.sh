#!/bin/bash
set -e

FILE="$1"
SRC_DIR="src/kf"

CLANG_TIDY_ARGS=(
    --config-file=.clang-tidy
    --header-filter="$SRC_DIR/.*"
    --extra-arg=-Wno-unknown-argument
    --extra-arg=-Wno-unknown-warning-option
    --checks='-*,readability-identifier-naming'
    --warnings-as-errors='readability-identifier-naming'
    -p .
)

# Run clang-tidy, ignore compilation errors, only check style
output=$(clang-tidy "$FILE" "${CLANG_TIDY_ARGS[@]}" 2>&1 || true)

# Report only readability-identifier-naming errors
if echo "$output" | grep -q "error:.*readability-identifier-naming"; then
    echo "$output" | grep --color=auto "error:.*readability-identifier-naming"
    exit 1
fi
exit 0
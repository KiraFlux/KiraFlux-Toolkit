#!/bin/bash
set -e
clang-tidy "$1" \
    --config-file=.clang-tidy \
    --header-filter='src/kf/.*' \
    --extra-arg=-Wno-unknown-argument \
    --extra-arg=-Wno-unknown-warning-option \
    --checks='-*,readability-identifier-naming' \
    --warnings-as-errors='readability-identifier-naming' \
    -p .
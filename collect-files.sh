#!/bin/bash
set -e

if [ "$1" = "full" ]; then
    find src/kf -type f \( -name "*.cpp" -o -name "*.hpp" \)
else
    base_ref="${GITHUB_BASE_REF:-main}"
    git fetch origin "$base_ref" --depth=1 2>/dev/null || true
    git diff --name-only --diff-filter=ACMRTUB "origin/$base_ref" HEAD -- '*.cpp' '*.hpp'
fi | sort | sed 's/.*/"&"/' | paste -sd, | sed 's/.*/[&]/'
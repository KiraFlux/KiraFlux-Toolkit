#!/bin/bash
set -e

MODE="$1"
SRC_DIR="src/kf"

if [ "$MODE" = "full" ]; then
    FILES=$(find "$SRC_DIR" -type f \( -name "*.cpp" -o -name "*.hpp" \))
else
    if [ -n "$GITHUB_BASE_REF" ]; then
        BASE="origin/$GITHUB_BASE_REF"
        git fetch origin "$GITHUB_BASE_REF" --depth=1 2>/dev/null || true
    else
        BASE="origin/main"
    fi
    FILES=$(git diff --name-only "$BASE" | grep -E "^${SRC_DIR}/.*\.(cpp|hpp)$" || true)
fi

# Convert to JSON array
echo "$FILES" | sort | sed 's/.*/"&"/' | paste -sd, | sed 's/.*/[&]/'
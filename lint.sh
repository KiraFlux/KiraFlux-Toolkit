#!/bin/bash
set -e

MODE="${1:-full}"

SRC_DIR="src/kf"
COMPILE_COMMANDS="compile_commands.json"

# Sed expressions to remove GCC-specific flags that confuse clang-tidy
SED_CLEANUP_ARGS=(
    -e 's/-mlongcalls/-mlong-calls/g'          # clang expects -mlong-calls
    -e 's/-fno-tree-switch-conversion//g'      # GCC-specific, not needed
    -e 's/-fstrict-volatile-bitfields//g'      # GCC-specific, ignore
    -e 's/-Wno-unknown-warning-option//g'      # already handled
)

CLANG_TIDY_ARGS=(
    --config-file=.clang-tidy
    --header-filter="$SRC_DIR/.*"
    --extra-arg=-Wno-unknown-argument
    --extra-arg=-Wno-unknown-warning-option
    --checks='-*,readability-identifier-naming'
    --warnings-as-errors='readability-identifier-naming'
    -p .
)

echo "Generating compile_commands.json..."
pio run --target compiledb > /dev/null 2>&1

echo "Cleaning up compile_commands.json..."
sed -i "${SED_CLEANUP_ARGS[@]}" "$COMPILE_COMMANDS"

# Determine which files to check
if [ "$MODE" = "diff" ]; then
    echo "Mode: diff (only changed files)"
    # Determine base branch: in CI it's origin/$GITHUB_BASE_REF, locally origin/main
    if [ -n "$GITHUB_BASE_REF" ]; then
        BASE="origin/$GITHUB_BASE_REF"
        # Ensure the base branch is fetched
        git fetch origin "$GITHUB_BASE_REF" --depth=1 2>/dev/null || true
    else
        BASE="origin/main"
    fi
    FILES=$(git diff --name-only "$BASE" | grep -E "^${SRC_DIR}/.*\.(cpp|hpp)$" || true)
    if [ -z "$FILES" ]; then
        echo "No changed files in $SRC_DIR to check."
        exit 0
    fi
else
    echo "Mode: full (all files in $SRC_DIR)"
    FILES=$(find "$SRC_DIR" -type f \( -name '*.cpp' -o -name '*.hpp' \))
fi

# Run clang-tidy on each file
errors_found=0
while IFS= read -r file; do
    [ -z "$file" ] && continue
    echo "Checking $file"
    output=$(clang-tidy "$file" "${CLANG_TIDY_ARGS[@]}" 2>&1 || true)
    if echo "$output" | grep -q "error:.*readability-identifier-naming"; then
        echo "$output" | grep --color=auto "error:.*readability-identifier-naming"
        errors_found=$((errors_found + 1))
    fi
done <<< "$FILES"

if [ "$errors_found" -gt 0 ]; then
    echo "Found style errors in $errors_found file(s)."
    exit 1
else
    echo "No style errors detected."
    exit 0
fi
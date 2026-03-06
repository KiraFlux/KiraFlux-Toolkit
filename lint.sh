#!/bin/bash
set -e

SRC_DIR="src/kf"
COMPILE_COMMANDS="compile_commands.json"

# Sed expressions to clean up GCC-specific flags that clang-tidy does not recognize
SED_CLEANUP_ARGS=(
    -e 's/-mlongcalls/-mlong-calls/g'          # clang expects -mlong-calls
    -e 's/-fno-tree-switch-conversion//g'      # GCC optimization flag, not needed for static analysis
    -e 's/-fstrict-volatile-bitfields//g'      # GCC-specific, irrelevant for linting
    -e 's/-Wno-unknown-warning-option//g'      # already handled by extra-arg
)

CLANG_TIDY_ARGS=(
    --config-file=.clang-tidy
    --header-filter="$SRC_DIR/.*"                           # only report diagnostics from our headers
    --extra-arg=-Wno-unknown-argument                       # suppress "unknown argument" warnings
    --extra-arg=-Wno-unknown-warning-option                 # suppress warnings about unknown warning flags
    --checks='-*,readability-identifier-naming'             # enable only the naming check
    --warnings-as-errors='readability-identifier-naming'    # treat naming violations as errors
    -p .                                                    # use compile_commands.json in current dir
)

echo "Generating compile_commands.json..."
pio run --target compiledb > /dev/null 2>&1

echo "Cleaning up compile_commands.json..."
sed -i "${SED_CLEANUP_ARGS[@]}" "$COMPILE_COMMANDS"

echo "Running clang-tidy on files in $SRC_DIR ..."
errors_found=0

while IFS= read -r file; do
    # Run clang-tidy and capture output (ignore normal exit code)
    output=$(clang-tidy "$file" "${CLANG_TIDY_ARGS[@]}" 2>&1 || true)

    # If output contains a naming error, print it and count the file
    if echo "$output" | grep -q "error:.*readability-identifier-naming"; then
        echo "$output" | grep --color=always "error:.*readability-identifier-naming"
        errors_found=$((errors_found + 1))
    fi
done < <(find "$SRC_DIR" -type f \( -name '*.cpp' -o -name '*.hpp' \))

if [ "$errors_found" -gt 0 ]; then
    echo "Found style errors in $errors_found file(s)."
    exit 1
else
    echo "No style errors detected."
    exit 0
fi
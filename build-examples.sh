#!/bin/bash
for dir in examples/*/; do
    if [ -d "$dir" ]; then
        echo "Building $(basename "$dir")..."
        pio run -d "$dir"
    fi
done
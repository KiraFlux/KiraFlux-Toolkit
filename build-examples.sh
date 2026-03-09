#!/bin/bash
err=0
for dir in examples/*/; do
    if [ -d "$dir" ]; then
        echo "Building $(basename "$dir")..."
        pio run --silent -d "$dir" || { echo "Build failed for $(basename "$dir")"; err=1; }
    fi
done
exit $err
#!/bin/bash

# This script builds all examples in the $GITHUB_WORKSPACE/examples directory using PlatformIO.
# ASSUMPTIONS: 
#   - PlatformIO is installed and available in the PATH.

examples_dir="$GITHUB_WORKSPACE/examples"

for example in $(ls "$examples_dir"); do
    echo "[$0]: Building example: $example"
    pio run -d "$examples_dir/$example" || {
        echo "[$0]: Build failed for example: $example"
        exit 1
    }
done

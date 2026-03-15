#!/bin/bash

# This script builds all examples in the $GITHUB_WORKSPACE/examples directory using PlatformIO.
# ASSUMPTIONS: 
#   - PlatformIO is installed and available in the PATH.

examples=$(ls $GITHUB_WORKSPACE/examples);

for example in $examples; do
    echo "Building example: $example"
    pio run -d $GITHUB_WORKSPACE/examples/$example || \
        echo "Build failed for example: $example"; exit 1
done

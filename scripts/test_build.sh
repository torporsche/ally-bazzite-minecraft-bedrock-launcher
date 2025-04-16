#!/bin/bash

# Script to test build system
# Created: 2025-04-16 23:19:51 UTC
# Author: torporsche

set -e  # Exit on any error

echo "Testing build system..."

# Create build directory
mkdir -p build
cd build

# Configure
cmake ..

# Build
cmake --build .

# Run tests if they exist
ctest --output-on-failure

echo "Build test complete!"
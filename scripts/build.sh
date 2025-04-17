#!/bin/bash

# Configuration
BUILD_TYPE="Release"
BUILD_DIR="build"
INSTALL_PREFIX="/usr"

# Create build directory
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
    -GNinja

# Build
ninja

# Create packages
cpack -G "DEB;RPM"

# Return to original directory
cd ..
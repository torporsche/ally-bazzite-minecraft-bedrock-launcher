#!/bin/bash

# Set error handling
set -e
echo "Checking and installing build dependencies for Ubuntu 22.04..."

# Function to check if a package is installed
check_package() {
    if ! dpkg -l "$1" >/dev/null 2>&1; then
        echo "Package $1 is not installed. Adding to installation list..."
        PACKAGES_TO_INSTALL="$PACKAGES_TO_INSTALL $1"
    fi
}

# Initialize packages to install
PACKAGES_TO_INSTALL=""

# Required packages list
REQUIRED_PACKAGES=(
    qt6-base-dev
    qt6-webengine-dev
    qt6-gamepad-dev
    libsdl2-dev
    libwayland-dev
    libegl-dev
    libgl-dev
    steam-devices
    lcov
    gcovr
    cmake
    ninja-build
    pkg-config
    vulkan-tools
    build-essential
    git
    wget
    unzip
)

# Optional packages (don't fail if missing)
OPTIONAL_PACKAGES=(
    gamescope
)

# Check each required package
for package in "${REQUIRED_PACKAGES[@]}"; do
    check_package "$package"
done

# Install missing packages if any
if [ ! -z "$PACKAGES_TO_INSTALL" ]; then
    echo "Installing missing packages..."
    sudo apt update
    sudo apt install -y $PACKAGES_TO_INSTALL
else
    echo "All required packages are already installed."
fi

# Check optional packages (don't fail if missing)
echo "Checking optional packages..."
for package in "${OPTIONAL_PACKAGES[@]}"; do
    if ! dpkg -l "$package" >/dev/null 2>&1; then
        echo "Optional package $package is not available or not installed."
    else
        echo "Optional package $package is installed."
    fi
done

# Verify CMake version
CMAKE_VERSION=$(cmake --version | head -n1 | cut -d" " -f3)
if [ "$(printf '%s\n' "3.20" "$CMAKE_VERSION" | sort -V | head -n1)" = "3.20" ]; then
    echo "CMake version $CMAKE_VERSION is sufficient"
else
    echo "CMake version $CMAKE_VERSION is too old. Minimum required is 3.20"
    exit 1
fi

echo "All build dependencies are satisfied."
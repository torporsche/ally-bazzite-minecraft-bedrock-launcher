#!/bin/bash

# Check if script is run with root privileges
if [ "$EUID" -ne 0 ]; then
    echo "Please run this script with sudo privileges"
    exit 1
fi

# Function to print colored output
print_status() {
    local color=$1
    local message=$2
    case $color in
        "green")  echo -e "\e[32m$message\e[0m" ;;
        "red")    echo -e "\e[31m$message\e[0m" ;;
        "yellow") echo -e "\e[33m$message\e[0m" ;;
        "blue")   echo -e "\e[34m$message\e[0m" ;;
    esac
}

# Function to check if a package is installed
check_package() {
    if dpkg -l | grep -q "^ii  $1 "; then
        return 0
    else
        return 1
    fi
}

# Function to check version requirements
check_version() {
    local package=$1
    local required_version=$2
    local installed_version=$(dpkg -l $package | grep "^ii" | awk '{print $3}' | cut -d'-' -f1)
    
    if [ -z "$installed_version" ]; then
        return 1
    fi
    
    if dpkg --compare-versions "$installed_version" "ge" "$required_version"; then
        return 0
    else
        return 1
    fi
}

# Function to add Qt repository
add_qt_repository() {
    print_status "blue" "Adding Qt repository..."
    
    # Add Qt repository key
    wget -nO- https://apt.qt.io/public.gpg | gpg --dearmor > /usr/share/keyrings/qt-archive-keyring.gpg
    
    # Add Qt repository
    echo "deb [signed-by=/usr/share/keyrings/qt-archive-keyring.gpg] https://apt.qt.io/qt6 jammy main" \
        > /etc/apt/sources.list.d/qt6.list
    
    apt-get update
}

# Required packages and their minimum versions
declare -A REQUIRED_PACKAGES=(
    ["build-essential"]="12.9"
    ["cmake"]="3.20"
    ["qt6-base-dev"]="6.2"
    ["qt6-webengine-dev"]="6.2"
    ["qt6-gamepad-dev"]="6.2"
    ["libsdl3-dev"]="3.0"
    ["libwayland-dev"]="1.20"
    ["libegl-dev"]="1.3"
    ["libgl-dev"]="1.3"
    ["libcurl4-openssl-dev"]="7.81"
    ["libssl-dev"]="3.0"
    ["ninja-build"]="1.10"
    ["pkg-config"]="0.29"
    ["libdbus-1-dev"]="1.12"
    ["libxkbcommon-dev"]="1.0"
    ["libvulkan-dev"]="1.3"
)

# Update package lists
print_status "blue" "Updating package lists..."
apt-get update

# Check and install packages
for package in "${!REQUIRED_PACKAGES[@]}"; do
    required_version=${REQUIRED_PACKAGES[$package]}
    
    if ! check_package "$package"; then
        print_status "yellow" "Installing $package..."
        apt-get install -y "$package"
    elif ! check_version "$package" "$required_version"; then
        print_status "yellow" "Updating $package to version >= $required_version..."
        apt-get install --only-upgrade -y "$package"
    else
        print_status "green" "$package is already installed with correct version"
    fi
done

# Check Qt repository
if ! grep -q "apt.qt.io" /etc/apt/sources.list.d/*; then
    add_qt_repository
fi

# Install additional Qt modules
print_status "blue" "Installing additional Qt modules..."
apt-get install -y \
    qt6-declarative-dev \
    qt6-tools-dev \
    qt6-tools-private-dev \
    qt6-multimedia-dev \
    qt6-positioning-dev \
    qt6-websockets-dev

# Install SDL dependencies
print_status "blue" "Installing SDL dependencies..."
apt-get install -y \
    libpulse-dev \
    libasound2-dev \
    libxext-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxss-dev

# Install development tools
print_status "blue" "Installing development tools..."
apt-get install -y \
    git \
    gdb \
    valgrind \
    clang \
    clang-format \
    clang-tidy

# Verify all installations
print_status "blue" "Verifying installations..."
all_installed=true

for package in "${!REQUIRED_PACKAGES[@]}"; do
    required_version=${REQUIRED_PACKAGES[$package]}
    if ! check_package "$package" || ! check_version "$package" "$required_version"; then
        print_status "red" "Failed to install/update $package to version >= $required_version"
        all_installed=false
    fi
done

# Check for Qt installation
if ! qmake6 --version > /dev/null 2>&1; then
    print_status "red" "Qt6 installation verification failed"
    all_installed=false
fi

# Check for SDL installation
if ! pkg-config --modversion sdl3 > /dev/null 2>&1; then
    print_status "red" "SDL3 installation verification failed"
    all_installed=false
fi

# Final status
if [ "$all_installed" = true ]; then
    print_status "green" "All prerequisites are installed successfully!"
    print_status "blue" "You can now build the Minecraft Bedrock Launcher"
else
    print_status "red" "Some prerequisites failed to install. Please check the error messages above."
    exit 1
fi

# Create build environment variables
cat > /etc/profile.d/minecraft-launcher-env.sh << EOF
export Qt6_DIR=/usr/lib/qt6
export SDL3_DIR=/usr/lib/x86_64-linux-gnu/cmake/SDL3
export CMAKE_PREFIX_PATH=/usr/lib/qt6:/usr/lib/x86_64-linux-gnu/cmake
EOF

print_status "blue" "Environment variables have been set. Please log out and log back in for them to take effect."

# Set appropriate permissions for the current user
chown $SUDO_USER:$SUDO_USER /etc/profile.d/minecraft-launcher-env.sh
chmod 644 /etc/profile.d/minecraft-launcher-env.sh

exit 0
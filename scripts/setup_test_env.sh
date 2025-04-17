#!/bin/bash

# Set error handling
set -e

# Function to check if user needs to log out
NEEDS_LOGOUT=0

echo "Setting up test environment for ally-mc-launcher..."

# Get the current username
CURRENT_USER=$(whoami)
PROJECT_DIR="$HOME/development/ally-mc-launcher"

# 1. Create project directory and navigate to it
mkdir -p "$PROJECT_DIR"
cd "$PROJECT_DIR"

# 2. Set up Steam SDK
if [ ! -d "/opt/steam-sdk" ]; then
    echo "Setting up Steam SDK..."
    chmod +x scripts/setup_steam.sh
    ./scripts/setup_steam.sh
fi

# 3. Set up ROG Ally hardware access
echo "Setting up ROG Ally hardware access..."
if [ ! -f "/etc/udev/rules.d/99-rog-ally.rules" ]; then
    sudo cp resources/udev/99-rog-ally.rules /etc/udev/rules.d/
    sudo udevadm control --reload-rules
    sudo udevadm trigger
fi

# Check and add user to required groups
for group in input video; do
    if ! groups "$CURRENT_USER" | grep -q "\b$group\b"; then
        sudo usermod -a -G "$group" "$CURRENT_USER"
        NEEDS_LOGOUT=1
    fi
done

# 4. Create required directories and files
echo "Creating required directories and files..."
mkdir -p ~/.local/share/minecraft/shader_cache
mkdir -p ~/.local/share/vulkan/implicit_layer.d
mkdir -p ~/.config/ally-mc-launcher

# Create test configuration file
cat > ~/.config/ally-mc-launcher/test_config.json << EOL
{
    "steam_integration": {
        "enabled": true,
        "big_picture": true
    },
    "hardware": {
        "default_profile": "balanced",
        "tdp_limit": 15,
        "fan_curve": "dynamic"
    },
    "graphics": {
        "api": "vulkan",
        "fsr": true,
        "shader_cache": true
    }
}
EOL

# 5. Set up environment variables
echo "Setting up environment variables..."
cat > "$PROJECT_DIR/setup_env.sh" << EOL
#!/bin/bash
export STEAM_SDK_PATH=/opt/steam-sdk
export LD_LIBRARY_PATH=/opt/steam-sdk/redistributable_bin/linux64:\$LD_LIBRARY_PATH
export QT_QPA_PLATFORM=wayland
export SDL_VIDEODRIVER=wayland
export DISPLAY=:0
export MESA_GLSL_CACHE_DIR=\$HOME/.local/share/minecraft/shader_cache
export VK_LAYER_PATH=\$HOME/.local/share/vulkan/implicit_layer.d
export TEST_CONFIG_PATH=\$HOME/.config/ally-mc-launcher/test_config.json
EOL

chmod +x "$PROJECT_DIR/setup_env.sh"
source "$PROJECT_DIR/setup_env.sh"

# 6. Prepare build directory
echo "Preparing build directory..."
mkdir -p "$PROJECT_DIR/build/tests"
cd "$PROJECT_DIR/build/tests"

# Configure CMake
cmake ../.. \
    -DBUILD_TESTING=ON \
    -DCODE_COVERAGE=ON \
    -DCMAKE_BUILD_TYPE=Debug \
    -GNinja

# 7. Verify Steam is running
if ! pgrep -x "steam" > /dev/null; then
    echo "Starting Steam..."
    steam -silent &
    sleep 5
fi

# 8. Create and run hardware access test
echo "Testing hardware access..."
cat > "$PROJECT_DIR/test_hardware_access.sh" << EOL
#!/bin/bash
echo "Testing hardware access..."
test -w /sys/class/powercap/powercap0/tdp && echo "TDP control: OK" || echo "TDP control: Failed"
test -w /sys/devices/platform/asus-nb-wmi/profile && echo "Profile control: OK" || echo "Profile control: Failed"
test -r /sys/class/hwmon/hwmon*/temp1_input && echo "Temperature monitoring: OK" || echo "Temperature monitoring: Failed"
EOL

chmod +x "$PROJECT_DIR/test_hardware_access.sh"
"$PROJECT_DIR/test_hardware_access.sh"

# Check if user needs to log out
if [ $NEEDS_LOGOUT -eq 1 ]; then
    echo "WARNING: You have been added to new groups. You must log out and log back in for these changes to take effect."
    echo "After logging back in:"
    echo "1. cd $PROJECT_DIR"
    echo "2. source ./setup_env.sh"
    echo "3. ./scripts/run_tests.sh"
    exit 1
else
    echo "Test environment setup complete!"
    echo "You can now run: ./scripts/run_tests.sh"
fi
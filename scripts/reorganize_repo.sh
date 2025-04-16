#!/bin/bash

# Script to reorganize repository structure
# Created: 2025-04-16 23:19:51 UTC
# Author: torporsche

set -e  # Exit on any error

echo "Starting repository reorganization..."

# Create necessary directories if they don't exist
mkdir -p src/{auth,game,gamepad,steam,ui,core}
mkdir -p resources/{steam/{configs,grid},icons}
mkdir -p .github/workflows
mkdir -p scripts

# Function to process a file
process_file() {
    local file="$1"
    if [[ $file == src_* ]]; then
        # Remove src_ prefix and convert remaining underscores to /
        local new_path=$(echo "$file" | sed 's/^src_/src\//' | sed 's/_/\//g')
        local dir=$(dirname "$new_path")
        
        # Create directory if it doesn't exist
        mkdir -p "$dir"
        
        # Move file to new location
        echo "Moving $file to $new_path"
        git mv "$file" "$new_path" 2>/dev/null || mv "$file" "$new_path"
        
        # Update include paths in the file
        if [[ $file == *.cpp || $file == *.hpp ]]; then
            echo "Updating includes in $new_path"
            sed -i 's/#include ".*\/\([^"]*\)"/#include "\1"/g' "$new_path"
        fi
    fi
}

# Process all source files
echo "Processing source files..."
for file in src_*; do
    if [ -f "$file" ]; then
        process_file "$file"
    fi
done

# Update CMakeLists.txt
echo "Updating CMakeLists.txt..."
cat > src/CMakeLists.txt << 'EOL'
# Source CMakeLists.txt
# Updated: 2025-04-16 23:19:51 UTC

set(SOURCES
    main.cpp
    auth/GooglePlayAuth.cpp
    auth/GooglePlayAPI.cpp
    game/GameManager.cpp
    gamepad/AllyGamepad.cpp
    gamepad/AllySystemControl.cpp
    steam/SteamIntegration.cpp
    steam/SteamBPMIntegration.cpp
    ui/LauncherWindow.cpp
    ui/SettingsDialog.cpp
    ui/VersionInstallDialog.cpp
    core/Config.cpp
)

set(HEADERS
    auth/GooglePlayAuth.hpp
    auth/GooglePlayAPI.hpp
    game/GameManager.hpp
    gamepad/AllyGamepad.hpp
    gamepad/AllySystemControl.hpp
    steam/SteamIntegration.hpp
    steam/SteamBPMIntegration.hpp
    ui/LauncherWindow.hpp
    ui/SettingsDialog.hpp
    ui/VersionInstallDialog.hpp
    core/Config.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::WebEngineWidgets
    Qt6::Gamepad
    SDL3::SDL3
    Wayland::Client
    EGL::EGL
    OpenGL::GL
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
EOL

# Create GitHub Actions workflow
echo "Creating GitHub Actions workflow..."
mkdir -p .github/workflows
cat > .github/workflows/build.yml << 'EOL'
name: Build and Test

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v4
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y build-essential cmake ninja-build
        sudo apt-get install -y qt6-base-dev qt6-webengine-dev qt6-gamepad-dev
        sudo apt-get install -y libsdl3-dev libwayland-dev libegl-dev

    - name: Configure CMake
      run: cmake -B build -DCMAKE_BUILD_TYPE=Release

    - name: Build
      run: cmake --build build --config Release

    - name: Test
      working-directory: build
      run: ctest -C Release --output-on-failure
EOL

# Make scripts executable
chmod +x scripts/*.sh

echo "Repository reorganization complete!"
echo "Please review changes and commit them to the repository."
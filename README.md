# Ally Bazzite Minecraft Bedrock Launcher

Current Date and Time (UTC): 2025-04-17 02:56:53
Current User's Login: torporsche

## Overview
A specialized Minecraft Bedrock launcher designed for the ROG Ally running on Bazzite OS, featuring Steam integration and hardware-specific optimizations.

## Features
- Optimized for ROG Ally hardware
- Steam Input integration with customized controller profiles
- Performance profiles with TDP and fan control
- FSR (FidelityFX Super Resolution) support
- Hardware-accelerated shader caching
- Steam Big Picture Mode support
- Wayland native with Gamescope integration
- Google Play Authentication
- Customizable UI with game settings and version control
- Resource Management with custom grid images

## System Requirements
- ROG Ally running Bazzite OS
- Qt 6.5 or higher
- SDL3
- Steam Runtime
- Steam SDK
- Wayland
- EGL/OpenGL support

## Build Dependencies

### Debian/Ubuntu
```bash
sudo apt install \
    build-essential \
    cmake \
    qt6-base-dev \
    qt6-webengine-dev \
    qt6-qttest-dev \
    libsdl3-dev \
    libwayland-dev \
    libegl-dev \
    libgl-dev
```

### Fedora/Bazzite
```bash
sudo dnf install \
    cmake \
    gcc-c++ \
    qt6-qtbase-devel \
    qt6-qtwebengine-devel \
    qt6-qttest-devel \
    SDL3-devel \
    wayland-devel \
    mesa-libEGL-devel \
    mesa-libGL-devel
```
## Prerequisites Guide for Ally Bazzite Minecraft Bedrock Launcher

This guide provides detailed instructions for setting up all required manual prerequisites before building the Ally Bazzite Minecraft Bedrock Launcher.

## Required Manual Setup Steps

### 1. Google Play Developer Account Setup

#### A. Create Google Cloud Project

1. Visit the [Google Cloud Console](https://console.cloud.google.com/)
2. Click "Create Project"
3. Enter project details:
   - Name: `MinecraftLauncherAPI` (or your preferred name)
   - Organization: (Select your organization or leave as "No Organization")
   - Location: (Choose your preferred location)
4. Click "Create"
5. Wait for project creation to complete
6. Select your new project from the project dropdown menu

#### B. Enable Required APIs

1. Navigate to "APIs & Services > Library"
2. Search for and enable the following APIs:
   - Google Play Android Developer API
   - Google Play Custom App Publishing API
   - OAuth 2.0 Playground

#### C. Configure OAuth Consent Screen

1. Go to "APIs & Services > OAuth consent screen"
2. Select "External" user type
3. Click "Create"
4. Fill in required information:
   - App name: "Minecraft Bedrock Launcher"
   - User support email: (Your email)
   - Developer contact email: (Your email)
   - Application home page: (Your repository URL)
   - Application privacy policy link: (Your privacy policy URL)
   - Application terms of service link: (Your terms of service URL)
5. Click "Save and Continue"
6. Skip "Scopes" section
7. Add test users (required for external apps):
   - Add your Google email address
   - Add any other test user emails
8. Click "Save and Continue"
9. Review your configuration
10. Click "Back to Dashboard"

#### D. Create API Credentials

1. Navigate to "APIs & Services > Credentials"
2. Click "Create Credentials"
3. Select "OAuth 2.0 Client ID"
4. Configure consent screen if prompted
5. Choose "Desktop app" as application type
6. Enter name: "Minecraft Bedrock Launcher"
7. Click "Create"
8. Download the JSON file
9. Rename downloaded file to `google_play_api_credentials.json`
10. Move file to project's resources directory:
    
```bash
mv google_play_api_credentials.json ally-bazzite-minecraft-bedrock-launcher/resources/
```
### 2. Steam SDK Setup

#### A. Join Steamworks

1. Visit [Steamworks](https://partner.steamgames.com/home)
2. Create a Steamworks account if you haven't already
3. Accept the Steamworks SDK License Agreement

#### B. Download and Install SDK

1. Download the Steamworks SDK from the [Steam Partner site](https://partner.steamgames.com/downloads/steamworks_sdk.zip)
2. Extract the SDK:

```bash
unzip steamworks_sdk.zip -d steamworks
```

3. Create a directory for the SDK:

```bash
sudo mkdir -p /opt/steam-sdk
```

4. Copy required files:

```bash
sudo cp -r steamworks/sdk/* /opt/steam-sdk/
```

5. Create symlinks:

``` bash
sudo ln -s /opt/steam-sdk/public/steam /usr/include/steam
sudo ln -s /opt/steam-sdk/redistributable_bin/linux64/libsteam_api.so /usr/lib/libsteam_api.so
```
## Building the Project

After completing the manual prerequisites above, you can build the project by following these steps:

1. Clone the repository:

```bash
git clone https://github.com/torporsche/ally-bazzite-minecraft-bedrock-launcher.git
cd ally-bazzite-minecraft-bedrock-launcher
```
## Troubleshooting

### Common Issues

#### Google Play API

* Ensure the credentials file is correctly named and placed in the resources directory
* Verify that all required APIs are enabled in Google Cloud Console
* Check that your Google account is added as a test user

#### Steam SDK

* Make sure all symlinks are correctly created
* Verify the SDK is installed in the correct location
* Ensure proper read permissions on SDK files

## Support

For additional help:

* Check the [GitHub Issues](https://github.com/torporsche/ally-bazzite-minecraft-bedrock-launcher/issues)
* Submit a new issue if you encounter problems

## Building from Source

1. Clone the repository:
   
```bash
git clone https://github.com/torporsche/ally-bazzite-minecraft-bedrock-launcher.git
cd ally-bazzite-minecraft-bedrock-launcher
```

2. Setup Steam SDK:

``` bash
./scripts/setup_steam.sh
```

3. Create build directory and configure:

```bash
mkdir build && cd build
cmake -GNinja ..
ninja
```

4. Build the project:

```bash
cmake --build . -j$(nproc)
```

5. Install (requires root privileges):

```bash
sudo cmake --install .
```

6. Setup ROG Ally hardware support:

```bash
sudo ./scripts/setup_rog_ally.sh
```

## The Remaining Steps are Automated by the Build and Can also be done Manually

## Testing

1. Set up the test environment:
   
```bash
./scripts/setup_test_env.sh
```

2. Run the test suite:

```bash
./scripts/run_tests.sh
```

For running specific tests:

```bash
cd build/tests
ctest -V -R TestSuiteName
```

To generate test coverage report:

```bash
cmake -DCODE_COVERAGE=ON ..
cmake --build .
./scripts/run_tests.sh
```

## Installation Packages

### DEB Package

```bash
cd build
cpack -G DEB
sudo dpkg -i ally-mc-launcher-0.1.0-Linux.deb
```

### RPM Package

```bash
cd build
cpack -G RPM
sudo rpm -i ally-mc-launcher-0.1.0-Linux.rpm
```

### Flatpak

```bash
flatpak-builder --user --install --force-clean build-dir \
    flatpak/com.github.torporsche.ally-mc-launcher.yml
```

## Configuration

Configuration files are installed in the following locations:

* System configuration: `/etc/ally-mc-launcher/config/`
* User configuration: `~/.config/ally-mc-launcher/`
* Gamepad profiles: `/usr/share/ally-mc-launcher/gamepad/`
* Shader cache: `/var/lib/ally-mc-launcher/shader_cache/`
* Game data: `~/.local/share/ally-mc-launcher/`
* Logs: `~/.local/share/ally-mc-launcher/logs/`

## Hardware Control

The launcher requires specific permissions to control ROG Ally hardware. These are set up automatically during installation, but can be manually configured:

1. Install udev rules:

```bash
sudo cp resources/udev/99-rog-ally.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

2. Add user to required groups:

```bash
sudo usermod -a -G input,gamepad $USER
```

## Steam Integration

### Manual Steam Setup

1. Open Steam
2. Click "Add a Non-Steam Game"
3. Browse to the launcher executable
4. Set launch options:

```bash
--fullscreen --gamemode
```

### Controller Configuration

* Steam Input is enabled by default
* Custom ROG Ally profile is automatically loaded
* Profile can be customized through Steam's controller configuration

## Troubleshooting

### Steam Integration Issues

* Verify Steam is running
* Check Steam SDK installation: `ls -l /opt/steam-sdk`
* Verify Steam Input configuration: `~/.local/share/Steam/config/controller_configs/`

### Hardware Control Issues

* Check udev rules: `ls -l /etc/udev/rules.d/99-rog-ally.rules`
* Verify user groups: `groups | grep -E "input|gamepad"`
* Check hardware access permissions: `ls -l /dev/input/event*`

### Build Issues

* Verify all dependencies are installed
* Check CMake configuration output
* Ensure Steam SDK is properly set up
* Verify Qt6 installation: `qmake6 --version`

### Performance Issues

* Check current performance mode in settings
* Verify TDP settings are appropriate
* Monitor shader cache usage
* Ensure FSR settings are configured correctly

## Contributing

1. Fork the repository
2. Create a feature branch
3. Run tests before committing
4. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Authors

* **torporsche**

## Acknowledgments

* ROG Ally community
* Bazzite OS team
* Minecraft Bedrock Edition developers
* Unofficial \*NIX Lancher for Minecraft community


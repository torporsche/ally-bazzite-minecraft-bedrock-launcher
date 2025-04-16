# Ally Bazzite Minecraft Bedrock Launcher

## Overview
The Ally Bazzite Minecraft Bedrock Launcher is a custom launcher designed to enhance the gaming experience for Minecraft Bedrock players on the ASUS ROG Ally. With seamless integration into Steam's Big Picture Mode, customizable gamepad support, and streamlined authentication processes, this launcher is tailored for gamers who want to play Minecraft Bedrock Edition with ease.

---

## Features
- **Google Play Authentication**: Simplify login to Minecraft Bedrock using Google Play credentials.
- **Steam Integration**: Launch Minecraft Bedrock from Steam's Big Picture Mode with custom grid images.
- **Gamepad Support**: Optimized control mapping for ASUS ROG Ally's gamepad.
- **Customizable UI**: A sleek launcher window with options to tweak game settings and versions.
- **Resource Management**: Includes custom grid images and icons for Steam compatibility.

---

## Prerequisites
Before building or running the launcher, ensure you have the following installed on your system:

1. **C++ Compiler**: GCC or MSVC (C++17 or newer supported).
2. **CMake**: For building the project.
3. **Git**: For version control.
4. **Google Play Developer API Access**: Required for authentication.
5. **Steam**: **Note**: Steam is required only for running the launcher and enabling integration with Big Picture Mode. It is not needed for building the launcher.

---

### Setting Up Google Play Developer API Access
To enable Google Play authentication for the launcher, follow these steps to set up the Google Play Developer API:

#### Step 1: Create a Google Cloud Project
1. Visit the [Google Cloud Console](https://console.cloud.google.com/).
2. Click **Create Project**.
3. Enter a project name (e.g., `MinecraftLauncherAPI`) and click **Create**.
4. After creating the project, select it from the project dropdown menu.

#### Step 2: Enable the Google Play Developer API
1. In the Google Cloud Console, navigate to **APIs & Services > Library**.
2. Search for **Google Play Developer API**.
3. Click **Enable** to activate the API for your project.

#### Step 3: Set Up OAuth Consent Screen
1. Navigate to **APIs & Services > OAuth consent screen**.
2. Choose **External** and click **Create**.
3. Fill out the required fields:
   - App name: `Minecraft Launcher`
   - User support email: Your email address
   - Developer contact email: Your email address
4. Click **Save and Continue** through the remaining steps, leaving defaults.

#### Step 4: Create API Credentials
1. Navigate to **APIs & Services > Credentials**.
2. Click **Create Credentials** and select **OAuth 2.0 Client IDs**.
3. Choose **Desktop app** as the application type.
4. Name your client (e.g., `MinecraftLauncherClient`) and click **Create**.
5. Download the JSON file containing your client ID and secret. Save this file as `google_play_api_credentials.json`.

#### Step 5: Place Credentials in the Launcher Directory
1. Move the `google_play_api_credentials.json` file into the `resources/` directory of your launcher project:
   ```bash
   mv google_play_api_credentials.json ally-bazzite-minecraft-bedrock-launcher/resources/
   ```

#### Step 6: Test API Access
1. Navigate to the launcher directory:
   ```bash
   cd ally-bazzite-minecraft-bedrock-launcher
   ```
2. Run the test script to verify Google Play API connectivity:
   ```bash
   ./scripts/test_google_play_api.sh
   ```
3. Ensure that the script outputs a successful connection message.

---

## Building from Source

### Clone the Repository
```bash
git clone https://github.com/torporsche/ally-bazzite-minecraft-bedrock-launcher.git
cd ally-bazzite-minecraft-bedrock-launcher
```

### Install Prerequisites
```bash
sudo ./scripts/setup_prerequisites.sh
```

### Create Build Directory
```bash
mkdir build
cd build
```

### Configure with CMake
```bash
cmake -GNinja ..
```

### Build the Project
```bash
ninja
```

### Install (Optional)
```bash
sudo ninja install
```

---

## Running the Launcher

### After Building
To run the launcher after building:
```bash
./ally-mc-launcher
```

### If Installed
If the launcher is installed:
```bash
ally-mc-launcher
```

---

## Configuration

- The launcher stores its configuration in:
  ```plaintext
  ~/.config/ally-mc-launcher/
  ```

- Game data is stored by default in:
  ```plaintext
  ~/.local/share/ally-mc-launcher/
  ```

---

## Steam Integration
The launcher automatically integrates with Steam when running in Big Picture Mode or Game Mode. To manually add the launcher to Steam:
1. Open Steam.
2. Click "Add a Non-Steam Game."
3. Browse to the launcher executable.
4. Set launch options to:
   ```bash
   --fullscreen --gamemode
   ```

---

## Troubleshooting

### Common Issues

#### Steam Controller Not Detected
- Ensure Steam Input is enabled in Steam settings.
- Try restarting Steam.

#### Performance Issues
- Check current performance mode in settings.
- Verify TDP settings are appropriate.

#### Installation Failures
- Verify internet connection.
- Check available storage space.
- Ensure Google Play account is properly authenticated.

---

### Logs
Logs are stored in:
```plaintext
~/.local/share/ally-mc-launcher/logs/
```

---

## Contributing

1. Fork the repository.
2. Create a feature branch.
3. Commit your changes.
4. Push to the branch.
5. Create a Pull Request.

---

## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Authors
- **torporsche**

---

## Acknowledgments
- ROG Ally community
- Bazzite OS team
- Minecraft Bedrock Edition developers

---

## Last Updated
2025-04-15 12:27:59 UTC
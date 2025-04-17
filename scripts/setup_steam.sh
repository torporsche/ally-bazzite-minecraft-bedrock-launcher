#!/bin/bash

# Install Steam SDK
STEAM_SDK_PATH="/opt/steam-sdk"
if [ ! -d "$STEAM_SDK_PATH" ]; then
    sudo mkdir -p "$STEAM_SDK_PATH"
    wget https://partner.steamgames.com/downloads/steamworks_sdk.zip
    unzip steamworks_sdk.zip -d "$STEAM_SDK_PATH"
    rm steamworks_sdk.zip
fi

# Set up environment variables
echo "export STEAM_SDK_PATH=$STEAM_SDK_PATH" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=$STEAM_SDK_PATH/redistributable_bin/linux64:$LD_LIBRARY_PATH" >> ~/.bashrc

# Create Steam application manifest
cat > ~/.local/share/Steam/steam/apps/ally_mc_launcher.vdf << EOL
"AppState"
{
    "appid"     "1234567890"
    "Universe"  "1"
    "name"      "Ally MC Launcher"
    "StateFlags"    "4"
    "installdir"    "Ally MC Launcher"
    "LastUpdated"   "1681689025"
    "SizeOnDisk"    "0"
    "StagingSize"   "0"
    "buildid"       "0"
    "LastOwner"     "0"
    "UpdateResult"  "0"
    "BytesToDownload"   "0"
    "BytesDownloaded"   "0"
    "BytesToStage"      "0"
    "BytesStaged"       "0"
    "TargetBuildID"     "0"
    "AutoUpdateBehavior"    "0"
    "AllowOtherDownloadsWhileRunning"   "0"
    "ScheduledAutoUpdate"   "0"
}
EOL

# Set permissions
chmod +x scripts/setup_steam.sh
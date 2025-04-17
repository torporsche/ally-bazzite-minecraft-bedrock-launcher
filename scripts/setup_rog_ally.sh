#!/bin/bash

# Install udev rules
sudo cp resources/udev/99-rog-ally.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger

# Set up initial performance profile
echo "1" | sudo tee /sys/devices/platform/asus-nb-wmi/profile > /dev/null

# Enable FreeSync by default
echo "1" | sudo tee /sys/class/drm/card0/device/freesync_enabled > /dev/null

# Set up initial TDP
echo "15000000" | sudo tee /sys/class/powercap/powercap0/tdp > /dev/null

# Add current user to required groups
sudo usermod -a -G input,video $USER

# Create configuration directory
mkdir -p ~/.config/ally-mc-launcher
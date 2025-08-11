#!/bin/bash

# Set error handling
set -e
echo "Setting up prerequisites for ally-mc-launcher build..."

# Source the existing dependency checker
source "$(dirname "$0")/check_dependencies.sh"

echo "Prerequisites setup completed successfully."
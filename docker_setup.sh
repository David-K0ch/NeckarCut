#!/bin/bash
# docker_setup.sh
# This script installs dependencies and builds the solver in a Debian 13.5 container.

# Exit immediately if a command exits with a non-zero status
set -e

# Update package lists and install required dependencies
apt-get update
apt-get install -y cmake g++ make

# Make sure our internal setup script is executable
chmod +x setup.sh

# Run the build script
./setup.sh

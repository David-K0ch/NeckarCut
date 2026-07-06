#!/bin/bash

# Remove old build directory to avoid CMake cache path conflicts
rm -rf build

# Create build directory
mkdir -p build
cd build

# Configure CMake (Release mode for performance)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build only the final binary
cmake --build . --target NeckarCut -j

#!/bin/bash

# Navigate to the build directory
mkdir -p build
cd build

# Run cmake and build
cmake ..
cmake --build .

# Run the executable
./tcity
#!/bin/bash
# Install dependencies first:
# Fedora: sudo dnf install mingw64-gcc mingw64-gcc-c++
# Ubuntu/Debian: sudo apt install mingw-w64

mkdir -p build-win
cd build-win

# Run CMake using the MinGW toolchain
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw.cmake -DCMAKE_BUILD_TYPE=Release ..

# Compile
make

echo "Build complete. Executable should be in build-win/CRayEngine.exe"

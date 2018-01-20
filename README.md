# ANTLR Cmake Starter

## Description

A ready to use starter project using ANTLR, Cmake and Emscripten.

There are no dependencies. The script `/tools/antlr/download_and_patch.sh` will
download ANTLR and it will be used as a CMake dependency.

## Demo
[Demo](https://ArthurSonzogni.github.com/ANTLR-cmake-starter)

## Instructions

```bash

# Download ANTLR.
cd tools/antlr
./download_and_patch.sh
cd ../..

# Build the project
mkdir build
cd build
cmake ..
make main.js
./src/main

# Test the demo
cd src
./run_web_server.sh
navigate to http://localhost:8080

```

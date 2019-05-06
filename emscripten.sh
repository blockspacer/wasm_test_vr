#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

mkdir -p build_emscripten
em++ --std=c++11 -s USE_WEBGL2=1 src/*.cpp -o build_emscripten/stl.js --preload-file src_asset

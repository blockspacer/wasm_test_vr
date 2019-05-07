#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

mkdir -p build_emscripten
em++                          \
  --std=c++11                 \
  -Werror                     \
  -s USE_WEBGL2=1             \
  --preload-file src_asset    \
  src/*.cpp                   \
  -o build_emscripten/stl.js  \


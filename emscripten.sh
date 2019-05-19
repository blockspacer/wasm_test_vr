#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

mkdir -p build_fbs_cpp
flatc -c -b -o build_fbs_cpp src_fbs/*.fbs

mkdir -p build_fbs_js
flatc -s -b -o build_fbs_js src_fbs/*.fbs
cp $FLATBUFFERS/js/flatbuffers.js build_fbs_js

mkdir -p build_emscripten
em++                         \
  --std=c++11                \
  -Werror                    \
  -s USE_WEBGL2=1            \
  --preload-file src_asset   \
  -I $FLATBUFFERS/include    \
  -I build_fbs_cpp           \
  src/*.cpp                  \
  -o build_emscripten/app.js \


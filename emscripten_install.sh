#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

TARGET="$1"
mkdir -p $TARGET

./emscripten.sh
#rm build_emscripten/stl.js
cp src_web/* $TARGET
cp build_emscripten/* $TARGET
cp build_fbs_js/* $TARGET


#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

./emscripten.sh
cp src_web/* /var/www/html/stl/
#rm build_emscripten/stl.js
cp build_emscripten/* /var/www/html/stl/

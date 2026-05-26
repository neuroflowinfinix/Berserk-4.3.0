#!/bin/bash
set -e
source /content/emsdk/emsdk_env.sh
cd source/src
emcc *.c pyrrhic/tbprobe.c noobprobe/noobprobe.c -Oz -flto -DNDEBUG -s WASM=1 \
    -s EXPORT_NAME=Berserk -s MODULARIZE=1 -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORTED_RUNTIME_METHODS="['ccall']" \
    -s EXPORTED_FUNCTIONS="['_main','_ExecCommand']" \
    -o ../../src/berserk.js
echo "Build complete."

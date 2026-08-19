#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
UF2_DIR="$ROOT_DIR/uf2"

if [[ -z "${PICO_SDK_PATH:-}" ]]; then
    echo "ERROR: PICO_SDK_PATH is not set."
    echo "Example: export PICO_SDK_PATH=~/pico/pico-sdk"
    exit 1
fi

cmake -S "$ROOT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" -j"$(nproc)"

mkdir -p "$UF2_DIR"
cp "$BUILD_DIR"/sn74hc*_test.uf2 "$UF2_DIR"/

echo
echo "Build complete. UF2 files copied to:"
echo "  $UF2_DIR"
ls -1 "$UF2_DIR"/*.uf2

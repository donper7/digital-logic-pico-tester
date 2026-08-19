#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
rm -rf "$ROOT_DIR/build"
find "$ROOT_DIR/uf2" -maxdepth 1 -type f -name '*.uf2' -delete
printf 'Cleaned build/ and generated UF2 files.\n'

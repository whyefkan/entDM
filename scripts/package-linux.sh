#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j"$(nproc)"

rm -rf dist/package
mkdir -p dist/package
cp build/entdm dist/package/
cp -r extension dist/package/
cp README.md LICENSE NOTICE dist/package/

tar -czf dist/entdm-linux-x86_64.tar.gz -C dist/package .

echo "Created dist/entdm-linux-x86_64.tar.gz"

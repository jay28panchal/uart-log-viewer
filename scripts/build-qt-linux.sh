#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/build/qt-linux"
DIST="$ROOT/dist"

mkdir -p "$BUILD" "$DIST"

cmake -S "$ROOT/cpp" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" --target uart-log-viewer

cmake --install "$BUILD" --prefix "$BUILD/install"

# Create DEB via CPack
cpack -G DEB -B "$BUILD" --config "$BUILD/CPackConfig.cmake"

# Move package
if ls "$BUILD"/*.deb >/dev/null 2>&1; then
  mv "$BUILD"/*.deb "$DIST/"
else
  echo "No .deb found in $BUILD"
  exit 1
fi

echo "Built DEB in $DIST"

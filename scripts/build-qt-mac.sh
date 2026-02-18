#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/build/qt-mac"
DIST="$ROOT/dist"

mkdir -p "$BUILD" "$DIST"

cmake -S "$ROOT/cpp" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" --target uart-log-viewer

APP_PATH="$BUILD/uart-log-viewer.app"
if [ ! -d "$APP_PATH" ]; then
  APP_PATH="$BUILD/Release/uart-log-viewer.app"
fi

# Deploy Qt runtime
macdeployqt "$APP_PATH"

# Create DMG if create-dmg is available
if command -v create-dmg >/dev/null 2>&1; then
  create-dmg --volname "UART Log Viewer" --window-pos 200 120 --window-size 800 400 \
    --icon-size 100 --icon "uart-log-viewer.app" 200 190 \
    --app-drop-link 600 185 \
    "$DIST/UART-Log-Viewer.dmg" "$APP_PATH"
  echo "Built DMG at $DIST/UART-Log-Viewer.dmg"
else
  echo "Built app at $APP_PATH"
  echo "To make a DMG: brew install create-dmg && rerun."
fi

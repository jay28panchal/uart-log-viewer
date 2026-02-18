#!/usr/bin/env bash
set -euo pipefail

APP_NAME="UART Log Viewer"
VERSION="$(cat "$(cd "$(dirname "$0")/.." && pwd)/VERSION")"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DIST="$ROOT/dist"
BUILD="$ROOT/build"

python3 -m pip install --upgrade pip
python3 -m pip install -r "$ROOT/requirements.txt"
python3 -m pip install pyinstaller

pyinstaller --noconsole --name "$APP_NAME" --clean --distpath "$DIST" --workpath "$BUILD" "$ROOT/src/uart_tabs.py"

# Create DMG if create-dmg is available
if command -v create-dmg >/dev/null 2>&1; then
  create-dmg --volname "UART Log Viewer" --window-pos 200 120 --window-size 800 400 \
    --icon-size 100 --icon "$APP_NAME.app" 200 190 \
    --app-drop-link 600 185 \
    "$DIST/UART-Log-Viewer.dmg" "$DIST/$APP_NAME.app"
  echo "Built DMG at $DIST/UART-Log-Viewer.dmg"
else
  echo "Built app at $DIST/$APP_NAME.app"
  echo "To make a DMG: brew install create-dmg && rerun."
fi

#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/build/qt-linux-appimage"
DIST="$ROOT/dist"
APP_VERSION="$(cat "$ROOT/VERSION")"

mkdir -p "$BUILD" "$DIST"

cmake -S "$ROOT/cpp" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" --target uart-log-viewer

APPDIR="$BUILD/AppDir"
mkdir -p "$APPDIR/usr/bin"

# Install binary
cp "$BUILD/uart-log-viewer" "$APPDIR/usr/bin/uart-log-viewer"

# Desktop + icon
mkdir -p "$APPDIR/usr/share/applications" "$APPDIR/usr/share/icons/hicolor/scalable/apps"
cp "$ROOT/cpp/resources/uart-log-viewer.desktop" "$APPDIR/usr/share/applications/"
cp "$ROOT/cpp/resources/uart-log-viewer.svg" "$APPDIR/usr/share/icons/hicolor/scalable/apps/"

# Bundle Qt libs using linuxdeploy
LINUXDEPLOY="$BUILD/linuxdeploy-x86_64.AppImage"
PLUGIN_QT="$BUILD/linuxdeploy-plugin-qt-x86_64.AppImage"
APPIMAGETOOL="$BUILD/appimagetool-x86_64.AppImage"

if [ ! -f "$LINUXDEPLOY" ]; then
  curl -L -o "$LINUXDEPLOY" https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
  chmod +x "$LINUXDEPLOY"
fi
if [ ! -f "$PLUGIN_QT" ]; then
  curl -L -o "$PLUGIN_QT" https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
  chmod +x "$PLUGIN_QT"
fi
if [ ! -f "$APPIMAGETOOL" ]; then
  curl -L -o "$APPIMAGETOOL" https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
  chmod +x "$APPIMAGETOOL"
fi

# Ensure Qt tools are in PATH
export PATH="$PATH:/usr/lib/qt6/bin:/usr/lib/x86_64-linux-gnu/qt6/bin"

"$LINUXDEPLOY" --appdir "$APPDIR" --plugin qt --output appimage --appimage-extract-and-run || true

# linuxdeploy may already produce AppImage; if not, use appimagetool
if ls "$BUILD"/*.AppImage >/dev/null 2>&1; then
  mv "$BUILD"/*.AppImage "$DIST/"
else
  "$APPIMAGETOOL" "$APPDIR" "$DIST/uart-log-viewer-${APP_VERSION}-x86_64.AppImage"
fi

echo "Built AppImage in $DIST"

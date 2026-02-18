#!/usr/bin/env bash
set -euo pipefail

APP_NAME="uart-log-viewer"
VERSION="$(cat "$(cd "$(dirname "$0")/.." && pwd)/VERSION")"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT/build/deb"
PKG_DIR="$BUILD_DIR/${APP_NAME}_${VERSION}"

rm -rf "$BUILD_DIR"
mkdir -p "$PKG_DIR/DEBIAN" "$PKG_DIR/usr/bin" "$PKG_DIR/usr/share/applications"

cat > "$PKG_DIR/DEBIAN/control" <<CTRL
Package: ${APP_NAME}
Version: ${VERSION}
Section: utils
Priority: optional
Architecture: all
Maintainer: UART Tabs <support@local>
Depends: python3, python3-tk, python3-serial, tzdata, zenity
Description: Multi-UART log viewer with tabs, timestamps, and theming
 UART Tabs provides a GUI for multiple serial ports with per-port tabs,
 timestamping, timezone selection, find, log saving, and send box.
CTRL

install -m 755 "$ROOT/src/uart_tabs.py" "$PKG_DIR/usr/bin/uart-log-viewer"

cat > "$PKG_DIR/usr/share/applications/uart-tabs.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=UART Log Viewer
Comment=Multi-UART viewer with per-port tabs
Exec=uart-log-viewer
Terminal=false
Icon=utilities-terminal
Categories=Development;Utility;
DESKTOP

mkdir -p "$ROOT/dist"
dpkg-deb --build "$PKG_DIR" "$ROOT/dist/uart-log-viewer_${VERSION}_all.deb"

echo "Built: $ROOT/dist/uart-log-viewer_${VERSION}_all.deb"

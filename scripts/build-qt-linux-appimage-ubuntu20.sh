#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="$ROOT/build/qt-linux-appimage-ubuntu20"
TOOLS="$BUILD/tools"
QT_VERSION="${QT_VERSION:-6.5.3}"
QT_ARCH="${QT_ARCH:-gcc_64}"

mkdir -p "$BUILD" "$TOOLS"

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "Missing required command: $1"
    echo "Install build deps (Ubuntu): sudo apt-get install -y build-essential cmake ninja-build python3-venv curl"
    exit 1
  fi
}

require_cmd cmake
require_cmd ninja
require_cmd python3
require_cmd curl

VENV="$TOOLS/venv"
if [ ! -d "$VENV" ]; then
  python3 -m venv "$VENV"
fi

"$VENV/bin/python" -m pip install --upgrade pip aqtinstall

QT_BASE="$BUILD/qt"
QT_ROOT="$QT_BASE/$QT_VERSION/$QT_ARCH"
if [ ! -d "$QT_ROOT" ]; then
  "$VENV/bin/aqt" install-qt linux desktop "$QT_VERSION" "$QT_ARCH" -O "$QT_BASE" -m qtserialport
fi

QT_ROOT="$QT_ROOT" "$ROOT/scripts/build-qt-linux-appimage.sh"

echo "Built AppImage targeting Ubuntu 20.04+ (glibc 2.31) in dist/"

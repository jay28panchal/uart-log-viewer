$ErrorActionPreference = "Stop"

$ROOT = Split-Path -Parent $PSScriptRoot
$BUILD = Join-Path $ROOT "build\qt-win"
$DIST = Join-Path $ROOT "dist"

New-Item -ItemType Directory -Force -Path $BUILD | Out-Null
New-Item -ItemType Directory -Force -Path $DIST | Out-Null

cmake -S "$ROOT\cpp" -B $BUILD -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build $BUILD --target uart-log-viewer

$exe = Join-Path $BUILD "uart-log-viewer.exe"
if (!(Test-Path $exe)) {
  $exe = Join-Path $BUILD "Release\uart-log-viewer.exe"
}

# Deploy Qt runtime
windeployqt $exe

# Create installer using Inno Setup
iscc "$ROOT\scripts\installer-win-qt.iss"

Write-Host "Built installer in $DIST"

$ErrorActionPreference = "Stop"

$APP_NAME = "UART Log Viewer"
$VERSION = Get-Content (Join-Path $ROOT "VERSION")
$ROOT = Split-Path -Parent $PSScriptRoot
$DIST = Join-Path $ROOT "dist"
$BUILD = Join-Path $ROOT "build"

python -m pip install --upgrade pip
python -m pip install -r (Join-Path $ROOT "requirements.txt")
python -m pip install pyinstaller

$src = Join-Path $ROOT "src\uart_tabs.py"
pyinstaller --noconsole --name "UART Log Viewer" --clean --distpath $DIST --workpath $BUILD $src

Write-Host "Built EXE at $DIST\UART Log Viewer\UART Log Viewer.exe"
Write-Host "Optional: use Inno Setup to create an installer."
Write-Host "Example command (after installing Inno Setup):"
Write-Host "  iscc $ROOT\scripts\installer-win.iss"

# UART Log Viewer

Cross-platform serial monitor with tabs, timestamps, search, theming, and send box.

## Platforms
- Ubuntu 20.04+
- macOS 12+
- Windows 10+

## End-user installation
### Ubuntu (DEB)
Install the provided `.deb`:

```bash
sudo apt-get install ./uart-log-viewer_1.0.1_all.deb
```

### Windows/macOS
Distribute the prebuilt installers:
- Windows: `UART-Log-Viewer-Setup.exe`
- macOS: `UART-Log-Viewer.dmg`

No Python required on end-user machines when using the installers.

## Developer build
### Ubuntu build (DEB)
```bash
./scripts/build-deb.sh
```

### Windows build (EXE + installer)
```powershell
# run in Windows PowerShell
./scripts/build-win.ps1
```

### macOS build (App + DMG)
```bash
# run on macOS
./scripts/build-mac.sh
```

## Runtime dependencies (bundled)
- Python 3
- Tkinter
- PySerial

## Notes
- If no serial ports are shown, verify USB driver installation (CP210x/CH340/FTDI).

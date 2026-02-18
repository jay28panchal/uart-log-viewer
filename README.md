# UART Log Viewer

Cross-platform serial monitor with tabs, timestamps, search, theming, and send box.

## Minimum OS Requirements
- Ubuntu 20.04+
- macOS 12+
- Windows 10+

## End-user installation
### Ubuntu (AppImage, recommended)
Download and run the AppImage (no extra installs):

```bash
chmod +x uart-log-viewer_1.0.2-x86_64.AppImage
./uart-log-viewer_1.0.2-x86_64.AppImage
```

### Ubuntu (DEB, optional)
Build locally if you prefer a `.deb`:

```bash
./scripts/build-qt-linux.sh
sudo apt-get install ./dist/uart-log-viewer_1.0.2_amd64.deb
```

### Windows/macOS
Distribute the prebuilt installers:
- Windows: `UART-Log-Viewer-Setup.exe`
- macOS: `UART-Log-Viewer.dmg`

No Python required on end-user machines. Built with Qt.

## Developer build
### Ubuntu build (DEB, Qt)
```bash
./scripts/build-qt-linux.sh
```

### Windows build (EXE + installer, Qt)
```powershell
# run in Windows PowerShell
./scripts/build-qt-win.ps1
```

### macOS build (App + DMG, Qt)
```bash
# run on macOS
./scripts/build-qt-mac.sh
```

## Runtime dependencies (bundled)
- Qt 6
- Qt SerialPort

## Notes
- If no serial ports are shown, verify USB driver installation (CP210x/CH340/FTDI).

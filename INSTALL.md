# UART Log Viewer — Installation Guide

This guide explains how to install **UART Log Viewer** on Ubuntu, macOS, and Windows. No Qt or Python is required for end users.

## Build From Source (All Platforms)
Use this only if you want to build the installers yourself.

### 1) Clone the repo
```bash
git clone https://github.com/jay28panchal/uart-log-viewer.git
cd uart-log-viewer
```

### 2) Build installers
Choose your platform below.

## Ubuntu 20.04+ (Minimum)
### Install
**Recommended (No extra installs): AppImage**
1. Download the AppImage (example: `uart-log-viewer_1.0.2-x86_64.AppImage`).
2. Right‑click the file → **Properties** → enable **Allow executing file as program**.
3. Double‑click to run.

**Alternative: DEB (build locally)**
If you prefer a `.deb`, build it locally from source (see below).

### AppImage (Terminal)
```bash
chmod +x uart-log-viewer_1.0.2-x86_64.AppImage
./uart-log-viewer_1.0.2-x86_64.AppImage
```

### DEB (Terminal, build locally)
```bash
./scripts/build-qt-linux.sh
sudo apt-get install ./dist/uart-log-viewer_1.0.2_amd64.deb
```

### Download From GitHub (Terminal)
Replace `<version>` with the release tag (example: `v1.0.2`).

```bash
curl -L -o uart-log-viewer_1.0.2-x86_64.AppImage \
  https://github.com/jay28panchal/uart-log-viewer/releases/download/<version>/uart-log-viewer_1.0.2-x86_64.AppImage
chmod +x uart-log-viewer_1.0.2-x86_64.AppImage
./uart-log-viewer_1.0.2-x86_64.AppImage

# Optional .deb (build locally):
./scripts/build-qt-linux.sh
sudo apt-get install ./dist/uart-log-viewer_1.0.2_amd64.deb
```

### Launch
Open the application menu and run **UART Log Viewer**.

### If You See “Qt_6.5 not found”
You installed a package built against a newer Qt than your system provides.
Download the latest `.deb` from **GitHub Releases** and reinstall it; it is built against the system Qt version.
If you still see the error, install the Qt 6 runtime packages:

```bash
sudo apt-get install -y libqt6core6 libqt6gui6 libqt6widgets6 libqt6serialport6
```

### Upgrading From Older Versions
If you already have an older version installed, installing the new `.deb` will **upgrade** it automatically.
If you used the older Python/Tkinter package (`uart-tabs`), the new package will replace it.

### Build AppImage (From Source)
From the repo folder:
```bash
./scripts/build-qt-linux-appimage.sh
```
Then run the generated AppImage from `dist/`.

### Build DEB (From Source)
From the repo folder:
```bash
./scripts/build-qt-linux.sh
```
Then install the generated `.deb` from `dist/`.

---

## macOS 12+ (Minimum)
### Install
1. Download `UART-Log-Viewer.dmg`.
2. Double-click the `.dmg`.
3. Drag **UART Log Viewer** into **Applications**.

### Download From GitHub (Terminal)
Replace `<version>` with the release tag (example: `v1.0.2`).

```bash
curl -L -o UART-Log-Viewer.dmg \
  https://github.com/jay28panchal/uart-log-viewer/releases/download/<version>/UART-Log-Viewer.dmg
open UART-Log-Viewer.dmg
```

### Launch
Open **Applications** and run **UART Log Viewer**.

### Build Installer (From Source)
From the repo folder:
```bash
./scripts/build-qt-mac.sh
```
Then open `dist/UART-Log-Viewer.dmg` and drag the app into **Applications**.

### Notes
If macOS blocks the app:
1. Go to **System Settings -> Privacy & Security**.
2. Click **Open Anyway** for UART Log Viewer.

---

## Windows 10+ (Minimum)
### Install
1. Download `UART-Log-Viewer-Setup.exe`.
2. Double-click the installer and follow the wizard.

### Download From GitHub (PowerShell)
Replace `<version>` with the release tag (example: `v1.0.2`).

```powershell
Invoke-WebRequest -Uri "https://github.com/jay28panchal/uart-log-viewer/releases/download/<version>/UART-Log-Viewer-Setup.exe" -OutFile "UART-Log-Viewer-Setup.exe"
Start-Process .\\UART-Log-Viewer-Setup.exe
```

### Launch
Use Start Menu or Desktop shortcut (if chosen during install).

### Build Installer (From Source)
From the repo folder (PowerShell):
```powershell
./scripts/build-qt-win.ps1
```
Then open `dist\\UART-Log-Viewer-Setup.exe` and follow the wizard.

---

## USB Driver Notes (All Platforms)
If no serial ports appear, you may need a USB‑serial driver:
- CP210x (Silicon Labs)
- CH340 (WCH)
- FTDI

Install the correct driver for your device, then replug the USB cable.

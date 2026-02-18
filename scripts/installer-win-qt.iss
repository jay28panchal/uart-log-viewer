[Setup]
AppName=UART Log Viewer
AppVersion=1.0.1
DefaultDirName={pf}\UART Log Viewer
DefaultGroupName=UART Log Viewer
OutputBaseFilename=UART-Log-Viewer-Setup
OutputDir=..\\dist
Compression=lzma
SolidCompression=yes

[Files]
Source: "..\build\qt-win\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{group}\UART Log Viewer"; Filename: "{app}\uart-log-viewer.exe"
Name: "{commondesktop}\UART Log Viewer"; Filename: "{app}\uart-log-viewer.exe"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked

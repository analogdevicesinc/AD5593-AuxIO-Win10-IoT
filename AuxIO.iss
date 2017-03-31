#define MyAppName "AuxIO"

[Setup]
AppId={{D06C1142-29F1-4F9F-99C5-CAF437C4CB51}}
AppName={#MyAppName}
AppVersion="0.1"
AppPublisher="Analog Devices, Inc."
AppPublisherURL="http://www.analog.com"
AppSupportURL="https://ez.analog.com/"
AppUpdatesURL="https://github.com/analogdevicesinc/AD5593-AuxIO-Win10-IoT/releases"
OutputBaseFilename=AuxIO-setup
OutputDir=x64
DefaultDirName="{pf}\AuxIO"
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
MinVersion=10.0

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "x64/Release/AuxIO_v01.exe"; DestDir: "{app}"; Flags: ignoreversion;  Check: IsWin64
Source: "x64/LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion;
Source: "Readme.txt"; DestDir: "{app}"; Flags: isreadme
Source: "x64/vc_redist.x64.exe"; DestDir: {tmp}; Flags: deleteafterinstall

[Run]
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/q /norestart";  StatusMsg: "Installing VC++ redistributables..."


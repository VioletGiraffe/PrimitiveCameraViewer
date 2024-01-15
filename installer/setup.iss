[Setup]
AppName=Primitive Camera Viewer
AppId=PrimitiveCameraViewer
AppVerName=Primitive Camera Viewer
DefaultDirName={pf}\Primitive Camera Viewer
DefaultGroupName=Primitive Camera Viewer
AllowNoIcons=true
OutputDir=.
OutputBaseFilename=PrimitiveCameraViewer
UsePreviousAppDir=yes

UninstallDisplayIcon={app}\PrimitiveCameraViewer.exe
SetupIconFile=../app/resources/icon.ico

ShowTasksTreeLines=yes

SolidCompression=true
LZMANumBlockThreads=4
Compression=lzma2/ultra64
LZMAUseSeparateProcess=yes
LZMABlockSize=8192

[Tasks]
Name: startup; Description: "Automatically run the program when Windows starts"; GroupDescription: "{cm:AdditionalIcons}";

[Files]

;App binaries
Source: binaries/*; DestDir: {app}; Flags: ignoreversion

;Qt binaries
Source: binaries/Qt/*; DestDir: {app}; Flags: ignoreversion recursesubdirs

;MSVC binaries
Source: binaries/64/msvcr/*; DestDir: {app}; Flags: ignoreversion

[Icons]
Name: {group}\Primitive Camera Viewer; Filename: {app}\PrimitiveCameraViewer.exe;
Name: {group}\{cm:UninstallProgram,Primitive Camera Viewer}; Filename: {uninstallexe}
Name: {userstartup}\Primitive Camera Viewer; Filename: {app}\PrimitiveCameraViewer.exe; Tasks: startup

[Run]
Filename: {app}\PrimitiveCameraViewer.exe; Description: {cm:LaunchProgram,Primitive Camera Viewer}; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: dirifempty; Name: "{app}"
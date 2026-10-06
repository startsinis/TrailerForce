#ifndef Version
#define Version "0.1.0"
#endif
[Setup]
AppId={{BEF537ED-CDA8-421B-8A91-93DDE5368754}
AppName=Trailer Force by Vinci Sounds
AppVersion={#Version}
AppPublisher=Vinci Sounds
DefaultDirName={autopf}\Vinci Sounds\Trailer Force
DisableDirPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
OutputDir=..\dist
OutputBaseFilename=TrailerForce-{#Version}-Windows-x64-Setup
Compression=lzma2
SolidCompression=yes
UninstallDisplayName=Trailer Force by Vinci Sounds
WizardStyle=modern
[Files]
Source: "..\build\TrailerForce_artefacts\Release\VST3\Trailer Force.vst3\*"; DestDir: "{commoncf64}\VST3\Trailer Force.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\docs\USER-MANUAL.md"; DestDir: "{app}"; Flags: ignoreversion
[Icons]
Name: "{group}\Trailer Force User Manual"; Filename: "{app}\USER-MANUAL.md"
Name: "{group}\Uninstall Trailer Force"; Filename: "{uninstallexe}"

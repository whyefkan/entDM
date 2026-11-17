#define AppName "entDM"
#define AppVersion "0.1.0"
#define AppPublisher "whyefkan"
#define AppUrl "https://github.com/whyefkan/entDM"

[Setup]
AppId={{A1F1FF0F-B5FD-4A48-9CFF-DFBD6584A1C5}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppUrl}
AppSupportURL={#AppUrl}/issues
AppUpdatesURL={#AppUrl}/releases
DefaultDirName={autopf}\entDM
DefaultGroupName=entDM
DisableProgramGroupPage=yes
OutputDir=..\..\dist
OutputBaseFilename=entdm-{#AppVersion}-windows-x64-setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog
CloseApplications=yes
UninstallDisplayIcon={app}\entdm.ico
SetupIconFile=..\..\assets\entdm.ico

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional icons:"; Flags: unchecked

[Files]
Source: "..\..\dist\package\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\entDM"; Filename: "{app}\entdm.exe"; IconFilename: "{app}\entdm.ico"
Name: "{group}\Uninstall entDM"; Filename: "{uninstallexe}"
Name: "{autodesktop}\entDM"; Filename: "{app}\entdm.exe"; IconFilename: "{app}\entdm.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\entdm.exe"; Description: "Launch entDM"; Flags: nowait postinstall skipifsilent

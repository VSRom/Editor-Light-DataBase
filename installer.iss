[Setup]
AppName=Editor Light DataBase
AppVersion=0.1-alpha
AppPublisher=Your Name
DefaultDirName={autopf}\EditorLightDataBase
DefaultGroupName=Editor Light DataBase
OutputDir=installer_output
OutputBaseFilename=EditorLightDataBase-Setup-0.1-alpha
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\Editor-Light-DataBase.exe
WizardStyle=modern
DisableProgramGroupPage=yes

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Создать ярлык на рабочем столе"; GroupDescription: "Дополнительные ярлыки:"; Flags: unchecked

[Files]
Source: "out\build\release\Editor-Light-DataBase.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "out\build\release\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "out\build\release\sqldrivers\*"; DestDir: "{app}\sqldrivers"; Flags: ignoreversion recursesubdirs
Source: "out\build\release\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs
Source: "out\build\release\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs
Source: "out\build\release\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs skipifsourcedoesntexist
Source: "out\build\release\drivers\*"; DestDir: "{app}\drivers"; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{group}\Editor Light DataBase"; Filename: "{app}\Editor-Light-DataBase.exe"
Name: "{group}\Удалить Editor Light DataBase"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Editor Light DataBase"; Filename: "{app}\Editor-Light-DataBase.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\Editor-Light-DataBase.exe"; Description: "Запустить Editor Light DataBase"; Flags: nowait postinstall skipifsilent unchecked
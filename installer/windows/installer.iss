; ============================================================================
;  uz++ Windows installer (Inno Setup 6.x)
; ----------------------------------------------------------------------------
;  Builds `uzpp-setup.exe` containing:
;    * uzpp.exe (compiler frontend)
;    * stdlib\ (header-only standard library)
;    * misollar\ (canonical examples)
;
;  Notes:
;    * MinGW is NOT bundled. The VS Code extension downloads MinGW from
;      WinLibs on first run; CLI users follow docs/getting-started.md.
;    * Default install dir: %LOCALAPPDATA%\Programs\uzpp\ (no admin required).
;      Power users can re-target to Program Files via the Browse dialog —
;      Inno Setup will then prompt for elevation automatically.
;    * Adds the install dir to the *user* PATH (HKCU). System-wide PATH
;      modification would require admin rights and is intentionally avoided.
;
;  Build (run from this directory):
;      iscc installer.iss
;  Output: Output\uzpp-setup.exe
; ============================================================================

#define AppName        "uz++"
#define AppPublisher   "timetolivechk-spec"
#define AppURL         "https://github.com/timetolivechk-spec/uzpp"
#define AppExe         "uzpp.exe"

; Version is read from CLI override (e.g. /DAppVersion=2.1.1) or falls back.
#ifndef AppVersion
  #define AppVersion   "2.1.1"
#endif

; Source paths are resolved relative to this .iss file. The build script in
; README.md copies the build artefacts into ..\..\dist\ before invoking iscc.
#ifndef SourceRoot
  #define SourceRoot   "..\..\dist"
#endif

[Setup]
AppId={{B4F1A7C0-2026-4B59-9A4D-UZPP00000001}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}/issues
AppUpdatesURL={#AppURL}/releases
DefaultDirName={localappdata}\Programs\uzpp
DefaultGroupName=uz++
DisableProgramGroupPage=yes
DisableDirPage=no
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir=Output
OutputBaseFilename=uzpp-setup
Compression=lzma2/ultra
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\{#AppExe}
UninstallDisplayName={#AppName} {#AppVersion}
ChangesEnvironment=yes
LicenseFile={#SourceRoot}\LICENSE.txt

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "addtopath";   Description: "{cm:AddToPathTask}";        GroupDescription: "{cm:IntegrationGroup}"; Flags: checkedonce
Name: "associateuzpp"; Description: "{cm:AssociateUzppTask}";  GroupDescription: "{cm:IntegrationGroup}"; Flags: checkedonce
Name: "startmenu";   Description: "{cm:StartMenuTask}";        GroupDescription: "{cm:IntegrationGroup}"; Flags: checkedonce

[CustomMessages]
english.AddToPathTask=Add uz++ to my user PATH (recommended)
english.AssociateUzppTask=Open .uzpp files with uz++ by default
english.StartMenuTask=Create a Start Menu shortcut
english.IntegrationGroup=System integration
russian.AddToPathTask=Добавить uz++ в пользовательский PATH (рекомендуется)
russian.AssociateUzppTask=Открывать .uzpp по умолчанию через uz++
russian.StartMenuTask=Создать ярлык в меню Пуск
russian.IntegrationGroup=Интеграция с системой

[Files]
Source: "{#SourceRoot}\uzpp.exe";   DestDir: "{app}";          Flags: ignoreversion
Source: "{#SourceRoot}\LICENSE.txt"; DestDir: "{app}";         Flags: ignoreversion
Source: "{#SourceRoot}\stdlib\*";   DestDir: "{app}\stdlib";   Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#SourceRoot}\misollar\*"; DestDir: "{app}\misollar"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#SourceRoot}\README.txt"; DestDir: "{app}";          Flags: ignoreversion isreadme

[Icons]
Name: "{group}\uz++ examples"; Filename: "{app}\misollar"; Tasks: startmenu
Name: "{group}\uz++ on GitHub"; Filename: "{#AppURL}";    Tasks: startmenu

[Registry]
; Add install dir to user PATH (HKCU\Environment) — only if task is checked.
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "Path"; \
    ValueData: "{olddata};{app}"; Tasks: addtopath; Check: NeedsAddPath('{app}'); \
    Flags: preservestringtype

; Register .uzpp file association (HKCU so no admin required).
Root: HKCU; Subkey: "Software\Classes\.uzpp"; ValueType: string; ValueName: ""; \
    ValueData: "uzpp.SourceFile"; Tasks: associateuzpp; Flags: uninsdeletevalue
Root: HKCU; Subkey: "Software\Classes\uzpp.SourceFile"; ValueType: string; \
    ValueName: ""; ValueData: "uz++ source file"; Tasks: associateuzpp; \
    Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\uzpp.SourceFile\DefaultIcon"; ValueType: string; \
    ValueName: ""; ValueData: """{app}\{#AppExe}"",0"; Tasks: associateuzpp
; Open default = "uzpp ishga-tushirish <file>"
Root: HKCU; Subkey: "Software\Classes\uzpp.SourceFile\shell\open\command"; \
    ValueType: string; ValueName: ""; \
    ValueData: """{app}\{#AppExe}"" ishga-tushirish ""%1"""; Tasks: associateuzpp
; "Edit with VS Code" (only registered; user still needs Code installed)
Root: HKCU; Subkey: "Software\Classes\uzpp.SourceFile\shell\edit\command"; \
    ValueType: string; ValueName: ""; \
    ValueData: "code ""%1"""; Tasks: associateuzpp

[Code]
function NeedsAddPath(Param: string): boolean;
var
  OrigPath: string;
begin
  if not RegQueryStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', OrigPath)
  then begin
    Result := True;
    exit;
  end;
  // Compare case-insensitively, treat both with and without trailing backslash.
  Result := Pos(';' + Lowercase(Param) + ';', ';' + Lowercase(OrigPath) + ';') = 0;
end;

[Run]
; Optional: launch the examples folder when finished, so the user can try
; misollar\01_salom_dunyo.uzpp immediately.
Filename: "{win}\explorer.exe"; Parameters: """{app}\misollar"""; \
    Description: "{cm:LaunchExamples}"; Flags: nowait postinstall skipifsilent

[CustomMessages]
english.LaunchExamples=Open the examples folder
russian.LaunchExamples=Открыть папку с примерами

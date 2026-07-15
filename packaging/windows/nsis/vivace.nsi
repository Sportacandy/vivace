; Vivace - NSIS installer (primary Windows installer).
;
; Installs the Qt-deployed tree (bin/ + plugins/ qml/ translations/ qt.conf,
; produced by `cmake --install`) that build_installer.ps1 stages, plus Start
; Menu / optional Desktop shortcuts, an uninstaller, and the "Installed apps"
; registry entry. Supports both install scopes (all users / just me) via
; MultiUser, like the Qt IFW installer's install-scope page.
;
; Defines passed by build_installer.ps1 (with fallbacks for a manual build):
;   VERSION  product version           DEPLOY  staged deploy tree
;   OUTFILE  output installer path      ICON    brand .ico
;   LICENSE  license text file
;
; Names mirror the IFW installer: the "Installed apps"/Start Menu entry is
; "Vivace Media Player"; the desktop shortcut and install dir are "Vivace".

Unicode true

!define APPNAME      "Vivace Media Player"
!define SHORTNAME    "Vivace"
!define PUBLISHER    "Vivace"
!define EXE          "bin\vivace.exe"
!define AUMID        "VivacePlayer.Vivace"   ; must match main.cpp's AUMID
!define UNINST_KEY   "Software\Microsoft\Windows\CurrentVersion\Uninstall\Vivace"

!ifndef VERSION
  !define VERSION "0.1.0"
!endif
!ifndef DEPLOY
  !define DEPLOY "..\packages\org.vivaceplayer.vivace\data"
!endif
!ifndef OUTFILE
  !define OUTFILE "..\..\..\VivaceSetup-NSIS.exe"
!endif
!ifndef ICON
  !define ICON "..\..\..\icons\vivace.ico"
!endif
!ifndef LICENSE
  !define LICENSE "..\..\..\LICENSE"
!endif

; ---- MultiUser (all-users vs per-user), integrated with MUI -----------------
; Vivace is 64-bit, so the all-users default must be the 64-bit Program Files
; (a 32-bit NSIS installer otherwise defaults to "Program Files (x86)").
!define MULTIUSER_EXECUTIONLEVEL Highest
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_COMMANDLINE
!define MULTIUSER_INSTALLMODE_INSTDIR "${SHORTNAME}"
!define MULTIUSER_USE_PROGRAMFILES64
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "${UNINST_KEY}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME "InstallLocation"
!include MultiUser.nsh
!include MUI2.nsh
!include LogicLib.nsh

Name "${APPNAME}"
OutFile "${OUTFILE}"
VIProductVersion "${VERSION}.0"
VIAddVersionKey "ProductName" "${APPNAME}"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "CompanyName" "${PUBLISHER}"
VIAddVersionKey "FileDescription" "${APPNAME} installer"
VIAddVersionKey "LegalCopyright" "GPL-3.0-or-later"
SetCompressor /SOLID lzma

!define MUI_ICON   "${ICON}"
!define MUI_UNICON "${ICON}"
!define MUI_ABORTWARNING

; Brand images (replace the NSIS defaults). Composed from the brand logo;
; ${__FILEDIR__} resolves them next to this script regardless of the build CWD.
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP        "${__FILEDIR__}\vivace-header.bmp"
!define MUI_HEADERIMAGE_UNBITMAP      "${__FILEDIR__}\vivace-header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP  "${__FILEDIR__}\vivace-wizard.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${__FILEDIR__}\vivace-wizard.bmp"

; ---- Installer pages --------------------------------------------------------
; No MUI_PAGE_STARTMENU: that page has TWO independent ways to end up not
; creating a shortcut - an explicit "do not create" checkbox (which
; MUI_STARTMENUPAGE_NODISABLE could remove) AND a separate ">Do not create a
; Start Menu folder" pseudo-entry in the folder list itself, with no public
; flag to disable it (see StartMenu.nsh: a folder value starting with ">"
; skips CreateShortCut, and whichever value the user leaves the page with is
; persisted to the registry and silently reused as the "last used" value on
; every future install/reinstall). NODISABLE alone did not fix this - the
; shortcut disappeared again on a later reinstall. Vivace does not need a
; user-chosen Start Menu folder name, so skip the page and use a fixed
; folder instead (see Section "!${APPNAME}" and Section "Uninstall").
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${LICENSE}"
!insertmacro MULTIUSER_PAGE_INSTALLMODE
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION LaunchAppDeElevated
!define MUI_FINISHPAGE_RUN_TEXT "Launch ${SHORTNAME}"
!insertmacro MUI_PAGE_FINISH

; ---- Uninstaller pages ------------------------------------------------------
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Function .onInit
  SetRegView 64   ; 64-bit app: keep registry out of WOW6432Node
  !insertmacro MULTIUSER_INIT
FunctionEnd

Function un.onInit
  SetRegView 64
  !insertmacro MULTIUSER_UNINIT
FunctionEnd

; "Launch Vivace" on the finish page. When MULTIUSER_EXECUTIONLEVEL Highest
; elevated this installer (an administrator installing for all users), a
; plain Exec here would launch Vivace with the SAME elevated (high-integrity)
; token. Windows then blocks drag-and-drop from Explorer (medium integrity)
; into that window via UIPI - the drop cursor shows "not allowed" and
; releasing the file does nothing. Vivace needs no elevation to run, so hand
; the launch to the already-running, non-elevated explorer.exe instead: it
; creates the process at the interactive user's normal integrity level,
; decoupled from this installer's token.
Function LaunchAppDeElevated
  Exec '"$WINDIR\explorer.exe" "$INSTDIR\${EXE}"'
FunctionEnd

; ---- Sections ---------------------------------------------------------------
Section "!${APPNAME}" SecMain
  SectionIn RO
  SetOutPath "$INSTDIR"
  ; set-aumid.ps1 is only for the IFW installer; NSIS sets the AUMID natively
  ; (StampAUMID), so it isn't shipped here.
  File /r /x "set-aumid.ps1" "${DEPLOY}\*"

  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; "Installed apps" / Programs and Features entry (SHELL_CONTEXT = HKLM for
  ; all-users, HKCU for per-user, chosen on the install-mode page).
  WriteRegStr   SHELL_CONTEXT "${UNINST_KEY}" "DisplayName"     "${APPNAME}"
  WriteRegStr   SHELL_CONTEXT "${UNINST_KEY}" "DisplayIcon"     "$INSTDIR\${EXE},0"
  WriteRegStr   SHELL_CONTEXT "${UNINST_KEY}" "DisplayVersion"  "${VERSION}"
  WriteRegStr   SHELL_CONTEXT "${UNINST_KEY}" "Publisher"       "${PUBLISHER}"
  WriteRegStr   SHELL_CONTEXT "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr   SHELL_CONTEXT "${UNINST_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr   SHELL_CONTEXT "${UNINST_KEY}" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
  WriteRegDWORD SHELL_CONTEXT "${UNINST_KEY}" "NoModify" 1
  WriteRegDWORD SHELL_CONTEXT "${UNINST_KEY}" "NoRepair" 1

  ; No subfolder: a single shortcut directly under $SMPROGRAMS is reachable
  ; straight from "All apps" ▸ V, rather than one extra click into a
  ; Vivace-only folder holding just this one shortcut (user directive,
  ; 2026-07-15). No separate "Uninstall" shortcut either - Settings/Control
  ; Panel already offers uninstall regardless of install scope, so a Start
  ; Menu entry for it was redundant.
  ;
  ; Clean up a PREVIOUS install's subfolder-based layout so an upgrade from
  ; an older build doesn't leave both the old folder and the new flat
  ; shortcut behind.
  Delete "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk"
  Delete "$SMPROGRAMS\${APPNAME}\Uninstall ${SHORTNAME}.lnk"
  RMDir  "$SMPROGRAMS\${APPNAME}"
  ; On a first-time install this shows up in Start Menu's "All apps" list
  ; immediately, but on an OVERWRITE install (the .lnk already exists) it can
  ; silently fail to appear there even though the file is written correctly
  ; (confirmed on disk) - CreateShortCut overwrites the file's bytes in
  ; place, which is not the same thing Explorer's directory watch treats as
  ; a genuine "file created" event, and NSIS's CreateShortCut does not tell
  ; the shell about the change itself. Same class of gotcha this project
  ; already hit with file-type associations (see
  ; UiHelpers::setFileAssociations, src/uihelpers.cpp - SHChangeNotify
  ; needed there too since Explorer doesn't always notice script-made shell
  ; changes on its own). Fix: delete the old .lnk first so the following
  ; CreateShortCut is a genuine create, then explicitly notify the shell.
  Delete "$SMPROGRAMS\${APPNAME}.lnk"
  CreateShortCut "$SMPROGRAMS\${APPNAME}.lnk" "$INSTDIR\${EXE}"
  Push "$SMPROGRAMS\${APPNAME}.lnk"
  Call StampAUMID
  ; SHCNE_UPDATEDIR on $SMPROGRAMS (its contents changed) + SHCNE_CREATE on
  ; the shortcut itself (SHCNF_PATHW since this is a Unicode build passing
  ; wide-string paths).
  System::Call 'shell32::SHChangeNotify(i 0x1000, i 0x0005, w "$SMPROGRAMS", i 0)'
  System::Call 'shell32::SHChangeNotify(i 0x0002, i 0x0005, w "$SMPROGRAMS\${APPNAME}.lnk", i 0)'
  ; Neither SHChangeNotify call was enough on an overwrite install (confirmed
  ; by the user: the .lnk is genuinely fresh on disk, still missing from "All
  ; apps"). Modern Windows renders that list via a separate process
  ; (StartMenuExperienceHost) with its own cache, which may not react to the
  ; classic shell notifications above the same way Explorer itself does.
  ; Broadcast WM_SETTINGCHANGE to all top-level windows as a broader nudge -
  ; a standard technique for getting shell components to re-read cached
  ; state after an installer changes something outside the normal registry/
  ; environment paths those windows already watch.
  System::Call 'user32::SendMessageTimeout(i 0xFFFF, i 0x001A, i 0, i 0, i 2, i 5000, *i .r0)'
SectionEnd

; Set System.AppUserModel.ID on a shortcut so the SMTC media flyout resolves to
; this app (must match main.cpp's AUMID). Done with the bundled System plugin
; (COM: IShellLink -> IPersistFile::Load -> IPropertyStore::SetValue(
; PKEY_AppUserModel_ID) -> Commit -> IPersistFile::Save) rather than shelling
; out to PowerShell, which anti-virus tools may quarantine. Arg: shortcut path.
Function StampAUMID
  Exch $0                 ; $0 = .lnk path
  Push $1
  Push $2
  Push $3
  Push $4
  Push $5
  Push $6
  Push $7
  Push $8
  StrCpy $1 "${AUMID}"
  System::Call 'ole32::CoInitialize(i0)'
  ; CLSID_ShellLink, IID_IShellLinkW
  System::Call 'ole32::CoCreateInstance(g"{00021401-0000-0000-C000-000000000046}",i0,i1,g"{000214F9-0000-0000-C000-000000000046}",*i.r2)i.r3'
  ${If} $3 == 0
    ; IShellLink::QueryInterface(IID_IPersistFile) [vtable 0]
    System::Call '$2->0(g"{0000010B-0000-0000-C000-000000000046}",*i.r4)i.r3'
    ${If} $3 == 0
      ; IPersistFile::Load(path, STGM_READWRITE) [vtable 5]
      System::Call '$4->5(w r0,i2)i.r3'
      ; IShellLink::QueryInterface(IID_IPropertyStore) [vtable 0]
      System::Call '$2->0(g"{886D8EEB-8CF2-4446-8D02-CDBA1DBDCF99}",*i.r5)i.r3'
      ${If} $3 == 0
        ; PROPERTYKEY PKEY_AppUserModel_ID {9F4C2855-...} pid 5; PROPVARIANT VT_LPWSTR(31)
        System::Call '*(&g16"{9F4C2855-9F79-4B39-A8D0-E1D42DE1D5F3}",i5)i.r6'
        System::Call '*(&w260 r1)i.r7'
        System::Call '*(&i2 31,&i2 0,&i2 0,&i2 0,i r7,i0)i.r8'
        ; IPropertyStore::SetValue [6], Commit [7]; IPersistFile::Save [6]
        System::Call '$5->6(i r6,i r8)i.r3'
        System::Call '$5->7()i.r3'
        System::Call '$4->6(w r0,i1)i.r3'
        System::Free $6
        System::Free $7
        System::Free $8
        System::Call '$5->2()'
      ${EndIf}
      System::Call '$4->2()'
    ${EndIf}
    System::Call '$2->2()'
  ${EndIf}
  System::Call 'ole32::CoUninitialize()'
  Pop $8
  Pop $7
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
FunctionEnd

Section "Desktop shortcut" SecDesktop
  CreateShortCut "$DESKTOP\${SHORTNAME}.lnk" "$INSTDIR\${EXE}"
SectionEnd

LangString DESC_SecMain     ${LANG_ENGLISH} "${APPNAME} and its runtime (required)."
LangString DESC_SecDesktop  ${LANG_ENGLISH} "Add a shortcut named ${SHORTNAME} on the desktop."
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMain}    $(DESC_SecMain)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} $(DESC_SecDesktop)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; ---- Uninstall --------------------------------------------------------------
Section "Uninstall"
  Delete "$SMPROGRAMS\${APPNAME}.lnk"
  ; Also clean up a pre-2026-07-15 subfolder-based install, if present.
  Delete "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk"
  Delete "$SMPROGRAMS\${APPNAME}\Uninstall ${SHORTNAME}.lnk"
  RMDir  "$SMPROGRAMS\${APPNAME}"
  Delete "$DESKTOP\${SHORTNAME}.lnk"

  RMDir /r "$INSTDIR"
  DeleteRegKey SHELL_CONTEXT "${UNINST_KEY}"

  ; Same Start Menu "All apps" cache-refresh gotcha as the install side
  ; (see Section "!${APPNAME}"), mirrored here: without an explicit nudge,
  ; the now-removed shortcut can keep showing as a stale entry. SHCNE_DELETE
  ; on the removed shortcut + SHCNE_UPDATEDIR on $SMPROGRAMS (not the
  ; CREATE/UPDATEDIR pair used on install - this is a removal).
  System::Call 'shell32::SHChangeNotify(i 0x0004, i 0x0005, w "$SMPROGRAMS\${APPNAME}.lnk", i 0)'
  System::Call 'shell32::SHChangeNotify(i 0x1000, i 0x0005, w "$SMPROGRAMS", i 0)'
  System::Call 'user32::SendMessageTimeout(i 0xFFFF, i 0x001A, i 0, i 0, i 2, i 5000, *i .r0)'
SectionEnd

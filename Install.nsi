;--------------------------------
; Includes

  !include "MUI2.nsh"
  !include "logiclib.nsh"
  !include "x64.nsh"

;--------------------------------
; Custom defines
  !define APPNAME "Gigi"
  !define DESCRIPTION "Rapid Graphics Development Platform"
  !define VERSIONMAJOR 0
  !define VERSIONMINOR 99
  !define VERSIONBUILD 7
  !define SLUG "${APPNAME} v${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}"

  # These will be displayed by the "Click here for support information" link in "Add/Remove Programs"
  # It is possible to use "mailto:" links in here to open the email client
  !define HELPURL "https://github.com/electronicarts/gigi" # "Support Information" link
  !define UPDATEURL "https://github.com/electronicarts/gigi" # "Product Updates" link
  !define ABOUTURL "https://github.com/electronicarts/gigi" # "Publisher" link

;--------------------------------
; General

  Name "${APPNAME}"
  OutFile "Gigi-${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}.x64.windows.installer.exe"
  InstallDir "$PROGRAMFILES64\${APPNAME}"
  InstallDirRegKey HKCU "Software\${APPNAME}" ""
  RequestExecutionLevel admin

;--------------------------------
; UI

  !define MUI_ICON "GigiEdit.ico"
  !define MUI_HEADERIMAGE
  ;!define MUI_WELCOMEFINISHPAGE_BITMAP "assets\welcome.bmp"
  ;!define MUI_HEADERIMAGE_BITMAP "assets\head.bmp"
  !define MUI_ABORTWARNING
  !define MUI_WELCOMEPAGE_TITLE "${SLUG} Setup"

;--------------------------------
; Pages

  ; Installer pages
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
  ;!insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  ; Uninstaller pages
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

  ; Set UI language
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Section - Install App

  Section "-hidden app"
    SectionIn RO
    SetOutPath "$INSTDIR"
    File /r "*.*"
    WriteRegStr HKCU "Software\${APPNAME}" "" $INSTDIR
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    # Start Menu
    createDirectory "$SMPROGRAMS\${APPNAME}"
    createShortCut "$SMPROGRAMS\${APPNAME}\GigiBrowser.lnk" "$INSTDIR\GigiBrowser.exe" "" "$INSTDIR\GigiBrowser.ico"
    createShortCut "$SMPROGRAMS\${APPNAME}\GigiEdit.lnk" "$INSTDIR\GigiEdit.exe" "" "$INSTDIR\GigiEdit.ico"
    createShortCut "$SMPROGRAMS\${APPNAME}\GigiViewerDX12.lnk" "$INSTDIR\GigiViewerDX12.exe" "" "$INSTDIR\GigiViewerDX12.ico"

    # Registry information for add/remove programs
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "DisplayName" "${APPNAME} - ${DESCRIPTION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "InstallLocation" "$\"$INSTDIR$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "DisplayIcon" "$\"$INSTDIR\GigiEdit.ico$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "Publisher" "$\"${APPNAME}$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "HelpLink" "$\"${HELPURL}$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "URLUpdateInfo" "$\"${UPDATEURL}$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "URLInfoAbout" "$\"${ABOUTURL}$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "DisplayVersion" "$\"${VERSIONMAJOR}.${VERSIONMINOR}.${VERSIONBUILD}$\""
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "VersionMajor" ${VERSIONMAJOR}
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "VersionMinor" ${VERSIONMINOR}
    # There is no option for modifying or repairing the install
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}" "NoRepair" 1

    # register file extensions
    WriteRegStr HKCR ".gg" ""  "GigiEdit"
    WriteRegStr HKCR "GigiEdit\shell\open\command" "" "$\"$INSTDIR\GigiEdit.exe$\" $\"%1$\""

  SectionEnd

;--------------------------------
; Remove empty parent directories

  Function un.RMDirUP
    !define RMDirUP '!insertmacro RMDirUPCall'

    !macro RMDirUPCall _PATH
          push '${_PATH}'
          Call un.RMDirUP
    !macroend

    ; $0 - current folder
    ClearErrors

    Exch $0
    ;DetailPrint "ASDF - $0\.."
    RMDir "$0\.."

    IfErrors Skip
    ${RMDirUP} "$0\.."
    Skip:

    Pop $0

  FunctionEnd

;--------------------------------
; Section - Uninstaller

Section "Uninstall"

  # Remove Start Menu launcher
  delete "$SMPROGRAMS\${APPNAME}\GigiBrowser.lnk"
  delete "$SMPROGRAMS\${APPNAME}\GigiEdit.lnk"
  delete "$SMPROGRAMS\${APPNAME}\GigiViewerDX12.lnk"
  # Try to remove the Start Menu folder - this will only happen if it is empty
  rmDir "$SMPROGRAMS\${APPNAME}"

  ;Delete Uninstall
  Delete "$INSTDIR\Uninstall.exe"

  ;Delete Folder
  RMDir /r "$INSTDIR"
  ${RMDirUP} "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\${APPNAME}"

  # Remove uninstaller information from the registry
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME} ${APPNAME}"

  # unregister file extensions
  DeleteRegKey HKCR ".gg"
  DeleteRegKey HKCR "GigiEdit\shell\open\command"

SectionEnd
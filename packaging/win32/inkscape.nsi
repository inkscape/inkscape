; #######################################
; Inkscape NSIS installer project file
; Used as of 0.40
; #######################################

; #######################################
; DEFINES
; #######################################
!define PRODUCT_NAME "Inkscape"
!define PRODUCT_VERSION "0.46+devel"
!define PRODUCT_REVISION 1
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\inkscape.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
;!define DUMMYINSTALL ; Define this to make it build quickly, not including any of the files or code in the sections, for quick testing of features of the installer and development thereof.

; #######################################
; MUI   SETTINGS
; #######################################
; MUI 1.67 compatible ------
SetCompressor /SOLID lzma
SetCompressorDictSize 32
RequestExecutionLevel highest
!include "MUI.nsh"
!include "LogicLib.nsh"
!include "sections.nsh"
!define MUI_ABORTWARNING
!define MUI_ICON "..\..\inkscape.ico";${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "welcomefinish.bmp"
!define MUI_COMPONENTSPAGE_SMALLDESC

;..................................................................................................
;Following two definitions required. Uninstall log will use these definitions.
;You may use these definitions also, when you want to set up the InstallDirRagKey,
;store the language selection, store Start Menu folder etc.
;Enter the windows uninstall reg sub key to add uninstall information to Add/Remove Programs also.

!define INSTDIR_REG_ROOT HKLM
!define INSTDIR_REG_KEY ${PRODUCT_UNINST_KEY}

;include the Uninstall log header
!include AdvUninstLog.nsh
!insertmacro INTERACTIVE_UNINSTALL

;For md5dll and messagebox
!addplugindir .

!include FileFunc.nsh
!insertmacro GetParameters
!insertmacro GetOptions
!insertmacro un.GetParent

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
; !define MUI_LICENSEPAGE_RADIOBUTTONS
LicenseForceSelection off
!define MUI_LICENSEPAGE_BUTTON $(lng_LICENSE_BUTTON)
!define MUI_LICENSEPAGE_TEXT_BOTTOM $(lng_LICENSE_BOTTOM_TEXT)
!insertmacro MUI_PAGE_LICENSE "..\..\Copying"
!insertmacro MUI_PAGE_COMPONENTS
; InstType $(lng_Full)
; InstType $(lng_Optimal)
; InstType $(lng_Minimal)
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\inkscape.exe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
UninstPage custom un.CustomPageUninstall
!insertmacro MUI_UNPAGE_INSTFILES
ShowUninstDetails hide
!insertmacro MUI_UNPAGE_FINISH

; #######################################
; STRING   LOCALIZATION
; #######################################
; Thanks to Adib Taraben and Luca Bruno for getting this started
; Add your translation here!  :-)
; I had wanted to list the languages alphabetically, but apparently
; the first is the default.  So putting English first is just being
; practical.  It is not chauvinism or hubris, I swear!  ;-)
; default language first

; Language files
!insertmacro MUI_RESERVEFILE_LANGDLL
!include English.nsh
!include Breton.nsh
!include Catalan.nsh
!include Czech.nsh
!include Finnish.nsh
!include French.nsh
!include Galician.nsh
!include German.nsh
!include Italian.nsh
!include Japanese.nsh
!include Polish.nsh
!include Russian.nsh
!include Slovak.nsh
!include Slovenian.nsh
!include Spanish.nsh
!include SimpChinese.nsh
!include TradChinese.nsh

ReserveFile "inkscape.nsi.uninstall"
ReserveFile "${NSISDIR}\Plugins\UserInfo.dll"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; #######################################
; SETTINGS
; #######################################

Name              "${PRODUCT_NAME}"
Caption           "$(lng_Caption)"
BrandingText      "$(lng_Caption)"
!ifndef PRODUCT_REVISION
OutFile           "Inkscape-${PRODUCT_VERSION}.exe"
!else
OutFile           "Inkscape-${PRODUCT_VERSION}-${PRODUCT_REVISION}.exe"
!endif
InstallDir        "$PROGRAMFILES\Inkscape"
InstallDirRegKey  HKLM "${PRODUCT_DIR_REGKEY}" ""

Var askMultiUser
Var filename
Var MultiUser
Var User
Var CMDARGS

; #######################################
;  I N S T A L L E R    S E C T I O N S
; #######################################

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;
; Delete prefs
; code originally taken from the vlc project
;;;;;;;;;;;;;;;;;;;;;;;;;;
!macro delprefs
  StrCpy $0 0
  DetailPrint "Deleting personal preferences..."
  DetailPrint "Finding all users..."
  ${Do}
 ; FIXME
  ; this will loop through all the logged users and "virtual" windows users
  ; (it looks like users are only present in HKEY_USERS when they are logged in)
    ClearErrors
    EnumRegKey $1 HKU "" $0
    ${IfThen} $1 == "" ${|} ${ExitDo} ${|}
    IntOp $0 $0 + 1
    ReadRegStr $2 HKU "$1\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" AppData
    ${IfThen} $2 == "" ${|} ${Continue} ${|}
    DetailPrint "Removing $2\Inkscape"
    Delete "$2\Inkscape\preferences.xml"
    Delete "$2\Inkscape\extension-errors.log"
    RMDir "$2\Inkscape"
  ${Loop}
!macroend


;--------------------------------
; Installer Sections
Section -removeInkscape
!ifndef DUMMYINSTALL
  ;remove the old Inkscape shortcuts from the startmenu
  ;just in case they are still there
  SetShellVarContext current
  Delete "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape\Inkscape.lnk"
  RMDir  "$SMPROGRAMS\Inkscape"
  Delete "$SMPROGRAMS\Inkscape.lnk"
  SetShellVarContext all
  Delete "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape\Inkscape.lnk"
  RMDir  "$SMPROGRAMS\Inkscape"
  Delete "$SMPROGRAMS\Inkscape.lnk"
!endif
SectionEnd

Section $(lng_Core) SecCore
  SectionIn 1 2 3 RO
!ifndef DUMMYINSTALL
  DetailPrint "Installing Inkscape core files..."
  SetOutPath $INSTDIR
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  SetOverwrite on
  SetAutoClose false

  File /a "..\..\inkscape\ink*.exe"
  File /a "..\..\inkscape\AUTHORS"
  File /a "..\..\inkscape\COPYING"
  File /a "..\..\inkscape\COPYING.LIB"
  File /a "..\..\inkscape\NEWS"
  File /a "..\..\inkscape\gspawn-win32-helper.exe"
  File /a "..\..\inkscape\gspawn-win32-helper-console.exe"
  File /nonfatal /a "..\..\inkscape\HACKING.txt"
  File /a "..\..\inkscape\README"
  File /nonfatal /a "..\..\inkscape\README.txt"
  File /a "..\..\inkscape\TRANSLATORS"
  File /nonfatal /a /r "..\..\inkscape\data"
  File /nonfatal /a /r "..\..\inkscape\doc"
  File /nonfatal /a /r "..\..\inkscape\plugins"
  File /nonfatal /a /r /x *.??*.???* /x "examples" /x "tutorials" "..\..\inkscape\share"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  ; this files are added because it slips through the filter
  SetOutPath $INSTDIR\share\clipart
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /a "..\..\inkscape\share\clipart\inkscape.logo.svg"
  ;File /a "..\..\inkscape\share\clipart\inkscape.logo.classic.svg"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\share\icons
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /a "..\..\inkscape\share\icons\inkscape.file.png"
  File /a "..\..\inkscape\share\icons\inkscape.file.svg"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\modules
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r "..\..\inkscape\modules\*.*"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\python
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r "..\..\inkscape\python\*.*"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
!endif
SectionEnd

Section $(lng_GTKFiles) SecGTK
  SectionIn 1 2 3 RO
!ifndef DUMMYINSTALL
  DetailPrint "Installing GTK files..."
  SetOutPath $INSTDIR
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  SetOverwrite on
  File /a /r "..\..\inkscape\*.dll"
  File /a /r /x "locale" "..\..\inkscape\lib"
  File /a /r "..\..\inkscape\etc"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
!endif
SectionEnd

Section -SetCurrentUserOnly
!ifndef DUMMYINSTALL
  StrCpy $MultiUser 0
  SetShellVarContext current
!endif
SectionEnd

Section $(lng_Alluser) SecAlluser
  SectionIn 1 2 3
!ifndef DUMMYINSTALL
  ; disable this option in Win95/Win98/WinME
  StrCpy $MultiUser 1
  DetailPrint "Installing in administrator mode (registry root will be HKLM)"
  SetShellVarContext all
!endif
SectionEnd

SectionGroup $(lng_Shortcuts) SecShortcuts

Section $(lng_Desktop) SecDesktop
!ifndef DUMMYINSTALL
  CreateShortCut "$DESKTOP\Inkscape.lnk" "$INSTDIR\inkscape.exe"
!endif
SectionEnd

Section $(lng_Quicklaunch) SecQuicklaunch
!ifndef DUMMYINSTALL
  ${IfThen} $QUICKLAUNCH != $TEMP ${|} CreateShortCut "$QUICKLAUNCH\Inkscape.lnk" "$INSTDIR\inkscape.exe" ${|}
!endif
SectionEnd

Section $(lng_SVGWriter) SecSVGWriter
  SectionIn 1 2 3
!ifndef DUMMYINSTALL
  DetailPrint "Associating SVG files with Inkscape"
  ReadRegStr $0 HKCR ".svg" ""
  ${If} $0 == ""
    StrCpy $0 svgfile
    WriteRegStr HKCR ".svg" "" $0
    WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  ${EndIf}
  WriteRegStr HKCR "$0\shell\edit\command" "" `"$INSTDIR\Inkscape.exe" "%1"`

  ReadRegStr $0 HKCR ".svgz" ""
  ${If} $0 == ""
    StrCpy $0 svgfile
    WriteRegStr HKCR ".svgz" "" $0
    WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  ${EndIf}
  WriteRegStr HKCR "$0\shell\edit\command" "" `"$INSTDIR\Inkscape.exe" "%1"`
!endif
SectionEnd

Section $(lng_ContextMenu) SecContextMenu
  SectionIn 1 2 3
!ifndef DUMMYINSTALL
  DetailPrint "Adding Inkscape to SVG file context menu"
  ReadRegStr $0 HKCR ".svg" ""
  ${If} $0 == ""
    StrCpy $0 svgfile
    WriteRegStr HKCR ".svg" "" $0
    WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  ${EndIf}
  WriteRegStr HKCR "$0\shell\${PRODUCT_NAME}\command" "" `"$INSTDIR\Inkscape.exe" "%1"`

  ReadRegStr $0 HKCR ".svgz" ""
  ${If} $0 == ""
    StrCpy $0 svgfile
    WriteRegStr HKCR ".svgz" "" $0
    WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  ${EndIf}
  WriteRegStr HKCR "$0\shell\${PRODUCT_NAME}\command" "" `"$INSTDIR\Inkscape.exe" "%1"`
!endif
SectionEnd

SectionGroupEnd

Section /o $(lng_DeletePrefs) SecPrefs
!ifndef DUMMYINSTALL
  !insertmacro delprefs
!endif
SectionEnd

SectionGroup $(lng_Addfiles) SecAddfiles

Section $(lng_Examples) SecExamples
  SectionIn 1 2
!ifndef DUMMYINSTALL
  SetOutPath $INSTDIR\share
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r /x "*.??*.???*" "..\..\inkscape\share\examples"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
!endif
SectionEnd

Section $(lng_Tutorials) SecTutorials
  SectionIn 1 2
!ifndef DUMMYINSTALL
  SetOutPath $INSTDIR\share
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r /x "*.??*.???*" "..\..\inkscape\share\tutorials"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
!endif
SectionEnd

SectionGroupEnd

SectionGroup $(lng_Languages) SecLanguages
  !macro Language SecName lng
    Section /o $(lng_${lng}) Sec${SecName}
      ;SectionIn 1 2 3
!ifndef DUMMYINSTALL
      SetOutPath $INSTDIR
      !insertmacro UNINSTALL.LOG_OPEN_INSTALL
      File /nonfatal /a "..\..\inkscape\*.${lng}.txt" ; FIXME: remove this?  No such files.
      !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      SetOutPath $INSTDIR\locale
      !insertmacro UNINSTALL.LOG_OPEN_INSTALL
      File /nonfatal /a /r "..\..\inkscape\locale\${lng}"
      !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      SetOutPath $INSTDIR\lib\locale
      !insertmacro UNINSTALL.LOG_OPEN_INSTALL
      File /nonfatal /a /r "..\..\inkscape\lib\locale\${lng}"
      !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      SetOutPath $INSTDIR\share\clipart
      !insertmacro UNINSTALL.LOG_OPEN_INSTALL
      File /nonfatal /a /r "..\..\inkscape\share\clipart\*.${lng}.svg"
      !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      ; the keyboard tables
      SetOutPath $INSTDIR\share\screens
      !insertmacro UNINSTALL.LOG_OPEN_INSTALL
      File /nonfatal /a /r "..\..\inkscape\share\screens\*.${lng}.svg"
      !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      SetOutPath $INSTDIR\share\templates
      !insertmacro UNINSTALL.LOG_OPEN_INSTALL
      File /nonfatal /a /r "..\..\inkscape\share\templates\*.${lng}.svg"
      !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      SetOutPath $INSTDIR\doc
      !insertmacro UNINSTALL.LOG_OPEN_INSTALL
      File /nonfatal /a /r "..\..\inkscape\doc\keys.${lng}.xml"
      File /nonfatal /a /r "..\..\inkscape\doc\keys.${lng}.html"
      !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      SectionGetFlags ${SecTutorials} $R1
      IntOp $R1 $R1 & ${SF_SELECTED}
      ${If} $R1 >= ${SF_SELECTED}
        SetOutPath $INSTDIR\share\tutorials
        !insertmacro UNINSTALL.LOG_OPEN_INSTALL
        File /nonfatal /a "..\..\inkscape\share\tutorials\*.${lng}.*"
        !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      ${EndIf}
    !endif
    SectionEnd
  !macroend

  !insertmacro Language Amharic am
  !insertmacro Language Arabic ar
  !insertmacro Language Azerbaijani az
  !insertmacro Language Byelorussian be
  !insertmacro Language Bulgarian bg
  !insertmacro Language Bengali bn
  !insertmacro Language Breton br
  !insertmacro Language Catalan ca
  !insertmacro Language CatalanValencia ca@valencia
  !insertmacro Language Czech cs
  !insertmacro Language Danish da
  !insertmacro Language German de
  !insertmacro Language Dzongkha dz
  !insertmacro Language Greek el
  !insertmacro Language EnglishAustralian en_AU
  !insertmacro Language EnglishCanadian en_CA
  !insertmacro Language EnglishBritain en_GB
  !insertmacro Language EnglishPiglatin en_US@piglatin
  !insertmacro Language Esperanto eo
  !insertmacro Language Spanish es
  !insertmacro Language SpanishMexico es_MX
  !insertmacro Language Estonian et
  !insertmacro Language Basque eu
  !insertmacro Language French fr
  !insertmacro Language Finnish fi
  !insertmacro Language Irish ga
  !insertmacro Language Gallegan gl
  !insertmacro Language Hebrew he
  !insertmacro Language Croatian hr
  !insertmacro Language Hungarian hu
  !insertmacro Language Indonesian id
  !insertmacro Language Italian it
  !insertmacro Language Japanese ja
  !insertmacro Language Khmer km
  !insertmacro Language Korean ko
  !insertmacro Language Lithuanian lt
  !insertmacro Language Mongolian mn
  !insertmacro Language Macedonian mk
  !insertmacro Language NorwegianBokmal nb
  !insertmacro Language Nepali ne
  !insertmacro Language Dutch nl
  !insertmacro Language NorwegianNynorsk nn
  !insertmacro Language Panjabi pa
  !insertmacro Language Polish pl
  !insertmacro Language Portuguese pt
  !insertmacro Language PortugueseBrazil pt_BR
  !insertmacro Language Romanian ro
  !insertmacro Language Russian ru
  !insertmacro Language Kinyarwanda rw
  !insertmacro Language Slovak sk
  !insertmacro Language Slovenian sl
  !insertmacro Language Albanian sq
  !insertmacro Language Serbian sr
  !insertmacro Language SerbianLatin sr@latin
  !insertmacro Language Swedish sv
  !insertmacro Language Thai th
  !insertmacro Language Turkish tr
  !insertmacro Language Ukrainian uk
  !insertmacro Language Vietnamese vi
  !insertmacro Language ChineseSimplified zh_CN
  !insertmacro Language ChineseTaiwan zh_TW
SectionGroupEnd


Section -FinalizeInstallation
!ifndef DUMMYINSTALL
  DetailPrint "Finalizing installation"
  ${IfThen} $MultiUser  = 1 ${|} SetShellVarContext all ${|}
  ${IfThen} $MultiUser != 1 ${|} SetShellVarContext current ${|}

  WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\inkscape.exe"
  WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "MultiUser" $MultiUser
  WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "askMultiUser" $askMultiUser
  WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "User" $User

  ; start menu entries
  CreateShortcut "$SMPROGRAMS\Inkscape.lnk" "$INSTDIR\inkscape.exe"

  ; uninstall settings
  ; WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegExpandStr SHCTX "${PRODUCT_UNINST_KEY}" "UninstallString" "${UNINST_EXE}"
  WriteRegExpandStr SHCTX "${PRODUCT_UNINST_KEY}" "InstallDir" $INSTDIR
  WriteRegExpandStr SHCTX "${PRODUCT_UNINST_KEY}" "InstallLocation" $INSTDIR
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Inkscape.exe,0"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoRepair" 1

  ;create/update log always within .onInstSuccess function
  !insertmacro UNINSTALL.LOG_UPDATE_INSTALL

  DetailPrint "Creating MD5 checksums"
  ClearErrors
  FileOpen $0 $INSTDIR\Uninstall.dat r
  FileOpen $9 $INSTDIR\Uninstall.log w
  ${IfNot} ${Errors}
    ${Do}
      ClearErrors
      FileRead $0 $1
      ${IfThen} ${Errors} ${|} ${ExitDo} ${|}
      StrCpy $1 $1 -2
      md5dll::GetMD5File /NOUNLOAD $1
      Pop $2
      ${IfThen} $2 != "" ${|} FileWrite $9 "$2  $1$\r$\n" ${|}
    ${Loop}
  ${EndIf}
  FileClose $0
  FileClose $9
  ; Not needed any more
  Delete $INSTDIR\Uninstall.dat
!endif
SectionEnd

; Last the Descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} $(lng_CoreDesc)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecGTK} $(lng_GTKFilesDesc)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts} $(lng_ShortcutsDesc)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecAlluser} $(lng_AlluserDesc)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} $(lng_DesktopDesc)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecQuicklaunch} $(lng_QuicklaunchDesc)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSVGWriter} $(lng_SVGWriterDesc)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecContextMenu} $(lng_ContextMenuDesc)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPrefs} $(lng_DeletePrefsDesc)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecAddfiles} $(lng_AddfilesDesc)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecExamples} $(lng_ExamplesDesc)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTutorials} $(lng_TutorialsDesc)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecLanguages} $(lng_LanguagesDesc)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

!macro Parameter key Section
  ${GetOptions} $CMDARGS "/${key}=" $1
  ${If} $1 == "OFF"
    SectionGetFlags ${Section} $0
    IntOp $2 ${SF_SELECTED} ~
    IntOp $0 $0 & $2
    SectionSetFlags ${Section} $0
  ${EndIf}
  ${If} $1 == "ON"
    SectionGetFlags ${Section} $0
    IntOp $0 $0 | ${SF_SELECTED}
    SectionSetFlags ${Section} $0
  ${EndIf}
!macroend

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY

  !macro LanguageAutoSelect SecName LocaleID
    ${If} $LANGUAGE = ${LocaleID}
      SectionGetFlags ${Sec${SecName}} $0
      IntOp $0 $0 | ${SF_SELECTED}
      SectionSetFlags ${Sec${SecName}} $0
    ${EndIf}
  !macroend

  ;!insertmacro LanguageAutoSelect English 1033
  !insertmacro LanguageAutoSelect Breton 1150
  !insertmacro LanguageAutoSelect Catalan 1027
  !insertmacro LanguageAutoSelect Czech 1029
  !insertmacro LanguageAutoSelect Finnish 1035
  !insertmacro LanguageAutoSelect French 1036
  !insertmacro LanguageAutoSelect Gallegan 1110 ; Galician, but section is called Gallegan
  !insertmacro LanguageAutoSelect German 1031
  !insertmacro LanguageAutoSelect Italian 1040
  !insertmacro LanguageAutoSelect Japanese 1041
  !insertmacro LanguageAutoSelect Polish 1045
  !insertmacro LanguageAutoSelect Russian 1049
  !insertmacro LanguageAutoSelect Slovak 1051
  !insertmacro LanguageAutoSelect Slovenian 1060
  !insertmacro LanguageAutoSelect Spanish 1034
  !insertmacro LanguageAutoSelect ChineseTaiwan 1028 ; TradChinese, but section is called ChineseTaiwan

  ${GetParameters} $CMDARGS
  ;prepare log always within .onInit function
  !insertmacro UNINSTALL.LOG_PREPARE_INSTALL

  ;Extract InstallOptions INI files
  StrCpy $AskMultiUser "1"
  StrCpy $MultiUser "0"
  ; this resets AskMultiUser if Win95/98/ME
  ClearErrors
  ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
  ${If} ${Errors}
    ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion" VersionNumber
    StrCpy $R0 $R0 1
    ${IfThen} $R0 = 4 ${|} StrCpy $AskMultiUser 0 ${|}
  ${EndIf}

  ; hide all user section if ME/9x
  ${IfThen} $AskMultiUser != 1 ${|} SectionSetText ${SecAlluser} "" ${|}

  ; hide if quick launch if not available
  ${IfThen} $QUICKLAUNCH == $TEMP ${|} SectionSetText ${SecQuicklaunch} "" ${|}

  ;check if user is admin
  ClearErrors
  UserInfo::GetName
  ${If} ${Errors}
    ; This one means you don't need to care about admin or
    ; not admin because Windows 9x doesn't either
    ${IfCmd} MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(lng_NOT_SUPPORTED)$(lng_OK_CANCEL_DESC)" /SD IDOK IDCANCEL ${||} Quit ${|}
  ${Else}
    Pop $User
    UserInfo::GetAccountType
    Pop $1
    ${If} $1 != "Admin"
    ${AndIf} ${Cmd} ${|} MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(lng_NO_ADMIN)$(lng_OK_CANCEL_DESC)" /SD IDOK IDCANCEL ${|}
      Quit
    ${EndIf}
  ${EndIf}

  ;check for previous installation
  ReadRegStr $0 HKLM "${PRODUCT_DIR_REGKEY}" "User"
  ${IfThen} $0 == "" ${|} ReadRegStr $0 HKCU "${PRODUCT_DIR_REGKEY}" "User" ${|}
  ;check user if applicable
  ${If} $0 != ""
  ${AndIf} $0 != $User
  ${AndIf} ${Cmd} ${|} MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(lng_DIFFERENT_USER)$(lng_OK_CANCEL_DESC)" /SD IDOK IDCANCEL ${|}
    Quit
  ${EndIf}

  ; call uninstall first
  ; code taken from the vlc project
    ReadRegStr $R0  HKLM ${PRODUCT_UNINST_KEY} "UninstallString"
    ReadRegStr $R1  HKLM ${PRODUCT_UNINST_KEY} "DisplayName"
    ${If} $R0 == ""
      ReadRegStr $R0  HKCU ${PRODUCT_UNINST_KEY} "UninstallString"
      ReadRegStr $R1  HKCU ${PRODUCT_UNINST_KEY} "DisplayName"
    ${EndIf}
    ${If} $R0 != ""
    ${AndIf} ${Cmd} ${|} MessageBox MB_YESNO|MB_ICONEXCLAMATION $(lng_WANT_UNINSTALL_BEFORE) /SD IDNO IDYES ${|}
      ExecWait $R0 ;Was '$R0 _?=$INSTDIR' but we do NOT want it leaving the uninstaller behind.
    ${EndIf}

  ; proccess command line parameter
  !insertmacro Parameter "GTK" ${SecGTK}
  !insertmacro Parameter "SHORTCUTS" ${secShortcuts}
  !insertmacro Parameter "ALLUSER" ${SecAlluser}
  !insertmacro Parameter "DESKTOP" ${SecDesktop}
  !insertmacro Parameter "QUICKLAUNCH" ${SecQUICKlaunch}
  !insertmacro Parameter "SVGEDITOR" ${SecSVGWriter}
  !insertmacro Parameter "CONTEXTMENUE" ${SecContextMenu}
  !insertmacro Parameter "PREFERENCES" ${SecPrefs}
  !insertmacro Parameter "ADDFILES" ${SecAddfiles}
  !insertmacro Parameter "EXAMPLES" ${SecExamples}
  !insertmacro Parameter "TUTORIALS" ${SecTutorials}
  !insertmacro Parameter "LANGUAGES" ${SecLanguages}
  !insertmacro Parameter "am" ${SecAmharic}
  !insertmacro Parameter "ar" ${SecArabic}
  !insertmacro Parameter "az" ${SecAzerbaijani}
  !insertmacro Parameter "be" ${SecByelorussian}
  !insertmacro Parameter "bg" ${SecBulgarian}
  !insertmacro Parameter "bn" ${SecBengali}
  !insertmacro Parameter "br" ${SecBreton}
  !insertmacro Parameter "ca" ${SecCatalan}
  !insertmacro Parameter "ca@valencia" ${SecCatalanValencia}
  !insertmacro Parameter "cs" ${SecCzech}
  !insertmacro Parameter "da" ${SecDanish}
  !insertmacro Parameter "de" ${SecGerman}
  !insertmacro Parameter "dz" ${SecDzongkha}
  !insertmacro Parameter "el" ${SecGreek}
  !insertmacro Parameter "en_AU" ${SecEnglishAustralian}
  !insertmacro Parameter "en_CA" ${SecEnglishCanadian}
  !insertmacro Parameter "en_GB" ${SecEnglishBritain}
  !insertmacro Parameter "en_US@piglatin" ${SecEnglishPiglatin}
  !insertmacro Parameter "eo" ${SecEsperanto}
  !insertmacro Parameter "es" ${SecSpanish}
  !insertmacro Parameter "es_MX" ${SecSpanishMexico}
  !insertmacro Parameter "et" ${SecEstonian}
  !insertmacro Parameter "eu" ${SecBasque}
  !insertmacro Parameter "fi" ${SecFinnish}
  !insertmacro Parameter "fr" ${SecFrench}
  !insertmacro Parameter "ga" ${SecIrish}
  !insertmacro Parameter "gl" ${SecGallegan}
  !insertmacro Parameter "he" ${SecHebrew}
  !insertmacro Parameter "hr" ${SecCroatian}
  !insertmacro Parameter "hu" ${SecHungarian}
  !insertmacro Parameter "id" ${SecIndonesian}
  !insertmacro Parameter "it" ${SecItalian}
  !insertmacro Parameter "ja" ${SecJapanese}
  !insertmacro Parameter "km" ${SecKhmer}
  !insertmacro Parameter "ko" ${SecKorean}
  !insertmacro Parameter "lt" ${SecLithuanian}
  !insertmacro Parameter "mk" ${SecMacedonian}
  !insertmacro Parameter "mn" ${SecMongolian}
  !insertmacro Parameter "nb" ${SecNorwegianBokmal}
  !insertmacro Parameter "ne" ${SecNepali}
  !insertmacro Parameter "nl" ${SecDutch}
  !insertmacro Parameter "nn" ${SecNorwegianNynorsk}
  !insertmacro Parameter "pa" ${SecPanjabi}
  !insertmacro Parameter "pl" ${SecPolish}
  !insertmacro Parameter "pt" ${SecPortuguese}
  !insertmacro Parameter "pt_BR" ${SecPortugueseBrazil}
  !insertmacro Parameter "ro" ${SecRomanian}
  !insertmacro Parameter "ru" ${SecRussian}
  !insertmacro Parameter "rw" ${SecKinyarwanda}
  !insertmacro Parameter "sk" ${SecSlovak}
  !insertmacro Parameter "sl" ${SecSlovenian}
  !insertmacro Parameter "sq" ${SecAlbanian}
  !insertmacro Parameter "sr" ${SecSerbian}
  !insertmacro Parameter "sr@latin" ${SecSerbianLatin}
  !insertmacro Parameter "sv" ${SecSwedish}
  !insertmacro Parameter "th" ${SecThai}
  !insertmacro Parameter "tr" ${SecTurkish}
  !insertmacro Parameter "uk" ${SecUkrainian}
  !insertmacro Parameter "vi" ${SecVietnamese}
  !insertmacro Parameter "zh_CN" ${SecChineseSimplified}
  !insertmacro Parameter "zh_TW" ${SecChineseTaiwan}

  ClearErrors
  ${GetOptions} $CMDARGS "/?" $1
  ${IfNot} ${Errors}
    MessageBox MB_OK "Possible parameters for installer:$\r$\n \
      /?: this help screen$\r$\n \
      /S: silent$\r$\n \
      /D=(directory): where to install Inkscape$\r$\n \
      /GTK=(OFF/ON): GTK+ Runtime environment$\r$\n \
      /SHORTCUTS=(OFF/ON): shortcuts to start Inkscape$\r$\n \
      /ALLUSER=(OFF/ON): for all users on the computer$\r$\n \
      /DESKTOP=(OFF/ON): Desktop icon$\r$\n \
      /QUICKLAUNCH=(OFF/ON): quick launch icon$\r$\n \
      /SVGEDITOR=(OFF/ON): default SVG editor$\r$\n \
      /CONTEXTMENUE=(OFF/ON): context menue integration$\r$\n \
      /PREFERENCES=(OFF/ON): delete users preference files$\r$\n \
      /ADDFILES=(OFF/ON): additional files$\r$\n \
      /EXAMPLES=(OFF/ON): examples$\r$\n \
      /TUTORIALS=(OFF/ON): tutorials$\r$\n \
      /LANGUAGES=(OFF/ON): translated menues, examples, etc.$\r$\n \
      /[locale code]=(OFF/ON): e.g am, es, es_MX as in Inkscape supported"
    Abort
  ${EndIf}
FunctionEnd

; --------------------------------------------------

Function un.CustomPageUninstall
  !insertmacro MUI_HEADER_TEXT "$(lng_UInstOpt)" "$(lng_UInstOpt1)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.uninstall" "Field 1" "Text" "$APPDATA\Inkscape\"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.uninstall" "Field 2" "Text" "$(lng_PurgePrefs)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "inkscape.nsi.uninstall"
  !insertmacro MUI_INSTALLOPTIONS_READ $MultiUser "inkscape.nsi.uninstall" "Field 2" "State"
FunctionEnd

Function un.onInit
  ;begin uninstall, could be added on top of uninstall section instead
  ;!insertmacro UNINSTALL.LOG_BEGIN_UNINSTALL
  ${IfNot} ${FileExists} $INSTDIR\uninstall.log
    MessageBox MB_OK|MB_ICONEXCLAMATION "$(lng_UninstallLogNotFound)" /SD IDOK
    Quit
  ${EndIf}
  ClearErrors
  StrCpy $User ""
  UserInfo::GetName
  ${IfNot} ${Errors}
    Pop $0
    StrCpy $User $0
  ${EndIf}
  StrCpy $askMultiUser 1
  StrCpy $MultiUser 1

  ; Test if this was a multiuser installation
  ReadRegStr $0 HKLM "${PRODUCT_DIR_REGKEY}" ""
  ${If} $0 == "$INSTDIR\inkscape.exe"
    ReadRegStr $MultiUser HKLM "${PRODUCT_DIR_REGKEY}" "MultiUser"
    ReadRegStr $askMultiUser HKLM "${PRODUCT_DIR_REGKEY}" "askMultiUser"
    ReadRegStr $0 HKLM "${PRODUCT_DIR_REGKEY}" "User"
  ${Else}
    ReadRegStr $MultiUser HKCU "${PRODUCT_DIR_REGKEY}" "MultiUser"
    ReadRegStr $askMultiUser HKCU "${PRODUCT_DIR_REGKEY}" "askMultiUser"
    ReadRegStr $0 HKCU "${PRODUCT_DIR_REGKEY}" "User"
  ${EndIf}
  ;check user if applicable
  ${If} $0 != ""
  ${AndIf} $0 != $User
  ${AndIf} ${Cmd} ${|} MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(lng_DIFFERENT_USER)$(lng_OK_CANCEL_DESC)" /SD IDOK IDCANCEL ${|}
    Quit
  ${EndIf}

  !insertmacro MUI_INSTALLOPTIONS_EXTRACT "inkscape.nsi.uninstall"

  SetShellVarContext all
  ${IfThen} $MultiUser = 0 ${|} SetShellVarContext current ${|}
FunctionEnd

Section Uninstall
!ifndef DUMMYINSTALL
  ; remove personal settings
  Delete "$APPDATA\Inkscape\extension-errors.log"
  ${If} $MultiUser = 0
    DetailPrint "Purging personal settings in $APPDATA\Inkscape"
    ;RMDir /r "$APPDATA\Inkscape"
    !insertmacro delprefs
  ${EndIf}

  ; Remove file associations for svg editor
  StrCpy $3 "svg"
  ${For} $2 0 1
    ${IfThen} $2 = 1 ${|} StrCpy $3 $3z ${|}
    DetailPrint "Removing file associations for $3 editor"
    ClearErrors
    ReadRegStr $0 HKCR ".$3" ""
    ${IfNot} ${Errors}
      ReadRegStr $1 HKCR "$0\shell\edit\command" ""
      ${If} $1 == `"$INSTDIR\Inkscape.exe" "%1"`
        DeleteRegKey HKCR "$0\shell\edit\command"
      ${EndIf}

      ClearErrors
      ReadRegStr $1 HKCR "$0\shell\open\command" ""
      ${If} $1 == `"$INSTDIR\Inkscape.exe" "%1"`
        DeleteRegKey HKCR "$0\shell\open\command"
      ${EndIf}

      DeleteRegKey HKCR "$0\shell\${PRODUCT_NAME}"
      DeleteRegKey /ifempty HKCR "$0\shell\edit"
      DeleteRegKey /ifempty HKCR "$0\shell\open"
      DeleteRegKey /ifempty HKCR "$0\shell"
      DeleteRegKey /ifempty HKCR "$0"
      DeleteRegKey /ifempty HKCR ".$3"
    ${EndIf}
  ${Next}

  SetShellVarContext all
  DeleteRegKey SHCTX "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey SHCTX "${PRODUCT_UNINST_KEY}"
  Delete "$DESKTOP\Inkscape.lnk"
  Delete "$QUICKLAUNCH\Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape.lnk"
  ;just in case they are still there
  Delete "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape\Inkscape.lnk"
  RMDir  "$SMPROGRAMS\Inkscape"

  SetShellVarContext current
  DeleteRegKey SHCTX "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey SHCTX "${PRODUCT_UNINST_KEY}"
  Delete "$DESKTOP\Inkscape.lnk"
  Delete "$QUICKLAUNCH\Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape.lnk"
  ;just in case they are still there
  Delete "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape\Inkscape.lnk"
  RMDir  "$SMPROGRAMS\Inkscape"

  InitPluginsDir
  SetPluginUnload manual

  ClearErrors
  FileOpen $0 $INSTDIR\uninstall.log r
  ${If} ${Errors} ;else uninstallnotfound
    MessageBox MB_OK|MB_ICONEXCLAMATION "$(lng_UninstallLogNotFound)" /SD IDOK
  ${Else}
    ${Do}
      ClearErrors
      FileRead $0 $1
      ${IfThen} ${Errors} ${|} ${ExitDo} ${|}
      ; cat the line into md5 and filename
      StrLen $2 $1
      ${IfThen} $2 <= 35 ${|} ${Continue} ${|}
      StrCpy $3 $1 32
      StrCpy $filename $1 $2-36 34 ;remove trailing CR/LF
      StrCpy $filename $filename -2
      ; $3 = MD5 when installed, then deletion choice
      ; $filename = file
      ; $5 = MD5 now
      ; $6 = "always"/"never" remove files touched by user

      ${If} ${FileExists} $filename
        ${If} $6 == "always"
          StrCpy $3 2
        ${Else}
          md5dll::GetMD5File /NOUNLOAD $filename
          Pop $5 ;md5 of file
          ${If} $3 != $5
          ${AndIf} $6 != "never"
            ; the md5 sums does not match so we ask
            messagebox::show MB_DEFBUTTON3|MB_TOPMOST "" "0,103" \
              "$(lng_FileChanged)" "$(lng_Yes)" "$(lng_AlwaysYes)" "$(lng_No)" "$(lng_AlwaysNo)"
            Pop $3
            ${IfThen} $3 = 2 ${|} StrCpy $6 "always" ${|}
            ${IfThen} $3 = 4 ${|} StrCpy $6 "never" ${|}
          ${EndIf}
        ${EndIf}

        ${If}   $3 = 1 ; yes
        ${OrIf} $3 = 2 ; always
          ; Remove File
          ClearErrors
          Delete $filename
          ;now recursivly remove the path
          ${Do}
            ClearErrors
            ${un.GetParent} $filename $filename
            ${IfThen} ${Errors} ${|} ${ExitDo} ${|}
            RMDir $filename
            ${IfThen} ${Errors} ${|} ${ExitDo} ${|}
          ${Loop}
        ${EndIf}
      ${EndIf}
    ${Loop}
  ${EndIf}
  FileClose $0
  Delete "$INSTDIR\uninstall.log"
  Delete "$INSTDIR\uninstall.exe"
  ; remove empty directories
  RMDir "$INSTDIR\data"
  RMDir "$INSTDIR\doc"
  RMDir "$INSTDIR\modules"
  RMDir "$INSTDIR\plugins"
  RMDir $INSTDIR
  SetAutoClose false
!endif
SectionEnd

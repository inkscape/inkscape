; Instructions for compilers
; ==========================
; This file generates the Inkscape installer, which is currently the
; preferred deployment method on Windows.
; 1. Install NSIS 3.0 or later (http://nsis.sourceforge.net/)
; 2. Compile Inkscape (http://wiki.inkscape.org/wiki/index.php/Win32Port)
; 3. Compile this file with NSIS.
;
;    There should be no need to set version  numbers in this file as it
;    gets them from the Bazaar branch info and inkscape.rc or
;    inkscape-version.cpp respectively. However, if the version number comes
;    out wrong or this script didn't compile properly then you can define
;    INKSCAPE_VERSION by uncommenting the next line and setting the correct
;    value:
;      !define INKSCAPE_VERSION "0.48"
;    If you ever need to do a second, third or Nth release of the build or
;    of the installer, then change the RELEASE_REVISION value below:
       !define RELEASE_REVISION 1

; There should never be any need for packagers to touch anything below
; this line. Otherwise file a bug or write to the mailing list.


; Define this to make it build quickly, not including any of the files or code in the sections,
; for quick testing of features of the installer and development thereof.
;!define DUMMYINSTALL


; Installer code {{{1
; Unicode, compression and admin requirement {{{2
Unicode true
SetCompressor /SOLID lzma
SetCompressorDictSize 32
RequestExecutionLevel admin

; Include required files {{{2
!include LogicLib.nsh
!include Sections.nsh
!include macros\ifexist.nsh
!include macros\RequireLatestNSIS.nsh
!include macros\SHMessageBoxCheck.nsh
!include macros\VersionCompleteXXXX.nsh
!include languages\_language_lists.nsh

; Advanced Uninstall Log {{{3
; We're abusing this script terribly and it's time to fix the broken uninstaller.
; However, for the moment, this is what we're using.
!define INSTDIR_REG_ROOT HKLM
!define INSTDIR_REG_KEY "${UNINST_KEY}"
!include macros\AdvUninstLog.nsh
;!insertmacro INTERACTIVE_UNINSTALL ; not needed anymore since we have our own uninstall logic; conflicts with other macros

; Initialise NSIS plug-ins {{{3
; The plugin used is md5dll
!addplugindir plugins

; FileFunc bits and pieces {{{3
!include FileFunc.nsh
!insertmacro GetParameters
!insertmacro GetSize
!insertmacro GetOptions
!insertmacro Locate
!insertmacro un.GetParent

; User interface {{{3
!include MUI.nsh
; MUI Configuration {{{4
!define MUI_ABORTWARNING
!define MUI_ICON ..\..\inkscape.ico
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP header.bmp
!define MUI_WELCOMEFINISHPAGE_BITMAP welcomefinish.bmp
!define MUI_UNWELCOMEFINISHPAGE_BITMAP welcomefinish.bmp
!define MUI_COMPONENTSPAGE_SMALLDESC

; Pages {{{4
; Installer pages {{{5
; Welcome page {{{6
!insertmacro MUI_PAGE_WELCOME
; License page {{{6
LicenseForceSelection off
;!define MUI_LICENSEPAGE_RADIOBUTTONS
!define MUI_LICENSEPAGE_BUTTON "$(^NextBtn)"
!define MUI_LICENSEPAGE_TEXT_BOTTOM "$(LICENSE_BOTTOM_TEXT)"
!insertmacro MUI_PAGE_LICENSE ..\..\Copying
; Components page {{{6
!insertmacro MUI_PAGE_COMPONENTS
InstType "$(Full)"
InstType "$(Optimal)"
InstType "$(Minimal)"
;Directory page {{{6
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page {{{6
!insertmacro MUI_PAGE_INSTFILES
; Finish page {{{6
!define MUI_FINISHPAGE_RUN "$INSTDIR\inkscape.exe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages {{{5
!insertmacro MUI_UNPAGE_CONFIRM
UninstPage custom un.CustomPageUninstall
!insertmacro MUI_UNPAGE_INSTFILES
ShowUninstDetails hide
!insertmacro MUI_UNPAGE_FINISH

; Localization {{{3
!define LANGFILE_LANGDLL_FMT "%ENGNAME% / %NATIVENAME%" ; include English name in language selection dialog
; See also the "Languages sections" SectionGroup lower down.
!insertmacro MUI_RESERVEFILE_LANGDLL
;TODO: check if `!insertmacro LANGFILE "English" "English"`-style lines are needed (don't think it should be due to MUI_LANGUAGE)
!echo `Loading language files...`
!verbose push
!verbose 3
!insertmacro MUI_LANGUAGE "English"
!insertmacro LANGFILE_INCLUDE "languages\English.nsh"
!macro INKLANGFILE  LocaleName LocaleID
  !insertmacro MUI_LANGUAGE "${LocaleName}"
  !insertmacro LANGFILE_INCLUDE_WITHDEFAULT "languages\${LocaleName}.nsh" "languages\English.nsh"
!macroend
; include list of available installer translations from /languages/_language_lists.nsh
!insertmacro INSTALLER_TRANSLATIONS INKLANGFILE
!verbose pop

ReserveFile inkscape.nsi.uninstall
ReserveFile /plugin UserInfo.dll
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; #######################################
; SETTINGS
; #######################################

; Find inkscape distribution directory (uncomment line below to manually define)
;!define INKSCAPE_DIST_DIR ..\..\inkscape
!ifdef INKSCAPE_DIST_DIR
  ${!ifnexist} ${INKSCAPE_DIST_DIR}\inkscape.exe
    !error "inkscape.exe not found in INKSCAPE_DIST_DIR ('${INKSCAPE_DIST_DIR}')"
!endif
!ifndef INKSCAPE_DIST_DIR
  ${!defineifexist} ${INKSCAPE_DIST_DIR}\inkscape.exe INKSCAPE_DIST_DIR ..\..\inkscape ; btool default
!endif
!ifndef INKSCAPE_DIST_DIR
  ${!defineifexist} ..\..\build\inkscape\inkscape.exe INKSCAPE_DIST_DIR ..\..\build\inkscape ; cmake default
!endif
!ifndef INKSCAPE_DIST_DIR
  !error "Couldn't find inkscape distribution directory in neither ..\..\inkscape nor ..\..\build\inkscape"
!endif
!echo `Bundling compiled Inkscape files from ${INKSCAPE_DIST_DIR}`

; Product details (version, name, registry keys etc.) {{{2
; Try to find version number in inkscape.rc first (e.g. 0.92pre1) {{{3
!ifndef INKSCAPE_VERSION
  !searchparse /noerrors /file ..\..\src\inkscape.rc `VALUE "ProductVersion", "` INKSCAPE_VERSION `"`
  !ifdef INKSCAPE_VERSION
    !echo `Got version number from ..\..\src\inkscape.rc: ${INKSCAPE_VERSION}`
  !endif
!endif
; Find the version number in inkscape-version.cpp (e.g. 0.47+devel) {{{3
!ifndef INKSCAPE_VERSION
  ; Official release format (no newlines)
  !searchparse /noerrors /file ..\..\src\inkscape-version.cpp `namespace Inkscape {  char const *version_string = "` INKSCAPE_VERSION ` r` BZR_REVISION `";  }`
  !ifndef INKSCAPE_VERSION
    ; Other format; sorry, it has to be done in two steps.
    !searchparse /noerrors /file ..\..\src\inkscape-version.cpp `char const *version_string = "` INKSCAPE_VERSION `";`
    !searchparse /noerrors `${INKSCAPE_VERSION}` `` INKSCAPE_VERSION ` r` BZR_REVISION
  !endif
  !ifdef INKSCAPE_VERSION
    !echo `Got version number from ..\..\src\inkscape-version.cpp: ${INKSCAPE_VERSION}`
  !endif
!endif
!ifndef INKSCAPE_VERSION
  !error "INKSCAPE_VERSION not defined and unable to get version number from either ..\..\src\inkscape.rc or ..\..\src\inkscape-version.cpp!"
!endif
!define FILENAME Inkscape-${INKSCAPE_VERSION}
!define BrandingText `Inkscape ${INKSCAPE_VERSION}`

; Detect architecture of the build
${!ifexist} ${INKSCAPE_DIST_DIR}\gspawn-win32-helper.exe
  !define BITNESS 32
!endif
${!ifexist} ${INKSCAPE_DIST_DIR}\gspawn-win64-helper.exe
  !define BITNESS 64
  !define /redef FILENAME `${FILENAME}-x64` ; add architecture to filename for 64-bit builds
!endif
!ifndef BITNESS
  !error "Could not detect architecture (BITNESS) of the Inkscape build"
!endif

; Check for the Bazaar revision number for lp:inkscape {{{3
${!ifexist} ..\..\.bzr\branch\last-revision
  !if `${BZR_REVISION}` == ``
    !undef BZR_REVISION
  !endif
  !ifndef BZR_REVISION
    !searchparse /noerrors /file ..\..\.bzr\branch\last-revision "" BZR_REVISION " "
  !endif
!endif

; Check for devel builds and clear up bzr revision number define {{{3
!searchparse /noerrors ${INKSCAPE_VERSION} "" INKSCAPE_VERSION_NUMBER "+devel"
!if ${INKSCAPE_VERSION_NUMBER} != ${INKSCAPE_VERSION}
  !define DEVEL
!endif
!if `${BZR_REVISION}` == ``
  !undef BZR_REVISION
!endif
; For releases like 0.48pre1, throw away the preN. It's too tricky to deal with
; it properly so I'll leave it alone. It's just a pre-release, so it doesn't
; really matter. So long as the final release works properly.
!ifndef DEVEL
  !undef INKSCAPE_VERSION_NUMBER
  !searchparse /noerrors ${INKSCAPE_VERSION} "" INKSCAPE_VERSION_NUMBER "pre" PRE_NUMBER
!endif

; Handle display version number and complete X.X version numbers into X.X.X.X {{{3
!ifdef DEVEL & BZR_REVISION
  !define /redef FILENAME `${FILENAME}-r${BZR_REVISION}`
  !define /redef BrandingText `${BrandingText} r${BZR_REVISION}`
  !define VERSION_X.X.X.X_REVISION ${BZR_REVISION}
; Handle the installer revision number {{{4
!else ifdef RELEASE_REVISION
  !define /redef FILENAME `${FILENAME}-${RELEASE_REVISION}`
  ; If we wanted the branding text to be like "Inkscape 0.48pre1 r9505" this'd do it.
  ;!ifdef BZR_REVISION
  ;  !define /redef BrandingText `${BrandingText} r${BZR_REVISION}`
  ;!endif
  !if `${RELEASE_REVISION}` != `1`
    !define /redef BrandingText `${BrandingText}, revision ${RELEASE_REVISION}`
  !endif
  !define VERSION_X.X.X.X_REVISION ${RELEASE_REVISION}
!else
  !define VERSION_X.X.X.X_REVISION 0
!endif

${VersionCompleteXXXRevision} ${INKSCAPE_VERSION_NUMBER} VERSION_X.X.X.X ${VERSION_X.X.X.X_REVISION}

; Product definitions {{{3
!define PRODUCT_NAME "Inkscape" ; TODO: fix up the language files to not use this and kill this line
!define INSTDIR_KEY "Software\Microsoft\Windows\CurrentVersion\App Paths\inkscape.exe"
!define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\Inkscape"
!define _FILENAME ${FILENAME}.exe
!undef FILENAME
!define FILENAME ${_FILENAME}
!undef _FILENAME

; Product information {{{3
Name              `Inkscape`
Caption           `Inkscape - $(CaptionDescription)`
BrandingText      `${BrandingText}`
OutFile           `${FILENAME}`
!if ${BITNESS} = 32
  InstallDir        "$PROGRAMFILES32\Inkscape"
!else
  InstallDir        "$PROGRAMFILES64\Inkscape"
!endif
InstallDirRegKey  HKLM "${INSTDIR_KEY}" ""

; Version information {{{3
VIProductVersion ${VERSION_X.X.X.X}
VIAddVersionKey /LANG=0 ProductName "Inkscape"
VIAddVersionKey /LANG=0 Comments "Licensed under the GNU GPL"
VIAddVersionKey /LANG=0 CompanyName "Inkscape Project"
VIAddVersionKey /LANG=0 LegalCopyright "© 2016 Inkscape Project"
VIAddVersionKey /LANG=0 FileDescription "Inkscape Vector Graphics Editor"
VIAddVersionKey /LANG=0 FileVersion ${VERSION_X.X.X.X}
VIAddVersionKey /LANG=0 ProductVersion ${VERSION_X.X.X.X}

; Variables {{{2
Var askMultiUser
Var filename
Var MultiUser
Var User
Var CMDARGS

!macro delprefs ; Delete preferences (originally from VLC) {{{2
  StrCpy $0 0
  DetailPrint "Deleting personal preferences..."
  DetailPrint "Finding all users..."
  ${Do}
  ; this will loop through all the logged users and "virtual" windows users
  ; (it looks like users are only present in HKEY_USERS when they are logged in)
    ClearErrors
    EnumRegKey $1 HKU "" $0
    ${IfThen} $1 == "" ${|} ${ExitDo} ${|}
    IntOp $0 $0 + 1
    ReadRegStr $2 HKU "$1\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" AppData
    ${IfThen} $2 == "" ${|} ${Continue} ${|}
    DetailPrint "Removing $2\Inkscape"

    Delete $2\Inkscape\preferences.xml
    Delete $2\Inkscape\extension-errors.log

    RMDir  $2\Inkscape\extensions
    RMDir  $2\Inkscape\icons
    RMDir  $2\Inkscape\keys
    RMDir  $2\Inkscape\palettes
    RMDir  $2\Inkscape\templates
    RMDir  $2\Inkscape
  ${Loop}
!macroend

; Sections (these do the work) {{{2

Section -removeInkscape ; Hidden, mandatory section to clean a previous installation {{{
!ifndef DUMMYINSTALL
  ;remove the old Inkscape shortcuts from the startmenu
  ;just in case they are still there
  SetShellVarContext current
  Delete "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk"
  Delete  $SMPROGRAMS\Inkscape\Inkscape.lnk
  RMDir   $SMPROGRAMS\Inkscape
  Delete  $SMPROGRAMS\Inkscape.lnk
  SetShellVarContext all
  Delete "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk"
  Delete  $SMPROGRAMS\Inkscape\Inkscape.lnk
  RMDir   $SMPROGRAMS\Inkscape
  Delete  $SMPROGRAMS\Inkscape.lnk
!endif
SectionEnd ; -removeInkscape }}}

Section "$(Core)" SecCore ; Mandatory Inkscape core files section {{{
  SectionIn 1 2 3 RO
!ifndef DUMMYINSTALL
  DetailPrint "Installing Inkscape core files..."

  SetOutPath $INSTDIR
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File           /a    ${INKSCAPE_DIST_DIR}\ink*.exe
  File /nonfatal /a    ${INKSCAPE_DIST_DIR}\inkscape.com ; not created as of Inkscape 0.92pre1
  File           /a    ${INKSCAPE_DIST_DIR}\AUTHORS
  File           /a    ${INKSCAPE_DIST_DIR}\COPYING
  File           /a    ${INKSCAPE_DIST_DIR}\GPL2.txt
  File           /a    ${INKSCAPE_DIST_DIR}\GPL3.txt
  File           /a    ${INKSCAPE_DIST_DIR}\LGPL2.1.txt
  File           /a    ${INKSCAPE_DIST_DIR}\NEWS
  File           /a    ${INKSCAPE_DIST_DIR}\gspawn-win${BITNESS}-helper.exe
  File           /a    ${INKSCAPE_DIST_DIR}\gspawn-win${BITNESS}-helper-console.exe
  File           /a    ${INKSCAPE_DIST_DIR}\README
  File           /a    ${INKSCAPE_DIST_DIR}\TRANSLATORS
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL

  SetOutPath $INSTDIR\data
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r ${INKSCAPE_DIST_DIR}\data\*.*
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\doc
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r ${INKSCAPE_DIST_DIR}\doc\*.*
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\plugins
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r ${INKSCAPE_DIST_DIR}\plugins\*.*
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\modules
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r ${INKSCAPE_DIST_DIR}\modules\*.*
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL

  ;exclude everything from /share for which we have separate sections below
  SetOutPath $INSTDIR\share
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r /x *.??*.???* /x examples /x extensions /x locale /x tutorials ${INKSCAPE_DIST_DIR}\share\*.*
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  ; this files are added because it slips through the filter
  SetOutPath $INSTDIR\share\icons
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /a ${INKSCAPE_DIST_DIR}\share\icons\inkscape.file.png
  File /a ${INKSCAPE_DIST_DIR}\share\icons\inkscape.file.svg
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\share\templates
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /a ${INKSCAPE_DIST_DIR}\share\templates\default.en_US.svg
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
!endif
SectionEnd ; SecCore }}}

Section "$(GTKFiles)" SecGTK ; Mandatory GTK files section {{{
  SectionIn 1 2 3 RO
!ifndef DUMMYINSTALL
  DetailPrint "Installing GTK files..."
  SetOutPath $INSTDIR
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /a /r /x python ${INKSCAPE_DIST_DIR}\*.dll
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\lib
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /a /r /x locale /x aspell-0.60 ${INKSCAPE_DIST_DIR}\lib\*.*
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\etc
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /a /r ${INKSCAPE_DIST_DIR}\etc\*.*
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
!endif
SectionEnd ; SecGTK }}}

Section -SetCurrentUserOnly ; Set the installation to "current user" only by default {{{
!ifndef DUMMYINSTALL
  StrCpy $MultiUser 0
  SetShellVarContext current
!endif
SectionEnd ; -SetCurrentUserOnly }}}

Section "$(Alluser)" SecAlluser ; Then offer the user the option to make it global (default) {{{
  SectionIn 1 2 3
!ifndef DUMMYINSTALL
  ; disable this option in Win95/Win98/WinME
  StrCpy $MultiUser 1
  DetailPrint "Installing in administrator mode (registry root will be HKLM)"
  SetShellVarContext all
!endif
SectionEnd ; SecAllUser }}}

Section /o "$(DeletePrefs)" SecPrefs ; Delete user preferences before installation {{{
!ifndef DUMMYINSTALL
  !insertmacro delprefs
!endif
SectionEnd ; SecPrefs }}}

SectionGroup "$(Shortcuts)" SecShortcuts ; Create shortcuts for the user {{{

Section "$(Startmenu)" SecStartMenu ; Start menu shortcut {{{
  SectionIn 1 2 3
!ifndef DUMMYINSTALL
  CreateShortcut $SMPROGRAMS\Inkscape.lnk $INSTDIR\inkscape.exe
!endif
SectionEnd ; SecStartMenu }}}

Section "$(Desktop)" SecDesktop ; Desktop shortcut {{{
  SectionIn 1 2 3
!ifndef DUMMYINSTALL
  CreateShortCut $DESKTOP\Inkscape.lnk $INSTDIR\inkscape.exe
!endif
SectionEnd ; SecDesktop }}}

Section "$(Quicklaunch)" SecQuickLaunch ; Quick Launch shortcut {{{
  SectionIn 1 2 3
!ifndef DUMMYINSTALL
  ${IfThen} $QUICKLAUNCH != $TEMP ${|} CreateShortCut $QUICKLAUNCH\Inkscape.lnk $INSTDIR\inkscape.exe ${|}
!endif
SectionEnd ; SecQuickLaunch }}}

Section "$(SVGWriter)" SecSVGWriter ; Register Inkscape as the default application for .svg[z] {{{
  SectionIn 1 2 3
!ifndef DUMMYINSTALL
  DetailPrint "Associating SVG files with Inkscape"
  StrCpy $3 svg
  ${For} $2 0 1
    ${IfThen} $2 = 1 ${|} StrCpy $3 $3z ${|}
    ReadRegStr $0 HKCR ".$3" ""
    ${If} $0 == ""
      StrCpy $0 "$3file"
      WriteRegStr HKCR ".$3" "" $0
      WriteRegStr HKCR $0 "" "Scalable Vector Graphics file"
    ${EndIf}
    WriteRegStr HKCR $0\shell\edit\command "" `"$INSTDIR\Inkscape.exe" "%1"`
  ${Next}
!endif
SectionEnd ; SecSVGWriter }}}

Section "$(ContextMenu)" SecContextMenu ; Put Inkscape in the .svg[z] context menus (but not as default) {{{
  SectionIn 1 2 3
!ifndef DUMMYINSTALL
  DetailPrint "Adding Inkscape to SVG file context menu"
  ReadRegStr $0 HKCR .svg ""
  ${If} $0 == ""
    StrCpy $0 svgfile
    WriteRegStr HKCR .svg "" $0
    WriteRegStr HKCR $0 "" "Scalable Vector Graphics file"
  ${EndIf}
  WriteRegStr HKCR $0\shell\Inkscape\command "" `"$INSTDIR\Inkscape.exe" "%1"`

  ReadRegStr $0 HKCR .svgz ""
  ${If} $0 == ""
    StrCpy $0 svgzfile
    WriteRegStr HKCR .svgz "" $0
    WriteRegStr HKCR $0 "" "Scalable Vector Graphics file"
  ${EndIf}
  WriteRegStr HKCR $0\shell\Inkscape\command "" `"$INSTDIR\Inkscape.exe" "%1"`
!endif
SectionEnd ; SecContextMenu }}}

SectionGroupEnd ; SecShortcuts }}}

Section "$(Python)" SecPython ; Python distribution {{{
  SectionIn 1 2
!ifndef DUMMYINSTALL
  DetailPrint "Installing Python..."
  SetOutPath $INSTDIR\python
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r ${INKSCAPE_DIST_DIR}\python\*.*
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
!endif
SectionEnd ; SecPython }}}

SectionGroup "$(Addfiles)" SecAddfiles ; Additional files {{{

Section "$(Extensions)" SecExtensions ; Extensions {{{
  SectionIn 1 2
!ifndef DUMMYINSTALL
  DetailPrint "Installing extensions..."
  SetOutPath $INSTDIR\share\extensions
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r ${INKSCAPE_DIST_DIR}\share\extensions\*.*
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
!endif
SectionEnd ; SecExtensions }}}

Section "$(Examples)" SecExamples ; Install example SVG files {{{
  SectionIn 1 2
!ifndef DUMMYINSTALL
  DetailPrint "Installing examples..."
  SetOutPath $INSTDIR\share\examples
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r /x *.??*.???* ${INKSCAPE_DIST_DIR}\share\examples\*.*
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
!endif
SectionEnd ; SecExamples }}}

Section "$(Tutorials)" SecTutorials ; Install tutorials {{{
  SectionIn 1 2
!ifndef DUMMYINSTALL
  DetailPrint "Installing tutorials..."
  SetOutPath $INSTDIR\share\tutorials
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r /x *.??*.???* ${INKSCAPE_DIST_DIR}\share\tutorials\*.*
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
!endif
SectionEnd ; SecTutorials }}}

Section "$(Dictionaries)" SecDictionaries ; Aspell dictionaries {{{
  SectionIn 1 2
!ifndef DUMMYINSTALL
  DetailPrint "Installing dictionaries..."
  SetOutPath $INSTDIR\lib\aspell-0.60
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r ${INKSCAPE_DIST_DIR}\lib\aspell-0.60\*.*
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
!endif
SectionEnd ; SecDictionaries }}}

SectionGroupEnd ; SecAddfiles }}}

SectionGroup "$(Languages)" SecLanguages ; Languages sections {{{
  !macro Language SecName lng ; A macro to create each section {{{
    Section /o "$(lng_${lng}) (${lng})" Sec${SecName}
      SectionIn 1 ; flags will be adjusted below, see LanguageAutoSelect in .onInit
    !ifndef DUMMYINSTALL
      DetailPrint "Installing translations and translated content for ${SecName} (${lng}) locale..."
      ; locale folders (/locale, /share/locale /lib/locale)
      ${!defineifexist} ${INKSCAPE_DIST_DIR}\locale EXISTS 1
      !ifdef EXISTS
        !undef EXISTS
        SetOutPath $INSTDIR\locale\${lng}
        !insertmacro UNINSTALL.LOG_OPEN_INSTALL
        File /nonfatal /a /r ${INKSCAPE_DIST_DIR}\locale\${lng}\*.*
        !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      !endif
      ${!defineifexist} ${INKSCAPE_DIST_DIR}\share\locale EXISTS 1
      !ifdef EXISTS
        !undef EXISTS
        SetOutPath $INSTDIR\share\locale\${lng}
        !insertmacro UNINSTALL.LOG_OPEN_INSTALL
        File /nonfatal /a /r ${INKSCAPE_DIST_DIR}\share\locale\${lng}\*.*
        !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      !endif
      ${!defineifexist} ${INKSCAPE_DIST_DIR}\lib\locale EXISTS 1
      !ifdef EXISTS
        !undef EXISTS
        SetOutPath $INSTDIR\lib\locale\${lng}
        !insertmacro UNINSTALL.LOG_OPEN_INSTALL
        File /nonfatal /a /r ${INKSCAPE_DIST_DIR}\lib\locale\${lng}\*.*
        !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      !endif

      ; localized documentation, templates and tutorials
      SetOutPath $INSTDIR\doc
      !insertmacro UNINSTALL.LOG_OPEN_INSTALL
      File /nonfatal /a ${INKSCAPE_DIST_DIR}\doc\*.${lng}.html ; keys.${lng}.html
      File /nonfatal /a ${INKSCAPE_DIST_DIR}\doc\*.${lng}.txt ; HACKING.${lng}.html
      !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      SetOutPath $INSTDIR\share\templates
      !insertmacro UNINSTALL.LOG_OPEN_INSTALL
      File /nonfatal /a /r ${INKSCAPE_DIST_DIR}\share\templates\*.${lng}.svg
      !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      SectionGetFlags ${SecTutorials} $R1
      IntOp $R1 $R1 & ${SF_SELECTED}
      ${If} $R1 >= ${SF_SELECTED}
        SetOutPath $INSTDIR\share\tutorials
        !insertmacro UNINSTALL.LOG_OPEN_INSTALL
        File /nonfatal /a ${INKSCAPE_DIST_DIR}\share\tutorials\*.${lng}.*
        !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
      ${EndIf}
    !endif
    SectionEnd
  !macroend ; Language }}}

  ; Now create each section with the Language macro.
  ; include list of available inkscape translations from /languages/_language_lists.nsh
  !insertmacro INKSCAPE_TRANSLATIONS Language
SectionGroupEnd ; SecLanguages }}}

Section -FinalizeInstallation ; Hidden, mandatory section to finalize installation {{{
!ifndef DUMMYINSTALL
  DetailPrint "Finalizing installation"
  ${IfThen} $MultiUser  = 1 ${|} SetShellVarContext all ${|}
  ${IfThen} $MultiUser != 1 ${|} SetShellVarContext current ${|}

  WriteRegStr SHCTX "${INSTDIR_KEY}" ""           $INSTDIR\inkscape.exe
  WriteRegStr SHCTX "${INSTDIR_KEY}" MultiUser    $MultiUser
  WriteRegStr SHCTX "${INSTDIR_KEY}" askMultiUser $askMultiUser
  WriteRegStr SHCTX "${INSTDIR_KEY}" User         $User

  ; uninstall settings
  ; WriteUninstaller $INSTDIR\uninst.exe
  WriteRegExpandStr SHCTX "${UNINST_KEY}" UninstallString ${UNINST_EXE}
  WriteRegExpandStr SHCTX "${UNINST_KEY}" InstallDir      $INSTDIR
  WriteRegExpandStr SHCTX "${UNINST_KEY}" InstallLocation $INSTDIR
  WriteRegStr       SHCTX "${UNINST_KEY}" DisplayName     "Inkscape ${INKSCAPE_VERSION}"
  WriteRegStr       SHCTX "${UNINST_KEY}" DisplayIcon     $INSTDIR\Inkscape.exe,0
  WriteRegStr       SHCTX "${UNINST_KEY}" DisplayVersion  ${INKSCAPE_VERSION}
  WriteRegStr       SHCTX "${UNINST_KEY}" Publisher       "Inkscape Project"
  WriteRegStr       SHCTX "${UNINST_KEY}" URLInfoAbout    "https://inkscape.org"

  WriteRegDWORD     SHCTX "${UNINST_KEY}" NoModify        1
  WriteRegDWORD     SHCTX "${UNINST_KEY}" NoRepair        1

  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD     SHCTX "${UNINST_KEY}" EstimatedSize   "$0"

  ;create/update log always within .onInstSuccess function
  !insertmacro UNINSTALL.LOG_UPDATE_INSTALL

  DetailPrint "Creating MD5 checksums"
  ClearErrors
  FileOpen $0 $INSTDIR\Uninstall.dat r
  FileOpen $9 $INSTDIR\Uninstall.log w
  FileRead $0 $1 ; read first line (which is the header)
  ${IfNot} ${Errors}
    ${Do}
      ClearErrors
      FileRead $0 $1
      ${IfThen} ${Errors} ${|} ${ExitDo} ${|}
      StrCpy $1 $1 -2 ; strip \r\n from path
      ${If} ${FileExists} $1\*.* ; ignore directories
        ${Continue}
      ${EndIf}
      md5dll::GetMD5File /NOUNLOAD $1
      Pop $2
      ${IfThen} $2 != "" ${|} FileWrite $9 "$2  $1$\r$\n" ${|}
    ${Loop}
  ${EndIf}
  FileClose $0
  FileClose $9
  ; Not needed any more
  ; Delete $INSTDIR\Uninstall.dat ; actually this is checked for in UNINSTALL.LOG_PREPARE_INSTALL, so keep it for now...
!endif
SectionEnd ; -FinalizeInstallation }}}

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN ; Section descriptions {{{
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "$(CoreDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecGTK} "$(GTKFilesDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts} "$(ShortcutsDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecAlluser} "$(AlluserDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} "$(StartmenuDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "$(DesktopDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecQuickLaunch} "$(QuicklaunchDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSVGWriter} "$(SVGWriterDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecContextMenu} "$(ContextMenuDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPrefs} "$(DeletePrefsDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecPython} "$(PythonDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecAddfiles} "$(AddfilesDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecExtensions} "$(ExtensionsDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecExamples} "$(ExamplesDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTutorials} "$(TutorialsDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDictionaries} "$(DictionariesDesc)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecLanguages} "$(LanguagesDesc)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END ; Section descriptions }}}


Function .onInit ; initialise the installer {{{2
  ; This code will be executed before the sections, but due to the
  ; language code in the sections it must come after it in the code.

  ; Language detection {{{
  !insertmacro MUI_LANGDLL_DISPLAY

  !macro LanguageAutoSelect LocaleName LocaleID
    ${If} $LANGUAGE = ${LocaleID}
      SectionSetInstTypes ${Sec${LocaleName}} 3 ; this equals binary "011" (which flags the default for sections 1 and 2 but not 3)
                                                ; and is equivalent to "SectionIn 1 2"
    ${EndIf}
  !macroend

  ; include list for installer autoselection from /languages/_language_lists.nsh
  ; No need for English to be detected as it's the default
  !insertmacro INSTALLER_TRANSLATIONS LanguageAutoSelect
  ; End of language detection }}}

  ; ser the second InstType ("Optimal") as default
  SetCurInstType 1

  !insertmacro UNINSTALL.LOG_PREPARE_INSTALL ; prepare advanced uninstallation log script

  ;Extract InstallOptions INI files
  StrCpy $AskMultiUser 1
  StrCpy $MultiUser 0
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

  ; Check for administrative privileges {{{
  ClearErrors
  UserInfo::GetName
  ${If} ${Errors}
    ; This one means you don't need to care about admin or
    ; not admin because Windows 9x doesn't either
    ${IfCmd} MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(NOT_SUPPORTED)$(OK_CANCEL_DESC)" /SD IDOK IDCANCEL ${||} Quit ${|}
  ${Else}
    Pop $User
    UserInfo::GetAccountType
    Pop $1
    ${If} $1 != Admin
    ${AndIf} ${Cmd} ${|} MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(NO_ADMIN)$(OK_CANCEL_DESC)" /SD IDOK IDCANCEL ${|}
      Quit
    ${EndIf}
  ${EndIf} ; }}}

  ; Detect an Inkscape installation by another user {{{
  ReadRegStr $0 HKLM "${INSTDIR_KEY}" User                              ; first global...
  ${IfThen} $0 == "" ${|} ReadRegStr $0 HKCU "${INSTDIR_KEY}" User ${|} ; then current user
  ${If} $0 != ""
  ${AndIf} $0 != $User
  ${AndIf} ${Cmd} ${|} MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(DIFFERENT_USER)$(OK_CANCEL_DESC)" /SD IDOK IDCANCEL ${|}
    Quit
  ${EndIf} ; }}}

  ; Request uninstallation of an old Inkscape installation {{{
  ReadRegStr $R0 HKLM "${UNINST_KEY}" UninstallString
  ReadRegStr $R1 HKLM "${UNINST_KEY}" DisplayName
  ${If} $R0 == ""
    ReadRegStr $R0 HKCU "${UNINST_KEY}" UninstallString
    ReadRegStr $R1 HKCU "${UNINST_KEY}" DisplayName
  ${EndIf}
  ${If} $R0 != ""
  ${AndIf} ${Cmd} ${|} MessageBox MB_YESNO|MB_ICONEXCLAMATION "$(WANT_UNINSTALL_BEFORE)" /SD IDNO IDYES ${|}
    ExecWait $R0
  ${EndIf} ; }}}

  ; Process command-line arguments (for automation) {{{
  !echo `Creating code to process command-line arguments...`
  !verbose push
  !verbose 3
  ${GetParameters} $CMDARGS

  !macro Parameter Section key
    ${GetOptions} $CMDARGS /${key}= $1
    ${If} $1 == OFF
      SectionGetFlags ${Sec${Section}} $0
      IntOp $2 ${SF_SELECTED} ~
      IntOp $0 $0 & $2
      SectionSetFlags ${Sec${Section}} $0
    ${EndIf}
    ${If} $1 == ON
      SectionGetFlags ${Sec${Section}} $0
      IntOp $0 $0 | ${SF_SELECTED}
      SectionSetFlags ${Sec${Section}} $0
    ${EndIf}
  !macroend

  !insertmacro Parameter GTK          GTK
  !insertmacro Parameter Shortcuts    SHORTCUTS
  !insertmacro Parameter Alluser      ALLUSER
  !insertmacro Parameter Desktop      DESKTOP
  !insertmacro Parameter QuickLaunch  QUICKLAUNCH
  !insertmacro Parameter SVGWriter    SVGEDITOR
  !insertmacro Parameter ContextMenu  CONTEXTMENUE
  !insertmacro Parameter Prefs        PREFERENCES
  !insertmacro Parameter Python       PYTHON
  !insertmacro Parameter Addfiles     ADDFILES
  !insertmacro Parameter Extensions   EXTENSIONS
  !insertmacro Parameter Examples     EXAMPLES
  !insertmacro Parameter Tutorials    TUTORIALS
  !insertmacro Parameter Dictionaries DICTIONARIES
  !insertmacro Parameter Languages    LANGUAGES

  ; include list of available inkscape translations for parameter generation from /languages/_language_lists.nsh
  !insertmacro INKSCAPE_TRANSLATIONS Parameter

  ClearErrors
  ${GetOptions} $CMDARGS /? $1
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
      /PYTHON=(OFF/ON): python distribution$\r$\n \
      /ADDFILES=(OFF/ON): additional files$\r$\n \
      /EXTENSIONS=(OFF/ON): extensions$\r$\n \
      /EXAMPLES=(OFF/ON): examples$\r$\n \
      /TUTORIALS=(OFF/ON): tutorials$\r$\n \
      /DICTIONARIES=(OFF/ON): dictionaries$\r$\n \
      /LANGUAGES=(OFF/ON): translated menues, examples, etc.$\r$\n \
      /[locale code]=(OFF/ON): e.g am, es, es_MX as in Inkscape supported"
    Abort
  ${EndIf}
  !verbose pop ; }}}
FunctionEnd ; .onInit }}}



; Uninstaller code {{{1
Function un.onInit ; initialise uninstaller {{{
  ;begin uninstall, could be added on top of uninstall section instead
  ;!insertmacro UNINSTALL.LOG_BEGIN_UNINSTALL
  ${IfNot} ${FileExists} $INSTDIR\uninstall.log
    MessageBox MB_OK|MB_ICONEXCLAMATION "$(UninstallLogNotFound)" /SD IDOK
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
    ReadRegStr $0            HKLM "${INSTDIR_KEY}" ""
  ${If} $0 == $INSTDIR\inkscape.exe
    ReadRegStr $MultiUser    HKLM "${INSTDIR_KEY}" MultiUser
    ReadRegStr $askMultiUser HKLM "${INSTDIR_KEY}" askMultiUser
    ReadRegStr $0            HKLM "${INSTDIR_KEY}" User
  ${Else}
    ReadRegStr $MultiUser    HKCU "${INSTDIR_KEY}" MultiUser
    ReadRegStr $askMultiUser HKCU "${INSTDIR_KEY}" askMultiUser
    ReadRegStr $0            HKCU "${INSTDIR_KEY}" User
  ${EndIf}
  ;check user if applicable
  ${If} $0 != ""
  ${AndIf} $0 != $User
  ${AndIf} ${Cmd} ${|} MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(DIFFERENT_USER)$(OK_CANCEL_DESC)" /SD IDOK IDCANCEL ${|}
    Quit
  ${EndIf}

  !insertmacro MUI_INSTALLOPTIONS_EXTRACT inkscape.nsi.uninstall

  SetShellVarContext all
  ${IfThen} $MultiUser = 0 ${|} SetShellVarContext current ${|}
FunctionEnd ; un.onInit }}}

Function un.CustomPageUninstall ; {{{
  SetShellVarContext current
  !insertmacro MUI_HEADER_TEXT "$(UInstOpt)" "$(UInstOpt1)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE inkscape.nsi.uninstall "Field 1" Text "$APPDATA\Inkscape\"
  !insertmacro MUI_INSTALLOPTIONS_WRITE inkscape.nsi.uninstall "Field 2" Text "$(PurgePrefs)"
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY inkscape.nsi.uninstall
  !insertmacro MUI_INSTALLOPTIONS_READ $MultiUser inkscape.nsi.uninstall "Field 2" State
FunctionEnd ; un.CustomPageUninstall }}}

Section Uninstall ; do the uninstalling {{{
!ifndef DUMMYINSTALL
  ; remove personal settings
  SetShellVarContext current
  Delete $APPDATA\Inkscape\extension-errors.log
  ${If} $MultiUser = 0
    DetailPrint "Purging personal settings in $APPDATA\Inkscape"
    ;RMDir /r $APPDATA\Inkscape
    !insertmacro delprefs
  ${EndIf}

  ; Remove file associations for svg editor
  StrCpy $3 svg
  ${For} $2 0 1
    ${IfThen} $2 = 1 ${|} StrCpy $3 $3z ${|}
    DetailPrint "Removing file associations for $3 editor"
    ClearErrors
    ReadRegStr $0 HKCR .$3 ""
    ${IfNot} ${Errors}
      ReadRegStr $1 HKCR $0\shell\edit\command ""
      ${If} $1 == `"$INSTDIR\Inkscape.exe" "%1"`
        DeleteRegKey HKCR $0\shell\edit\command
      ${EndIf}

      ClearErrors
      ReadRegStr $1 HKCR $0\shell\open\command ""
      ${If} $1 == `"$INSTDIR\Inkscape.exe" "%1"`
        DeleteRegKey HKCR $0\shell\open\command
      ${EndIf}

      DeleteRegKey HKCR $0\shell\Inkscape
      DeleteRegKey /ifempty HKCR $0\shell\edit
      DeleteRegKey /ifempty HKCR $0\shell\open
      DeleteRegKey /ifempty HKCR $0\shell
      DeleteRegKey /ifempty HKCR $0
      DeleteRegKey /ifempty HKCR .$3
    ${EndIf}
  ${Next}

  SetShellVarContext all
  DeleteRegKey SHCTX "${INSTDIR_KEY}"
  DeleteRegKey SHCTX "${UNINST_KEY}"
  Delete $DESKTOP\Inkscape.lnk
  Delete $QUICKLAUNCH\Inkscape.lnk
  Delete $SMPROGRAMS\Inkscape.lnk
  ;just in case they are still there
  Delete "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk"
  Delete  $SMPROGRAMS\Inkscape\Inkscape.lnk
  RMDir   $SMPROGRAMS\Inkscape

  SetShellVarContext current
  DeleteRegKey SHCTX "${INSTDIR_KEY}"
  DeleteRegKey SHCTX "${UNINST_KEY}"
  Delete $DESKTOP\Inkscape.lnk
  Delete $QUICKLAUNCH\Inkscape.lnk
  Delete $SMPROGRAMS\Inkscape.lnk
  ;just in case they are still there
  Delete "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk"
  Delete $SMPROGRAMS\Inkscape\Inkscape.lnk
  RMDir  $SMPROGRAMS\Inkscape

  InitPluginsDir

  ClearErrors
  FileOpen $9 $INSTDIR\uninstall.log r
  ${If} ${Errors} ;else uninstallnotfound
    MessageBox MB_OK|MB_ICONEXCLAMATION "$(UninstallLogNotFound)" /SD IDOK
  ${Else}
    ${SHMessageBoxCheckInit} "inkscape_uninstall_other_files"
    ${Do}
      ClearErrors
      FileRead $9 $1
      ${IfThen} ${Errors} ${|} ${ExitDo} ${|}
      ; cat the line into md5 and filename
      StrLen $2 $1
      ${IfThen} $2 <= 35 ${|} ${Continue} ${|}
      StrCpy $3 $1 32
      StrCpy $filename $1 $2-36 34 ;remove trailing CR/LF
      StrCpy $filename $filename -2
      ; $filename = file
      ; $0 = shall file be deleted?
      ; $3 = MD5 when installed
      ; $4 = MD5 now

      ${If} ${FileExists} $filename
        md5dll::GetMD5File /NOUNLOAD $filename
        Pop $4 ;md5 of file
        ${If} $3 == $4
          StrCpy $0 ${IDYES}
        ${Else}
          ; the md5 sums does not match so we ask
          ${SHMessageBoxCheck} "$(MUI_UNTEXT_CONFIRM_TITLE)" "$(FileChanged)" ${MB_YESNO}|${MB_ICONQUESTION}
          Pop $0
        ${EndIf}

        ${If} $0 = ${IDYES}
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
    ${SHMessageBoxCheckCleanup}
  ${EndIf}
  FileClose $9
  ; remove Python cache files that may have been created
  loopFiles:
      StrCpy $R1 0
      ${Locate} "$INSTDIR" "/L=F /M=*.pyc" "un.DeleteFile"
      StrCmp $R1 0 0 loopFiles
  ; remove empty directories
  loopDirs:
    StrCpy $R1 0
    ${Locate} "$INSTDIR" "/L=DE" "un.DeleteDir"
    StrCmp $R1 0 0 loopDirs
  ; remove the uninstaller and installation directory itself
  Delete $INSTDIR\uninstall.dat
  Delete $INSTDIR\uninstall.log
  Delete $INSTDIR\uninstall.exe
  RMDir $INSTDIR
!endif
SectionEnd ; Uninstall }}}
; }}}

Function un.DeleteFile
  Delete $R9
  IntOp $R1 $R1 + 1
  Push 0 # required by ${Locate}!
FunctionEnd

Function un.DeleteDir
  RMDir $R9
  IntOp $R1 $R1 + 1
  Push 0 # required by ${Locate}!
FunctionEnd

; This file has been optimised for use in Vim with folding.
; (If you can't cope, :set nofoldenable) vim:fen:fdm=marker

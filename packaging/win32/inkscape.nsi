; #######################################
; Inkscape NSIS installer project file
; Used as of 0.40
; #######################################

; #######################################
; DEFINES
; #######################################
!define PRODUCT_NAME "Inkscape"
!define PRODUCT_VERSION "0.45+devel"
!define PRODUCT_PUBLISHER "Inkscape Organization"
!define PRODUCT_WEB_SITE "http://www.inkscape.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\inkscape.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
;!define UNINST_EXE "uninstall.exe"



; #######################################
; MUI   SETTINGS
; #######################################
; MUI 1.67 compatible ------
SetCompressor /SOLID lzma
!include "MUI.nsh"
!include "sections.nsh"
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "header.bmp"
!define MUI_COMPONENTSPAGE_SMALLDESC

;..................................................................................................
;Following two definitions required. Uninstall log will use these definitions.
;You may use these definitions also, when you want to set up the InstallDirRagKey,
;store the language selection, store Start Menu folder etc.
;Enter the windows uninstall reg sub key to add uninstall information to Add/Remove Programs also.

!define INSTDIR_REG_ROOT "HKLM"
!define INSTDIR_REG_KEY ${PRODUCT_UNINST_KEY}

;include the Uninstall log header
!include AdvUninstLog.nsh

;Specify the preferred uninstaller operation mode, either unattended or interactive.
;You have to type either !insertmacro UNATTENDED_UNINSTALL, or !insertmacro INTERACTIVE_UNINSTALL.
;Be aware only one of the following two macros has to be inserted, neither both, neither none.

;!insertmacro UNATTENDED_UNINSTALL
!insertmacro INTERACTIVE_UNINSTALL

!addplugindir .
!include "FileFunc.nsh"
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
!include "english.nsh" 
!include "breton.nsh"
!include "catalan.nsh"
!include "czech.nsh" 
!include "finnish.nsh" 
!include "french.nsh" 
!include "galician.nsh" 
!include "german.nsh" 
!include "italian.nsh" 
!include "japanese.nsh"
!include "polish.nsh" 
!include "russian.nsh" 
!include "slovak.nsh" 
!include "slovenian.nsh" 
!include "spanish.nsh" 

ReserveFile "inkscape.nsi.uninstall"
ReserveFile "${NSISDIR}\Plugins\UserInfo.dll"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS ;InstallOptions


; #######################################
; SETTINGS
; #######################################

Name              "${PRODUCT_NAME} ${PRODUCT_VERSION}"
Caption           $(lng_Caption)
OutFile           "Inkscape-${PRODUCT_VERSION}-1.win32.exe"
InstallDir        "$PROGRAMFILES\Inkscape"
InstallDirRegKey  HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails   hide
ShowUnInstDetails hide

var askMultiUser
Var MultiUser
var User

; #######################################
;  I N S T A L L E R    S E C T I O N S
; #######################################

; Turn off old selected section
; GetWindowsVersion
;
; Based on Yazno's function, http://yazno.tripod.com/powerpimpit/
; Updated by Joost Verburg
; Updated for Windows 98 SE by Matthew Win Tibbals 5-21-03
;
; Returns on top of stack
;
; Windows Version (95, 98, ME, NT x.x, 2000, XP, 2003)
; or
; '' (Unknown Windows Version)
;
; Usage:
;   Call GetWindowsVersion
;   Pop $R0
;   ; at this point $R0 is "NT 4.0" or whatnot
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Function GetWindowsVersion
 
  Push $R0
  Push $R1
 
  ClearErrors
 
  ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" CurrentVersion
 
  IfErrors 0 lbl_winnt
 
  ; we are not NT
  ReadRegStr $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion" VersionNumber
 
  StrCpy $R1 $R0 1
  StrCmp $R1 '4' 0 lbl_error
 
  StrCpy $R1 $R0 3
 
  StrCmp $R1 '4.0' lbl_win32_95
  StrCmp $R1 '4.9' lbl_win32_ME lbl_win32_98
 
  lbl_win32_95:
    StrCpy $R0 '95'
	StrCpy $AskMultiUser "0"
  Goto lbl_done
 
  lbl_win32_98:
    StrCpy $R0 '98'
	StrCpy $AskMultiUser "0"
  Goto lbl_done
  lbl_win32_ME:
    StrCpy $R0 'ME'
	StrCpy $AskMultiUser "0"
  Goto lbl_done
 
  lbl_winnt:
 
  StrCpy $R1 $R0 1
 
  StrCmp $R1 '3' lbl_winnt_x
  StrCmp $R1 '4' lbl_winnt_x
 
  StrCpy $R1 $R0 3
 
  StrCmp $R1 '5.0' lbl_winnt_2000
  StrCmp $R1 '5.1' lbl_winnt_XP
  StrCmp $R1 '5.2' lbl_winnt_2003 lbl_error
 
  lbl_winnt_x:
    StrCpy $R0 "NT $R0" 6
  Goto lbl_done
 
  lbl_winnt_2000:
    Strcpy $R0 '2000'
  Goto lbl_done
 
  lbl_winnt_XP:
    Strcpy $R0 'XP'
  Goto lbl_done
 
  lbl_winnt_2003:
    Strcpy $R0 '2003'
  Goto lbl_done
 
  lbl_error:
    Strcpy $R0 ''
  lbl_done:
 
  Pop $R1
  Exch $R0

FunctionEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 

 ; StrStr
 ; input, top of stack = string to search for
 ;        top of stack-1 = string to search in
 ; output, top of stack (replaces with the portion of the string remaining)
 ; modifies no other variables.
 ;
 ; Usage:
 ;   Push "this is a long ass string"
 ;   Push "ass"
 ;   Call StrStr
 ;   Pop $R0
 ;  ($R0 at this point is "ass string")

 Function StrStr
   Exch $R1 ; st=haystack,old$R1, $R1=needle
   Exch    ; st=old$R1,haystack
   Exch $R2 ; st=old$R1,old$R2, $R2=haystack
   Push $R3
   Push $R4
   Push $R5
   StrLen $R3 $R1
   StrCpy $R4 0
   ; $R1=needle
   ; $R2=haystack
   ; $R3=len(needle)
   ; $R4=cnt
   ; $R5=tmp
   loop:
     StrCpy $R5 $R2 $R3 $R4
     StrCmp $R5 $R1 done
     StrCmp $R5 "" done
     IntOp $R4 $R4 + 1
     Goto loop
 done:
   StrCpy $R1 $R2 "" $R4
   Pop $R5
   Pop $R4
   Pop $R3
   Pop $R2
   Exch $R1
 FunctionEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

 ; GetParameters
 ; input, none
 ; output, top of stack (replaces, with e.g. whatever)
 ; modifies no other variables.
 
 Function GetParameters
 
   Push $R0
   Push $R1
   Push $R2
   Push $R3
   
   StrCpy $R2 1
   StrLen $R3 $CMDLINE
   
   ;Check for quote or space
   StrCpy $R0 $CMDLINE $R2
   StrCmp $R0 '"' 0 +3
     StrCpy $R1 '"'
     Goto loop
   StrCpy $R1 " "
   
   loop:
     IntOp $R2 $R2 + 1
     StrCpy $R0 $CMDLINE 1 $R2
     StrCmp $R0 $R1 get
     StrCmp $R2 $R3 get
     Goto loop
   
   get:
     IntOp $R2 $R2 + 1
     StrCpy $R0 $CMDLINE 1 $R2
     StrCmp $R0 " " get
     StrCpy $R0 $CMDLINE "" $R2
   
   Pop $R3
   Pop $R2
   Pop $R1
   Exch $R0
 
 FunctionEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; GetParameterValue
; Chris Morgan<cmorgan@alum.wpi.edu> 5/10/2004
; -Updated 4/7/2005 to add support for retrieving a command line switch
;  and additional documentation
;
; Searches the command line input, retrieved using GetParameters, for the
; value of an option given the option name.  If no option is found the
; default value is placed on the top of the stack upon function return.
;
; This function can also be used to detect the existence of just a
; command line switch like /OUTPUT  Pass the default and "OUTPUT"
; on the stack like normal.  An empty return string "" will indicate
; that the switch was found, the default value indicates that
; neither a parameter or switch was found.
;
; Inputs - Top of stack is default if parameter isn't found,
;  second in stack is parameter to search for, ex. "OUTPUT"
; Outputs - Top of the stack contains the value of this parameter
;  So if the command line contained /OUTPUT=somedirectory, "somedirectory"
;  will be on the top of the stack when this function returns
;
; Register usage
;$R0 - default return value if the parameter isn't found
;$R1 - input parameter, for example OUTPUT from the above example
;$R2 - the length of the search, this is the search parameter+2
;      as we have '/OUTPUT='
;$R3 - the command line string
;$R4 - result from StrStr calls
;$R5 - search for ' ' or '"'
 
Function GetParameterValue
  Exch $R0  ; get the top of the stack(default parameter) into R0
  Exch      ; exchange the top of the stack(default) with
            ; the second in the stack(parameter to search for)
  Exch $R1  ; get the top of the stack(search parameter) into $R1
 
  ;Preserve on the stack the registers used in this function
  Push $R2
  Push $R3
  Push $R4
  Push $R5
 
  Strlen $R2 $R1+2    ; store the length of the search string into R2
 
  Call GetParameters  ; get the command line parameters
  Pop $R3             ; store the command line string in R3
 
  # search for quoted search string
  StrCpy $R5 '"'      ; later on we want to search for a open quote
  Push $R3            ; push the 'search in' string onto the stack
  Push '"/$R1='       ; push the 'search for'
  Call StrStr         ; search for the quoted parameter value
  Pop $R4
  StrCpy $R4 $R4 "" 1   ; skip over open quote character, "" means no maxlen
  StrCmp $R4 "" "" next ; if we didn't find an empty string go to next
 
  # search for non-quoted search string
  StrCpy $R5 ' '      ; later on we want to search for a space since we
                      ; didn't start with an open quote '"' we shouldn't
                      ; look for a close quote '"'
  Push $R3            ; push the command line back on the stack for searching
  Push '/$R1='        ; search for the non-quoted search string
  Call StrStr
  Pop $R4
 
  ; $R4 now contains the parameter string starting at the search string,
  ; if it was found
next:
  StrCmp $R4 "" check_for_switch ; if we didn't find anything then look for
                                 ; usage as a command line switch
  # copy the value after /$R1= by using StrCpy with an offset of $R2,
  # the length of '/OUTPUT='
  StrCpy $R0 $R4 "" $R2  ; copy commandline text beyond parameter into $R0
  # search for the next parameter so we can trim this extra text off
  Push $R0
  Push $R5            ; search for either the first space ' ', or the first
                      ; quote '"'
                      ; if we found '"/output' then we want to find the
                      ; ending ", as in '"/output=somevalue"'
                      ; if we found '/output' then we want to find the first
                      ; space after '/output=somevalue'
  Call StrStr         ; search for the next parameter
  Pop $R4
  StrCmp $R4 "" done  ; if 'somevalue' is missing, we are done
  StrLen $R4 $R4      ; get the length of 'somevalue' so we can copy this
                      ; text into our output buffer
  StrCpy $R0 $R0 -$R4 ; using the length of the string beyond the value,
                      ; copy only the value into $R0
  goto done           ; if we are in the parameter retrieval path skip over
                      ; the check for a command line switch
 
; See if the parameter was specified as a command line switch, like '/output'
check_for_switch:
  Push $R3            ; push the command line back on the stack for searching
  Push '/$R1'         ; search for the non-quoted search string
  Call StrStr
  Pop $R4
  StrCmp $R4 "" done  ; if we didn't find anything then use the default
  StrCpy $R0 ""       ; otherwise copy in an empty string since we found the
                      ; parameter, just didn't find a value
 
done:
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0 ; put the value in $R0 at the top of the stack
FunctionEnd

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

!macro Language polng lng
  SectionIn 1 2 3
  SetOutPath $INSTDIR
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a "..\..\inkscape\*.${lng}.txt"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\locale
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r "..\..\inkscape\locale\${polng}"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\lib\locale
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r "..\..\inkscape\lib\locale\${polng}"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\share\clipart
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r "..\..\inkscape\share\clipart\*.${polng}.svg"  
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  ; the keyboard tables
  SetOutPath $INSTDIR\share\screens
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r "..\..\inkscape\share\screens\*.${polng}.svg"  
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\share\templates
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r "..\..\inkscape\share\templates\*.${polng}.svg"  
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SetOutPath $INSTDIR\doc
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r "..\..\inkscape\doc\keys.${polng}.xml"  
  File /nonfatal /a /r "..\..\inkscape\doc\keys.${polng}.html"  
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  SectionGetFlags ${SecTutorials} $R1 
  IntOp $R1 $R1 & ${SF_SELECTED} 
  IntCmp $R1 ${SF_SELECTED} 0 skip_tutorials 
    SetOutPath $INSTDIR\share\tutorials
    !insertmacro UNINSTALL.LOG_OPEN_INSTALL
    File /nonfatal /a "..\..\inkscape\share\tutorials\*.${polng}.*"
    !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
  skip_tutorials:
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 

;;;;;;;;;;;;;;;;;;;;;;;;;;
; Delete prefs
; code taken from the vlc project
;;;;;;;;;;;;;;;;;;;;;;;;;;
!macro delprefs
  StrCpy $0 0
	DetailPrint "Delete personal preferences ..."
	DetailPrint "try to find all users ..."
	delprefs-Loop:
 ; FIXME
  ; this will loop through all the logged users and "virtual" windows users
  ; (it looks like users are only present in HKEY_USERS when they are logged in)
    ClearErrors
    EnumRegKey $1 HKU "" $0
    StrCmp $1 "" delprefs-End
    IntOp $0 $0 + 1
    ReadRegStr $2 HKU "$1\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" AppData
    StrCmp $2 "" delprefs-Loop
	DetailPrint "$2\Inkscape will be removed"
    Delete "$2\Inkscape\preferences.xml"
    Delete "$2\Inkscape\extension-errors.log"
    RMDir "$2\Inkscape"
    Goto delprefs-Loop
  delprefs-End:
!macroend


;--------------------------------
; Installer Sections
Section -removeInkscape
  ; check for an old installation and clean that dlls and stuff

FindFirstINSTDIR:
    FindFirst $0 $1 $INSTDIR\*.*
FindINSTDIR:
    StrCmp $1 "" FindNextDoneINSTDIR
    StrCmp $1 "." FindNextINSTDIR
	StrCmp $1 ".." FindNextINSTDIR
	Goto FoundSomethingINSTDIR
FindNextINSTDIR:
    FindNext $0 $1
	Goto FindINSTDIR
FoundSomethingINSTDIR:
	MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "$(lng_ClearDirectoryBefore)" /SD IDCANCEL IDRETRY FindFirstINSTDIR
	Quit
FindNextDoneINSTDIR:

    ;remove the old inkscape shortcuts from the startmenu
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
  
SectionEnd

Section $(lng_Core) SecCore

  DetailPrint "Installing Inkscape Core Files ..."

  SectionIn 1 2 3 RO
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
SectionEnd

Section $(lng_GTKFiles) SecGTK

  DetailPrint "Installing GTK Files ..."
  
  SectionIn 1 2 3 RO
  SetOutPath $INSTDIR
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  SetOverwrite on
  File /a /r "..\..\inkscape\*.dll"
  File /a /r /x "locale" "..\..\inkscape\lib"
  File /a /r "..\..\inkscape\etc"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
SectionEnd

Section $(lng_Alluser) SecAlluser
  ; disable this option in Win95/Win98/WinME
  SectionIn 1 2 3 
  StrCpy $MultiUser "1"
  StrCmp $MultiUser "1" "" SingleUser
    DetailPrint "admin mode, registry root will be HKLM"
    SetShellVarContext all
    Goto endSingleUser
  SingleUser:
    DetailPrint "single user mode, registry root will be HKCU"
    SetShellVarContext current
  endSingleUser:		
SectionEnd

SectionGroup $(lng_Shortcuts) SecShortcuts

Section /o $(lng_Desktop) SecDesktop
  ClearErrors
  CreateShortCut "$DESKTOP\Inkscape.lnk" "$INSTDIR\inkscape.exe"
  IfErrors 0 +2
    DetailPrint "Uups! Problems creating desktop shortcuts"
SectionEnd

Section /o $(lng_Quicklaunch) SecQuicklaunch
  ClearErrors
  StrCmp $QUICKLAUNCH $TEMP +2
    CreateShortCut "$QUICKLAUNCH\Inkscape.lnk" "$INSTDIR\inkscape.exe"
  IfErrors 0 +2
    DetailPrint "Uups! Problems creating quicklaunch shortcuts"
SectionEnd

Section $(lng_SVGWriter) SecSVGWriter 
  SectionIn 1 2 3
  ; create file associations, test before if needed
  DetailPrint "creating file associations"
  ClearErrors
  ReadRegStr $0 HKCR ".svg" ""
  StrCmp $0 "" 0 +3
    WriteRegStr HKCR ".svg" "" "svgfile"
    WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  ReadRegStr $0 HKCR ".svgz" ""
  StrCmp $0 "" 0 +3
    WriteRegStr HKCR ".svgz" "" "svgfile"
    WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  IfErrors 0 +2
    DetailPrint "Uups! Problems creating file assoziations for svg writer"
  
  DetailPrint "creating default editor"
  ClearErrors
  ReadRegStr $0 HKCR ".svg" ""
  WriteRegStr HKCR "$0\shell\edit\command" "" '"$INSTDIR\Inkscape.exe" "%1"'
  ReadRegStr $0 HKCR ".svgz" ""
  WriteRegStr HKCR "$0\shell\edit\command" "" '"$INSTDIR\Inkscape.exe" "%1"'
  IfErrors 0 +2
    DetailPrint "Uups! Problems creating default editor"
SectionEnd

Section $(lng_ContextMenu) SecContextMenu
  SectionIn 1 2 3
  ; create file associations, test before if needed
  DetailPrint "creating file associations"
  ClearErrors
  ReadRegStr $0 HKCR ".svg" ""
  StrCmp $0 "" 0 +3
    WriteRegStr HKCR ".svg" "" "svgfile"
    WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  ReadRegStr $0 HKCR ".svgz" ""
  StrCmp $0 "" 0 +3
    WriteRegStr HKCR ".svgz" "" "svgfile"
    WriteRegStr HKCR "svgfile" "" "Scalable Vector Graphics file"
  IfErrors 0 +2
    DetailPrint "Uups! Problems creating file assoziations for context menu"
  
  DetailPrint "creating context menue"
  ClearErrors
  ReadRegStr $0 HKCR ".svg" ""
  WriteRegStr HKCR "$0\shell\${PRODUCT_NAME}\command" "" '"$INSTDIR\Inkscape.exe" "%1"'
  ReadRegStr $0 HKCR ".svgz" ""
  WriteRegStr HKCR "$0\shell\${PRODUCT_NAME}\command" "" '"$INSTDIR\Inkscape.exe" "%1"'
  IfErrors 0 +2
    DetailPrint "Uups! Problems creating context menue integration"

SectionEnd

SectionGroupEnd

Section /o $(lng_DeletePrefs) SecPrefs
	!insertmacro delprefs
SectionEnd

SectionGroup $(lng_Addfiles) SecAddfiles

Section $(lng_Examples) SecExamples
  SectionIn 1 2
  SetOutPath $INSTDIR\share
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r /x "*.??*.???*" "..\..\inkscape\share\examples"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
SectionEnd

Section $(lng_Tutorials) SecTutorials
  SectionIn 1 2
  SetOutPath $INSTDIR\share
  !insertmacro UNINSTALL.LOG_OPEN_INSTALL
  File /nonfatal /a /r /x "*.??*.???*" "..\..\inkscape\share\tutorials"
  !insertmacro UNINSTALL.LOG_CLOSE_INSTALL
SectionEnd

SectionGroupEnd

SectionGroup /e $(lng_Languages) SecLanguages

Section $(lng_am) SecAmharic
  !insertmacro Language am am
SectionEnd

Section $(lng_ar) SecArabic
  !insertmacro Language ar ar
SectionEnd

Section $(lng_az) SecAzerbaijani
  !insertmacro Language az az
SectionEnd

Section $(lng_be) SecByelorussian
  !insertmacro Language be be
SectionEnd

Section $(lng_bg) SecBulgarian
  !insertmacro Language bg bg
SectionEnd

Section $(lng_bn) SecBengali
  !insertmacro Language bn bn
SectionEnd

Section $(lng_br) SecBreton
  !insertmacro Language br br
SectionEnd

Section $(lng_ca) SecCatalan
  !insertmacro Language ca ca
SectionEnd

Section $(lng_ca@valencia) SecCatalanValencia
  !insertmacro Language ca@valencia ca@valencia
SectionEnd

Section $(lng_cs) SecCzech
  !insertmacro Language cs cs
SectionEnd

Section $(lng_da) SecDanish
  !insertmacro Language da da
SectionEnd

Section $(lng_de) SecGerman
  !insertmacro Language 'de' 'de'
SectionEnd

Section $(lng_dz) SecDzongkha
  !insertmacro Language dz dz
SectionEnd

Section $(lng_el) SecGreek
  !insertmacro Language el el
SectionEnd

Section $(lng_en) SecEnglish
  SectionIn 1 2 3 RO
SectionEnd

Section $(lng_en_AU) SecEnglishAustralian
  !insertmacro Language en_AU en_AU
SectionEnd

Section $(lng_en_CA) SecEnglishCanadian
  !insertmacro Language en_CA en_CA
SectionEnd

Section $(lng_en_GB) SecEnglishBritain
  !insertmacro Language en_GB en_GB
SectionEnd

Section $(lng_en_US@piglatin) SecEnglishPiglatin
  !insertmacro Language en_US@piglatin en_US@Piglatin
SectionEnd

Section $(lng_eo) SecEsperanto
  !insertmacro Language eo eo
SectionEnd

Section $(lng_es) SecSpanish
  !insertmacro Language 'es' 'es'
SectionEnd

Section $(lng_es_MX) SecSpanishMexico
  !insertmacro Language 'es_MX' 'es_MX'
SectionEnd

Section $(lng_et) SecEstonian
  !insertmacro Language et et
SectionEnd

Section $(lng_eu) SecBasque
  !insertmacro Language eu eu
SectionEnd

Section $(lng_fr) SecFrench
  !insertmacro Language 'fr' 'fr'
SectionEnd

Section $(lng_fi) SecFinnish
  !insertmacro Language 'fi' 'fi'
SectionEnd

Section $(lng_ga) SecIrish
  !insertmacro Language ga ga
SectionEnd

Section $(lng_gl) SecGallegan
  !insertmacro Language gl gl
  SectionIn 1 2 3
SectionEnd

Section $(lng_he) SecHebrew
  !insertmacro Language he he
  SectionIn 1 2 3
SectionEnd

Section $(lng_hr) SecCroatian
  !insertmacro Language hr hr
  SectionIn 1 2 3
SectionEnd

Section $(lng_hu) SecHungarian
  !insertmacro Language hu hu
  SectionIn 1 2 3
SectionEnd

Section $(lng_id) SecIndonesian
  !insertmacro Language id id
  SectionIn 1 2 3
SectionEnd

Section $(lng_it) SecItalian
  !insertmacro Language it it
  SectionIn 1 2 3
SectionEnd

Section $(lng_ja) SecJapanese
  !insertmacro Language 'ja' 'jp'
SectionEnd

Section $(lng_km) SecKhmer
  !insertmacro Language km km
SectionEnd

Section $(lng_ko) SecKorean
  !insertmacro Language 'ko' 'ko'
SectionEnd

Section $(lng_lt) SecLithuanian
  !insertmacro Language 'lt' 'lt'
SectionEnd

Section $(lng_mn) SecMongolian
  !insertmacro Language mn mn
SectionEnd

Section $(lng_mk) SecMacedonian
  !insertmacro Language mk mk
SectionEnd

Section $(lng_nb) SecNorwegianBokmal
  !insertmacro Language nb nb
SectionEnd

Section $(lng_ne) SecNepali
  !insertmacro Language ne ne
SectionEnd

Section $(lng_nl) SecDutch
  !insertmacro Language nl nl
SectionEnd

Section $(lng_nn) SecNorwegianNynorsk
  !insertmacro Language nn nn
SectionEnd

Section $(lng_pa) SecPanjabi
  !insertmacro Language pa pa
SectionEnd

Section $(lng_pl) SecPolish
  !insertmacro Language pl pl
SectionEnd

Section $(lng_pt) SecPortuguese
  !insertmacro Language pt pt
SectionEnd

Section $(lng_pt_BR) SecPortugueseBrazil
  !insertmacro Language pt_BR pt_BR
SectionEnd

Section $(lng_ro) SecRomanian
  !insertmacro Language ro ro
SectionEnd

Section $(lng_ru) SecRussian
  !insertmacro Language ru ru
SectionEnd

Section $(lng_rw) SecKinyarwanda
  !insertmacro Language rw rw
SectionEnd

Section $(lng_sk) SecSlovak
  !insertmacro Language sk sk
SectionEnd

Section $(lng_sl) SecSlovenian
  !insertmacro Language sl sl
SectionEnd

Section $(lng_sq) SecAlbanian
  !insertmacro Language sq sq
SectionEnd

Section $(lng_sr) SecSerbian
  !insertmacro Language sr sr
SectionEnd

Section $(lng_sr@latin) SecSerbianLatin
  !insertmacro Language 'sr@latin' 'sr@latin'
SectionEnd

Section $(lng_sv) SecSwedish
  !insertmacro Language sv sv
SectionEnd

Section $(lng_th) SecThai
  !insertmacro Language th th
SectionEnd

Section $(lng_tr) SecTurkish
  !insertmacro Language tr tr
SectionEnd

Section $(lng_uk) SecUkrainian
  !insertmacro Language uk uk
SectionEnd

Section $(lng_vi) SecVietnamese
  !insertmacro Language vi vi
SectionEnd

Section $(lng_zh_CN) SecChineseSimplified
  !insertmacro Language zh_CN zh_CN
SectionEnd

Section $(lng_zh_TW) SecChineseTaiwan
  !insertmacro Language zh_TW zh_TW
SectionEnd

SectionGroupEnd


Section -FinalizeInstallation
	DetailPrint "finalize installation"
  StrCmp $MultiUser "1" "" SingleUser
    DetailPrint "admin mode, registry root will be HKLM"
    SetShellVarContext all
    Goto endSingleUser
  SingleUser:
    DetailPrint "single user mode, registry root will be HKCU"
    SetShellVarContext current
  endSingleUser:		

  ; check for writing registry
  ClearErrors
  WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\inkscape.exe"  
  ;IfErrors 0 +4
  ;  DetailPrint "fatal: failed to write to ${PRODUCT_DIR_REGKEY}"
  ;  DetailPrint "aborting installation"
  ;	Abort
  WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "MultiUser" "$MultiUser"  
  WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "askMultiUser" "$askMultiUser"
  WriteRegStr SHCTX "${PRODUCT_DIR_REGKEY}" "User" "$User"
  IfErrors 0 +2
    DetailPrint "fatal: failed to write to registry installation info"

  ; start menu entries
  ClearErrors
  CreateShortCut "$SMPROGRAMS\Inkscape.lnk" "$INSTDIR\inkscape.exe"
  IfErrors 0 +2
    DetailPrint "fatal: failed to write to start menu info"

  ; uninstall settings
  ClearErrors
  ; WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegExpandStr SHCTX "${PRODUCT_UNINST_KEY}" "UninstallString" "${UNINST_EXE}"
  WriteRegExpandStr SHCTX "${PRODUCT_UNINST_KEY}" "InstallDir" "$INSTDIR"
  WriteRegExpandStr SHCTX "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Inkscape.exe,0"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoModify" "1"
  WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoRepair" "1"
  IfErrors 0 +2
    DetailPrint "fatal: failed to write to registry un-installation info"

  ;create/update log always within .onInstSuccess function
  !insertmacro UNINSTALL.LOG_UPDATE_INSTALL
  
	DetailPrint "create MD5 sums"
	ClearErrors
	FileOpen $0 $INSTDIR\uninstall.dat r
	FileOpen $9 $INSTDIR\uninstall.log w
	IfErrors doneinstall
readnextlineinstall:
	ClearErrors
	FileRead $0 $1
	IfErrors doneinstall
	StrCpy $1 $1 -2
	;DetailPrint $1
	md5dll::GetMD5File /NOUNLOAD $1
	Pop $2
	;DetailPrint $2
	StrCmp $2 "" +2
	FileWrite $9 "$2  $1$\r$\n"
	Goto readnextlineinstall
doneinstall:
	FileClose $0
	FileClose $9
	; this file is not needed anymore
	Delete $INSTDIR\uninstall.dat
	
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
  Push ${key}
  Push ""
  Call GetParameterValue
  Pop $1
  StrCmp $1 "OFF" 0 +5
    SectionGetFlags ${Section} $0
    IntOp $2 ${SF_SELECTED} ~
    IntOp $0 $0 & $2
    SectionSetFlags ${Section} $0
  StrCmp $1 "ON" 0 +4
    SectionGetFlags ${Section} $0
    IntOp $0 $0 | ${SF_SELECTED}
    SectionSetFlags ${Section} $0
!macroend

Function .onInit
  ;prepare log always within .onInit function
  !insertmacro UNINSTALL.LOG_PREPARE_INSTALL

  ;Extract InstallOptions INI files
  StrCpy $AskMultiUser "1"
  StrCpy $MultiUser "0"
  ; this resets AskMultiUser if Win95/98/ME
  Call GetWindowsVersion
  Pop $R0
  DetailPrint "detected operating system $R0"
  ;MessageBox MB_OK "operating system: $R0; AskMultiuser: $AskMultiUser"
  
  ; hide all user section if win98
  StrCmp $AskMultiUser "1" +2
    SectionSetText ${SecAlluser} ""

  ; hide if quick launch if not available
  StrCmp $QUICKLAUNCH $TEMP 0 +2
    SectionSetText ${SecQuicklaunch} ""

  ;check if user is admin
  ClearErrors
	UserInfo::GetName
	IfErrors info_Win9x
	Pop $0
	StrCpy $User $0
	UserInfo::GetAccountType
	Pop $1
	StrCmp $1 "Admin" info_done

	MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(lng_NO_ADMIN)$(lng_OK_CANCEL_DESC)" /SD IDOK IDOK info_done IDCANCEL +1
		Quit
		
	Goto info_done

	info_Win9x:
		# This one means you don't need to care about admin or
		# not admin because Windows 9x doesn't either
		MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(lng_NOT_SUPPORTED)$(lng_OK_CANCEL_DESC)" /SD IDOK IDOK info_done IDCANCEL +1
			Quit
			
	info_done:

  ;check for previous installation
  ReadRegStr $0 HKLM "${PRODUCT_DIR_REGKEY}" "User"
  StrCmp $0 "" +1 +2
  ReadRegStr $0 HKCU "${PRODUCT_DIR_REGKEY}" "User"
  ;check user if applicable
  StrCmp $0 "" diff_user_install_done
    StrCmp $0 $User diff_user_install_done
	  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(lng_DIFFERENT_USER)$(lng_OK_CANCEL_DESC)" /SD IDOK IDOK diff_user_install_done IDCANCEL +1
		Quit
   diff_user_install_done:
	  
  ; call uninstall first
  ; code taken from the vlc project
    ReadRegStr $R0  HKLM ${PRODUCT_UNINST_KEY} "UninstallString"
	ReadRegStr $R1  HKLM ${PRODUCT_UNINST_KEY} "DisplayName"
	StrCmp $R0 "" +1 +3
    ReadRegStr $R0  HKCU ${PRODUCT_UNINST_KEY} "UninstallString"
	ReadRegStr $R1  HKCU ${PRODUCT_UNINST_KEY} "DisplayName"
	StrCmp $R0 "" uninstall_before_done
	 
	  MessageBox MB_YESNO|MB_ICONEXCLAMATION $(lng_WANT_UNINSTALL_BEFORE) /SD IDNO IDNO uninstall_before_done 
	  ;Run the uninstaller
	  ;uninst:
		DetailPrint "execute $R0 in $INSTDIR"
	    ClearErrors
	    ExecWait '$R0 _?=$INSTDIR' ;Do not copy the uninstaller to a temp file
 	  uninstall_before_done:
	  
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
  
  Push "?"
  Push "TEST"
  Call GetParameterValue
  Pop $1
  StrCmp $1 "TEST" +3
    MessageBox MB_OK "possible parameters for installer:$\r$\n \
      /?: this help screen$\r$\n \
      /S: silent$\r$\n \
      /D=(directory): where to install inkscape$\r$\n \
      /GTK=(OFF/ON): GTK+ Runtime environment$\r$\n \
      /SHORTCUTS=(OFF/ON): shortcuts to start inkscape$\r$\n \
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
FunctionEnd

Function .onSelChange
FunctionEnd


Function .onInstSuccess

FunctionEnd

; --------------------------------------------------

Function un.CustomPageUninstall
  !insertmacro MUI_HEADER_TEXT "$(lng_UInstOpt)" "$(lng_UInstOpt1)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.uninstall" "Field 1" "Text" "$APPDATA\Inkscape\"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.uninstall" "Field 2" "Text" "$(lng_PurgePrefs)"

  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "inkscape.nsi.uninstall"
  !insertmacro MUI_INSTALLOPTIONS_READ $MultiUser "inkscape.nsi.uninstall" "Field 2" "State"
  DetailPrint "keepfiles = $MultiUser" 
	  ;MessageBox MB_OK "adminmode = $MultiUser MultiUserOS = $askMultiUser"

FunctionEnd


Function un.onInit
  ;begin uninstall, could be added on top of uninstall section instead
  ;!insertmacro UNINSTALL.LOG_BEGIN_UNINSTALL
	IfFileExists $INSTDIR\uninstall.log uninstalllogpresent
	MessageBox MB_OK|MB_ICONEXCLAMATION "$(lng_UninstallLogNotFound)" /SD IDOK
	Quit
uninstalllogpresent:
  ClearErrors
  StrCpy $User ""
	UserInfo::GetName
	IfErrors +3
	Pop $0
	StrCpy $User $0

  StrCpy $askMultiUser "1"
  StrCpy $MultiUser "1"
 
  ; Test if this was a multiuser installation
  ReadRegStr $0 HKLM "${PRODUCT_DIR_REGKEY}" ""
  StrCmp $0  "$INSTDIR\inkscape.exe" 0 hkcu_user_uninstall  
    ReadRegStr $MultiUser HKLM "${PRODUCT_DIR_REGKEY}" "MultiUser"
    ReadRegStr $askMultiUser HKLM "${PRODUCT_DIR_REGKEY}" "askMultiUser"
	ReadRegStr $0 HKLM "${PRODUCT_DIR_REGKEY}" "User"
	Goto check_user_uninstall
  hkcu_user_uninstall:
  ReadRegStr $MultiUser HKCU "${PRODUCT_DIR_REGKEY}" "MultiUser"
  ReadRegStr $askMultiUser HKCU "${PRODUCT_DIR_REGKEY}" "askMultiUser"
  ReadRegStr $0 HKCU "${PRODUCT_DIR_REGKEY}" "User"
  ;check user if applicable
  check_user_uninstall:
  StrCmp $0 "" diff_user_uninstall_done
	StrCmp $0 $User diff_user_uninstall_done
	  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(lng_DIFFERENT_USER)$(lng_OK_CANCEL_DESC)" /SD IDOK IDOK diff_user_uninstall_done IDCANCEL +1
		Quit
  diff_user_uninstall_done:
    
 !insertmacro MUI_INSTALLOPTIONS_EXTRACT "inkscape.nsi.uninstall"

 ;check whether Multi user installation ?
 SetShellVarContext all
 StrCmp $MultiUser "0" 0 +2 
 SetShellVarContext current
 ;MessageBox MB_OK "adminmode = $MultiUser MultiUserOS = $askMultiUser" 
   
FunctionEnd

# removes a file and if the directory is empty afterwards the directory also
# push md5, push filename, call unremovefilename
Function un.RemoveFile
	Var /Global filename
	Var /Global md5sum
	Var /Global ismd5sum
	Var /Global removenever	; never remove a touched file
	Var /Global removealways	; always remove files touched by user
	Pop $filename
	Pop $md5sum
	
	IfFileExists $filename +2 0
		Return
	StrCmp $removealways "always" unremovefile 0
	md5dll::GetMD5File /NOUNLOAD $filename
	Pop $ismd5sum ;md5 of file
	StrCmp $md5sum $ismd5sum  unremovefile 0
	;DetailPrint "uups MD5 does not match"
	StrCmp $removenever "never" 0 +2
		Return
	; the md5 sums does not match so we ask
	messagebox::show MB_DEFBUTTON3|MB_TOPMOST "" "0,103" \
		"$(lng_FileChanged)" "$(lng_Yes)" "$(lng_AlwaysYes)" "$(lng_No)" "$(lng_AlwaysNo)"
	;DetailPrint "messagebox finished"
	Pop $md5sum
	;DetailPrint "messagebox call returned... $md5sum"
	StrCmp $md5sum "1" unremovefile 0	; Yes
	StrCmp $md5sum "2" 0 unremoveno	; Yes always
	StrCpy $removealways "always"
	;DetailPrint "removealways"
	Goto unremovefile
unremoveno:
	StrCmp $md5sum "3" 0 unremovenever	; No
	;DetailPrint "No remove"
	Return
unremovenever:
	StrCpy $removenever "never"
	;DetailPrint "removenever"
	Return
unremovefile:
	;DetailPrint "removefile"
	ClearErrors
	Delete $filename
	;now recursivly remove the path
unrmdir:
	${un.GetParent} $filename $filename
	IfErrors 0 +2
		Return
	RMDir $filename
	IfErrors +2
		Goto unrmdir
FunctionEnd

Section Uninstall

  ; remove personal settings
  Delete "$APPDATA\Inkscape\extension-errors.log"
  StrCmp $MultiUser "0" 0 endPurge  ; multiuser assigned in dialog
    DetailPrint "purge personal settings in $APPDATA\Inkscape"
    ;RMDir /r "$APPDATA\Inkscape"
	!insertmacro delprefs
	endPurge:

  ; Remove file associations for svg editor
  DetailPrint "removing file associations for svg editor"
  ClearErrors
  ReadRegStr $0 HKCR ".svg" ""
  DetailPrint ".svg associated as $0"
  IfErrors endUninstSVGEdit  
    ReadRegStr $1 HKCR "$0\shell\edit\command" ""
	IfErrors 0 +2  
      DetailPrint "svg editor is $1"
    StrCmp $1 '"$INSTDIR\Inkscape.exe" "%1"' 0 +3
      DetailPrint "removing default .svg editor"
      DeleteRegKey HKCR "$0\shell\edit\command"
    DeleteRegKey /ifempty HKCR "$0\shell\edit"
    DeleteRegKey /ifempty HKCR "$0\shell"
    DeleteRegKey /ifempty HKCR "$0"
  endUninstSVGEdit:
  
  ClearErrors
  ReadRegStr $2 HKCR ".svgz" ""
  DetailPrint ".svgz associated as $2"
  IfErrors endUninstSVGZEdit  
    ReadRegStr $3 HKCR "$2\shell\edit\command" ""
    IfErrors 0 +2  
      DetailPrint "svgz editor is $1"
    StrCmp $3 '"$INSTDIR\Inkscape.exe" "%1"' 0 +3
      DetailPrint "removing default .svgz editor"
      DeleteRegKey HKCR "$2\shell\edit\command"
    DeleteRegKey /ifempty HKCR "$2\shell\edit"
    DeleteRegKey /ifempty HKCR "$2\shell"
    DeleteRegKey /ifempty HKCR "$2"
  endUninstSVGZEdit:
  
  ; Remove file associations for svg editor
  DetailPrint "removing file associations for svg editor"
  ClearErrors
  ReadRegStr $0 HKCR ".svg" ""
  IfErrors endUninstSVGView
    ReadRegStr $1 HKCR "$0\shell\open\command" ""
    IfErrors 0 +2  
      DetailPrint "svg viewer is $1"
    StrCmp $1 '"$INSTDIR\Inkscape.exe" "%1"' 0 +3
      DetailPrint "removing default .svg viewer"
      DeleteRegKey HKCR "$0\shell\open\command"
    DeleteRegKey /ifempty HKCR "$0\shell\open"
    DeleteRegKey /ifempty HKCR "$0\shell"
    DeleteRegKey /ifempty HKCR "$0"
  endUninstSVGView:
  
  ClearErrors
  ReadRegStr $2 HKCR ".svgz" ""
  IfErrors endUninstSVGZView
    ReadRegStr $3 HKCR "$2\shell\open\command" ""
    IfErrors 0 +2  
      DetailPrint "svgz viewer is $1"
    StrCmp $3 '"$INSTDIR\Inkscape.exe" "%1"' 0 +3
      DetailPrint "removing default .svgz viewer"
      DeleteRegKey HKCR "$2\shell\open\command"
    DeleteRegKey /ifempty HKCR "$2\shell\open"
    DeleteRegKey /ifempty HKCR "$2\shell"
    DeleteRegKey /ifempty HKCR "$2"
  endUninstSVGZView:
  
  ; Remove file associations for context menue
  DetailPrint "removing file associations for svg editor"
  ClearErrors
  ReadRegStr $0 HKCR ".svg" ""
  IfErrors endUninstSVGContext
  DetailPrint "removing default .svg context menue"
  DeleteRegKey HKCR "$0\shell\${PRODUCT_NAME}"
  DeleteRegKey /ifempty HKCR "$0\shell"
  DeleteRegKey /ifempty HKCR "$0"
  endUninstSVGContext:
  
  ClearErrors
  ReadRegStr $2 HKCR ".svgz" ""
  IfErrors endUninstSVGZContext
  DetailPrint "removing default .svgzcontext menue"
  DeleteRegKey HKCR "$2\shell\${PRODUCT_NAME}"
  DeleteRegKey /ifempty HKCR "$2\shell"
  DeleteRegKey /ifempty HKCR "$2"
  endUninstSVGZContext:

  ReadRegStr $1 HKCR "$0" ""
  StrCmp $1 "" 0 +3
    DetailPrint "removing filetype .svg $0"
    DeleteRegKey HKCR ".svg"
  
  ReadRegStr $3 HKCR "$2" ""
  StrCmp $3 "" 0 +3
    DetailPrint "removing filetype .svgz $2"
    DeleteRegKey HKCR ".svgz"
  
    
  SetShellVarContext all
  DetailPrint "removing product regkey"
  DeleteRegKey SHCTX "${PRODUCT_DIR_REGKEY}"
  DetailPrint "removing uninstall info"
  DeleteRegKey SHCTX "${PRODUCT_UNINST_KEY}"
  DetailPrint "removing shortcuts"
  Delete "$DESKTOP\Inkscape.lnk"
  Delete "$QUICKLAUNCH\Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape.lnk"
  ;just in case they are still there
  Delete "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape\Inkscape.lnk"
  RMDir  "$SMPROGRAMS\Inkscape"

  SetShellVarContext current
  DetailPrint "removing product regkey"
  DeleteRegKey SHCTX "${PRODUCT_DIR_REGKEY}"
  DetailPrint "removing uninstall info"
  DeleteRegKey SHCTX "${PRODUCT_UNINST_KEY}"
  DetailPrint "removing shortcuts"
  Delete "$DESKTOP\Inkscape.lnk"
  Delete "$QUICKLAUNCH\Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape.lnk"
  ;just in case they are still there
  Delete "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape\Inkscape.lnk"
  RMDir  "$SMPROGRAMS\Inkscape"

  DetailPrint "removing uninstall info"

  ;uninstall from path, must be repeated for every install logged path individual
  ;!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\lib\locale"
  ;!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\locale"
  ;!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\doc"
  ;!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\share\tutorials"
  ;!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\share\templates"
  ;!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\share\screens"
  ;!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\share\clipart"
  ;!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\share\extensions"
  ;!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\share\icons"
  ;!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\share"
  ;!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\modules"
  ;!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR\python"
  ;!insertmacro UNINSTALL.LOG_UNINSTALL "$INSTDIR"

  ;end uninstall, after uninstall from all logged paths has been performed
  ;!insertmacro UNINSTALL.LOG_END_UNINSTALL

  ;RMDir /r "$INSTDIR"
  
	StrCpy $removenever ""
	StrCpy $removealways ""
	
	InitPluginsDir
	SetPluginUnload manual
	
	ClearErrors
	FileOpen $0 $INSTDIR\uninstall.log r
	IfErrors uninstallnotfound
readnextline:  
	ClearErrors
	FileRead $0 $1
	IfErrors done
	; cat the line into md5 and filename
	StrLen $2 $1
	IntCmp $2 35 readnextline readnextline
	StrCpy $3 $1 32
	StrCpy $4 $1 $2-36 34	#remove trailing CR/LF
	StrCpy $4 $4 -2
	Push $3
	Push $4
	Call un.RemoveFile
	Goto readnextline
uninstallnotfound:
	MessageBox MB_OK|MB_ICONEXCLAMATION "$(lng_UninstallLogNotFound)" /SD IDOK
done:
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

SectionEnd


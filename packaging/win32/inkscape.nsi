; #######################################
; Inkscape NSIS installer project file
; Used as of 0.40
; #######################################

; #######################################
; DEFINES
; #######################################
!define PRODUCT_NAME "Inkscape"
!define PRODUCT_VERSION "0.43+0.44pre3"
!define PRODUCT_PUBLISHER "Inkscape Organization"
!define PRODUCT_WEB_SITE "http://www.inkscape.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\inkscape.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"



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
;!include "catalan.nsh" 
!include "czech.nsh" 
!include "french.nsh" 
!include "german.nsh" 
!include "italian.nsh" 
;!include "polish.nsh" 
!include "spanish.nsh" 

ReserveFile "inkscape.nsi.uninstall"


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
  File /nonfatal /a "..\..\inkscape\*.${lng}.txt"
  SetOutPath $INSTDIR\locale
  File /nonfatal /a /r "..\..\inkscape\locale\${polng}"
  SetOutPath $INSTDIR\lib\locale
  File /nonfatal /a /r "..\..\inkscape\lib\locale\${polng}"
  ; the keyboard tables
  SetOutPath $INSTDIR\share\screens
  File /nonfatal /a /r "..\..\inkscape\share\screens\keys.${polng}.svg"  
  SetOutPath $INSTDIR\share\templates
  File /nonfatal /a /r "..\..\inkscape\share\templates\default.${polng}.svg"  
  SetOutPath $INSTDIR\doc
  File /nonfatal /a /r "..\..\inkscape\doc\keys.${polng}.xml"  
  File /nonfatal /a /r "..\..\inkscape\doc\keys.${polng}.html"  
  SectionGetFlags ${SecTutorials} $R1 
  IntOp $R1 $R1 & ${SF_SELECTED} 
  IntCmp $R1 ${SF_SELECTED} 0 skip_tutorials 
    SetOutPath $INSTDIR\share\tutorials
    File /nonfatal /a "..\..\inkscape\share\tutorials\*.${polng}.*"
  skip_tutorials:
!macroend

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 




;--------------------------------
; Installer Sections

Section -removeInkscape
  ; check for an old installation and clean that dlls and stuff
  ClearErrors
  IfFileExists $INSTDIR\etc 0 doDeleteLib
    DetailPrint "$INSTDIR\etc exists, will be removed"
    RmDir /r $INSTDIR\etc
	IfErrors 0 +4
      DetailPrint "fatal: failed to delete $INSTDIR\etc"
      DetailPrint "aborting installation"
	  Abort
  doDeleteLib:

  ClearErrors
  IfFileExists $INSTDIR\lib 0 doDeleteLocale
    DetailPrint "$INSTDIR\lib exists, will be removed"  
    RmDir /r $INSTDIR\lib
	IfErrors 0 +4
      DetailPrint "fatal: failed to delete $INSTDIR\lib"
      DetailPrint "aborting installation"
	  Abort
  doDeleteLocale:

  ClearErrors
  IfFileExists $INSTDIR\locale 0 doDeleteDll
    DetailPrint "$INSTDIR\locale exists, will be removed"
    RmDir /r $INSTDIR\locale
	IfErrors 0 +4
      DetailPrint "fatal: failed to delete $INSTDIR\locale"
      DetailPrint "aborting installation"
	  Abort
  doDeleteDll:

  ClearErrors
  FindFirst $0 $1 $INSTDIR\*.dll
    FindNextLoop:
    StrCmp $1 "" FindNextDone
    DetailPrint "$INSTDIR\$1 exists, will be removed"
    Delete $INSTDIR\$1
    IfErrors 0 +4
      DetailPrint "fatal: failed to delete $INSTDIR\$1"
      DetailPrint "aborting installation"
      Abort
    FindNext $0 $1
    Goto FindNextLoop
  FindNextDone:
SectionEnd

Section $(lng_Core) SecCore

  DetailPrint "Installing Inkscape Core Files ..."

  SectionIn 1 2 3 RO
  SetOutPath $INSTDIR
  SetOverwrite on
  SetAutoClose false

  File /a "..\..\inkscape\ink*.exe"
  File /a "..\..\inkscape\AUTHORS"
  File /a "..\..\inkscape\COPYING"
  File /a "..\..\inkscape\COPYING.LIB"
  File /a "..\..\inkscape\NEWS"
  File /a "..\..\inkscape\HACKING.txt"
  File /a "..\..\inkscape\README"
  File /a "..\..\inkscape\README.txt"
  File /a "..\..\inkscape\TRANSLATORS"
  File /nonfatal /a /r "..\..\inkscape\data"
  File /nonfatal /a /r "..\..\inkscape\doc"
  File /nonfatal /a /r "..\..\inkscape\plugins"
  File /nonfatal /a /r /x *.??*.???* /x "examples" /x "tutorials" "..\..\inkscape\share"
  ; this files are added because it slips through the filter
  SetOutPath $INSTDIR\share\clipart
  File /a "..\..\inkscape\share\clipart\inkscape.logo.svg"
  ;File /a "..\..\inkscape\share\clipart\inkscape.logo.classic.svg"  
  SetOutPath $INSTDIR\share\extensions
  File /a "..\..\inkscape\share\extensions\pdf_output.inx.txt"
  File /a "..\..\inkscape\share\extensions\pdf_output_via_gs_on_win32.inx.txt"
  SetOutPath $INSTDIR\modules
  File /nonfatal /a /r "..\..\inkscape\modules\*.*"
  SetOutPath $INSTDIR\python
  File /nonfatal /a /r "..\..\inkscape\python\*.*"

  
SectionEnd

Section $(lng_GTKFiles) SecGTK

  DetailPrint "Installing GTK Files ..."
  
  SectionIn 1 2 3 RO
  SetOutPath $INSTDIR
  SetOverwrite on
  File /a /r "..\..\inkscape\*.dll"
  File /a /r /x "locale" "..\..\inkscape\lib"
  File /a /r "..\..\inkscape\etc"
SectionEnd

Section $(lng_Alluser) SecAlluser
  ; disable this option in Win95/Win98/WinME
  SectionIn 1 2 3 
  StrCpy $MultiUser "1"
SectionEnd

SectionGroup $(lng_Shortcuts) SecShortcuts

Section $(lng_Desktop) SecDesktop
  SectionIn 1 2 3
  ClearErrors
  CreateShortCut "$DESKTOP\Inkscape.lnk" "$INSTDIR\inkscape.exe"
  IfErrors 0 +2
    DetailPrint "Uups! Problems creating desktop shortcuts"
SectionEnd

Section $(lng_Quicklaunch) SecQuicklaunch
  SectionIn 1 2 3
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

SectionGroup $(lng_Addfiles) SecAddfiles

Section $(lng_Examples) SecExamples
  SectionIn 1 2
  SetOutPath $INSTDIR\share
  File /nonfatal /a /r /x "*.??*.???*" "..\..\inkscape\share\examples"
SectionEnd

Section $(lng_Tutorials) SecTutorials
  SectionIn 1 2
  SetOutPath $INSTDIR\share
  File /nonfatal /a /r /x "*.??*.???*" "..\..\inkscape\share\tutorials"
SectionEnd

SectionGroupEnd

SectionGroup /e $(lng_Languages) SecLanguages

Section $(lng_am) SecAmharic
  !insertmacro Language am am
SectionEnd

Section $(lng_az) SecAzerbaijani
  !insertmacro Language az az
SectionEnd

Section $(lng_be) SecByelorussian
  !insertmacro Language be be
SectionEnd

Section $(lng_ca) SecCatalan
  !insertmacro Language ca ca
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

Section $(lng_el) SecGreek
  !insertmacro Language el el
SectionEnd

Section $(lng_en) SecEnglish
  SectionIn 1 2 3 RO
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

Section $(lng_fr) SecFrench
  !insertmacro Language 'fr' 'fr'
SectionEnd

Section $(lng_ga) SecIrish
  !insertmacro Language ga ga
SectionEnd

Section $(lng_gl) SecGallegan
  !insertmacro Language gl gl
  SectionIn 1 2 3
SectionEnd

Section $(lng_hu) SecHungarian
  !insertmacro Language hu hu
  SectionIn 1 2 3
SectionEnd

Section $(lng_it) SecItalian
  !insertmacro Language it it
  SectionIn 1 2 3
SectionEnd

Section $(lng_ja) SecJapanese
  !insertmacro Language 'ja' 'jp'
SectionEnd

Section $(lng_ko) SecKorean
  !insertmacro Language 'ko' 'ko'
SectionEnd

Section $(lng_lt) SecLithuanian
  !insertmacro Language 'lt' 'lt'
SectionEnd

Section $(lng_mk) SecMacedonian
  !insertmacro Language mk mk
SectionEnd

Section $(lng_nb) SecNorwegianBokmal
  !insertmacro Language nb nb
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

Section $(lng_ru) SecRussian
  !insertmacro Language ru ru
SectionEnd

Section $(lng_sk) SecSlovak
  !insertmacro Language sk sk
SectionEnd

Section $(lng_sl) SecSlovenian
  !insertmacro Language sl sl
SectionEnd

Section $(lng_sr) SecSerbian
  !insertmacro Language sr sr
SectionEnd

Section $(lng_sr@Latn) SecSerbianLatin
  !insertmacro Language 'sr@Latn' 'sr@Latn'
SectionEnd

Section $(lng_sv) SecSwedish
  !insertmacro Language sv sv
SectionEnd

Section $(lng_tr) SecTurkish
  !insertmacro Language tr tr
SectionEnd

Section $(lng_uk) SecUkrainian
  !insertmacro Language uk uk
SectionEnd

Section $(lng_zh_CN) SecChineseSimplified
  !insertmacro Language zh_CN zh_CN
SectionEnd

Section $(lng_zh_TW) SecChineseTaiwan
  !insertmacro Language zh_TW zh_TW
SectionEnd

SectionGroupEnd


Section -FinalizeInstallation
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
  CreateDirectory "$SMPROGRAMS\Inkscape"
  CreateShortCut "$SMPROGRAMS\Inkscape\Inkscape.lnk" "$INSTDIR\inkscape.exe"
  CreateShortCut "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk" "$INSTDIR\uninst.exe"
  IfErrors 0 +2
    DetailPrint "fatal: failed to write to start menu info"

  ; uninstall settings
  ClearErrors
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegExpandStr SHCTX "${PRODUCT_UNINST_KEY}" "UninstallString" '"$INSTDIR\uninst.exe"'
  WriteRegExpandStr SHCTX "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Inkscape.exe,0"
  WriteRegStr SHCTX "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoModify" "1"
  WriteRegDWORD SHCTX "${PRODUCT_UNINST_KEY}" "NoRepair" "1"
  IfErrors 0 +2
    DetailPrint "fatal: failed to write to registry un-installation info"
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

	MessageBox MB_OK|MB_ICONEXCLAMATION "$(lng_NO_ADMIN)"

	Goto info_done

	info_Win9x:
		# This one means you don't need to care about admin or
		# not admin because Windows 9x doesn't either
		MessageBox MB_OK|MB_ICONEXCLAMATION $(lng_NOT_SUPPORTED)

	info_done:

  ;check for previous installation
  ReadRegStr $0 HKLM "${PRODUCT_DIR_REGKEY}" "User"
  StrCmp $0 "" +1 +2
  ReadRegStr $0 HKCU "${PRODUCT_DIR_REGKEY}" "User"
  ;check user if applicable
  StrCmp $0 "" +3
    StrCmp $0 $User +2
	  MessageBox MB_OK|MB_ICONEXCLAMATION "$(lng_DIFFERENT_USER)"
	
  ; proccess command line parameter
  !insertmacro Parameter "GTK" ${SecGTK}
  !insertmacro Parameter "SHORTCUTS" ${secShortcuts}
  !insertmacro Parameter "ALLUSER" ${SecAlluser}
  !insertmacro Parameter "DESKTOP" ${SecDesktop}
  !insertmacro Parameter "QUICKLAUNCH" ${SecQUICKlaunch}
  !insertmacro Parameter "SVGEDITOR" ${SecSVGWriter}
  !insertmacro Parameter "CONTEXTMENUE" ${SecContextMenu}
  !insertmacro Parameter "ADDFILES" ${SecAddfiles}
  !insertmacro Parameter "EXAMPLES" ${SecExamples}
  !insertmacro Parameter "TUTORIALS" ${SecTutorials}
  !insertmacro Parameter "LANGUAGES" ${SecLanguages}
  !insertmacro Parameter "am" ${SecAmharic}
  !insertmacro Parameter "az" ${SecAzerbaijani}
  !insertmacro Parameter "be" ${SecByelorussian}
  !insertmacro Parameter "ca" ${SecCatalan}
  !insertmacro Parameter "cs" ${SecCzech}
  !insertmacro Parameter "da" ${SecDanish}
  !insertmacro Parameter "de" ${SecGerman}
  !insertmacro Parameter "el" ${SecGreek}
  !insertmacro Parameter "es" ${SecSpanish}
  !insertmacro Parameter "es_MX" ${SecSpanishMexico}
  !insertmacro Parameter "et" ${SecEstonian}
  !insertmacro Parameter "fr" ${SecFrench}
  !insertmacro Parameter "ga" ${SecIrish}
  !insertmacro Parameter "gl" ${SecGallegan}
  !insertmacro Parameter "hu" ${SecHungarian}
  !insertmacro Parameter "it" ${SecItalian}
  !insertmacro Parameter "ja" ${SecJapanese}
  !insertmacro Parameter "ko" ${SecKorean}
  !insertmacro Parameter "mk" ${SecMacedonian}
  !insertmacro Parameter "nb" ${SecNorwegianBokmal}
  !insertmacro Parameter "nl" ${SecDutch}
  !insertmacro Parameter "nn" ${SecNorwegianNynorsk}
  !insertmacro Parameter "pa" ${SecPanjabi}
  !insertmacro Parameter "pl" ${SecPolish}
  !insertmacro Parameter "pt" ${SecPortuguese}
  !insertmacro Parameter "pt_BR" ${SecPortugueseBrazil}
  !insertmacro Parameter "ru" ${SecRussian}
  !insertmacro Parameter "sk" ${SecSlovak}
  !insertmacro Parameter "sl" ${SecSlovenian}
  !insertmacro Parameter "sr" ${SecSerbian}
  !insertmacro Parameter "sr@Latn" ${SecSerbianLatin}
  !insertmacro Parameter "sv" ${SecSwedish}
  !insertmacro Parameter "tr" ${SecTurkish}
  !insertmacro Parameter "uk" ${SecUkrainian}
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
      /ADDFILES=(OFF/ON): additional files$\r$\n \
      /EXAMPLES=(OFF/ON): examples$\r$\n \
      /TUTORIALS=(OFF/ON): tutorials$\r$\n \
      /LANGUAGES=(OFF/ON): translated menues, examples, etc.$\r$\n \
      /[locale code]=(OFF/ON): e.g am, es, es_MX as in Inkscape supported"
    Abort
FunctionEnd

Function .onSelChange
FunctionEnd

; --------------------------------------------------

Function un.CustomPageUninstall
  !insertmacro MUI_HEADER_TEXT "$(lng_UInstOpt)" "$(lng_UInstOpt1)"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.uninstall" "Field 1" "Text" "$APPDATA\Inkscape\preferences.xml"
  !insertmacro MUI_INSTALLOPTIONS_WRITE "inkscape.nsi.uninstall" "Field 2" "Text" "$(lng_PurgePrefs)"

  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "inkscape.nsi.uninstall"
  !insertmacro MUI_INSTALLOPTIONS_READ $MultiUser "inkscape.nsi.uninstall" "Field 2" "State"
  DetailPrint "keepfiles = $MultiUser" 
	  ;MessageBox MB_OK "adminmode = $MultiUser MultiUserOS = $askMultiUser" 

FunctionEnd


Function un.onInit

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
  StrCmp $0  "$INSTDIR\inkscape.exe" 0 +5  
    ReadRegStr $MultiUser HKLM "${PRODUCT_DIR_REGKEY}" "MultiUser"
    ReadRegStr $askMultiUser HKLM "${PRODUCT_DIR_REGKEY}" "askMultiUser"
	ReadRegStr $0 HKLM "${PRODUCT_DIR_REGKEY}" "User"
	Goto +4
  ReadRegStr $MultiUser HKCU "${PRODUCT_DIR_REGKEY}" "MultiUser"
  ReadRegStr $askMultiUser HKCU "${PRODUCT_DIR_REGKEY}" "askMultiUser"
  ReadRegStr $0 HKCU "${PRODUCT_DIR_REGKEY}" "User"
  ;check user if applicable
  StrCmp $0 "" +3
    StrCmp $0 $User +2
	  MessageBox MB_OK|MB_ICONEXCLAMATION "$(lng_DIFFERENT_USER)"
    
 !insertmacro MUI_INSTALLOPTIONS_EXTRACT "inkscape.nsi.uninstall"

 ;check whether Multi user installation ?
 SetShellVarContext all
 StrCmp $MultiUser "0" 0 +2 
 SetShellVarContext current
 ;MessageBox MB_OK "adminmode = $MultiUser MultiUserOS = $askMultiUser" 
   
FunctionEnd

Section Uninstall

  ; remove personal settings
  Delete "$APPDATA\Inkscape\extension-errors.log"
  StrCmp $MultiUser "0" 0 endPurge  ; multiuser assigned in dialog
    DetailPrint "purge personal settings in $APPDATA\Inkscape"
    RMDir /r "$APPDATA\Inkscape"
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
  Delete "$SMPROGRAMS\Inkscape\Uninstall Inkscape.lnk"
  Delete "$SMPROGRAMS\Inkscape\Inkscape.lnk"
  RMDir  "$SMPROGRAMS\Inkscape"

  DetailPrint "removing uninstall info"
  RMDir /r "$INSTDIR"

  SetAutoClose false

SectionEnd


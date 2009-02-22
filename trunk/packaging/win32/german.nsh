; #######################################
; german.nsh
; german language strings for inkscape installer
; windows code page: 1252
; Authors:
; Adib Taraben theAdib@googlemail.com
;
; 27 july 2006 new languages en_CA, en_GB, fi, hr, mn, ne, rw, sq
; 11 august 2006 new languages dz bg
; 24 october 2006 new languages en_US@piglatin, th
; 3rd December 2006 new languages eu km
; 14th December 2006 new lng_DeletePrefs, lng_DeletePrefsDesc, lng_WANT_UNINSTALL_BEFORE and lng_OK_CANCEL_DESC
; february 15 2007 new language bn, en_AU, eo, id, ro
; april 11 2007 new language he
; october 2007 new language ca@valencian
; January 2008 new uninstaller messages
; February 2008 new languages ar, br

!insertmacro MUI_LANGUAGE "German"

; Product name
LangString lng_Caption   ${LANG_GERMAN}  "${PRODUCT_NAME} -- Open Source SVG-Vektorillustrator"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_GERMAN} "Weiter >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_GERMAN} "$(^Name) wird unter der GNU General Public License (GPL) veröffentlicht. Die Lizenz dient hier nur der Information. $_CLICK"

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_GERMAN} "$R1 wurde bereits installiert. $\nSoll die vorhergehende Version vor dem Installieren von $(^Name) zuerst deinstalliert werden?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_GERMAN} "$\n$\nOK um fortzufahren oder Abbrechen zum sofortigen Beenden."

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_GERMAN} "Inkscape wurde durch den Benutzer $0 installiert.$\r$\nWenn Sie fortfahren kann die Aktion möglicherweise nicht korrekt abgeschlossen werden!$\r$\nBitte melden Sie sich als $0 an und versuchen Sie es erneut."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_GERMAN} "Sie sind nicht Computeradministrator.$\r$\nDas Installieren für alle Benutzer kann möglicherweise nicht korrekt abgeschlossen werden.$\r$\nBitte deselektieren Sie die Option 'für Alle Benutzer'."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_GERMAN} "Es ist bekannt, dass Inkscape unter Windows 95/98/ME nicht oder nicht stabil läuft!$\r$\nBitte prüfen Sie die offizielle Webseite für detaillierte Informationen."

; Full install type
LangString lng_Full $(LANG_GERMAN) "Komplett"

; Optimal install type
LangString lng_Optimal $(LANG_GERMAN) "Optimal"

; Minimal install type
LangString lng_Minimal $(LANG_GERMAN) "Minimal"


; Core section
LangString lng_Core $(LANG_GERMAN) "${PRODUCT_NAME} Vektorillustrator (erforderlich)"

; Core section description
LangString lng_CoreDesc $(LANG_GERMAN) "${PRODUCT_NAME} Basis-Dateien und -DLLs"


; GTK+ section
LangString lng_GTKFiles $(LANG_GERMAN) "GTK+ Runtime Umgebung (erforderlich)"

; GTK+ section description
LangString lng_GTKFilesDesc $(LANG_GERMAN) "Ein Multi-Plattform GUI Toolkit, verwendet von ${PRODUCT_NAME}"


; shortcuts section
LangString lng_Shortcuts $(LANG_GERMAN) "Verknüpfungen"

; shortcuts section description
LangString lng_ShortcutsDesc $(LANG_GERMAN) "Verknüpfungen zum Start von ${PRODUCT_NAME}"

; multi user installation
LangString lng_Alluser  ${LANG_GERMAN}  "für Alle Benutzer"

; multi user installation description
LangString lng_AlluserDesc  ${LANG_GERMAN}  "Installiert diese Anwendung für alle Benutzer dieses Computers (all users)"

; Start Menu  section
LangString lng_Startmenu $(LANG_GERMAN) "Startmenü"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_GERMAN) "Erstellt einen Eintrag für ${PRODUCT_NAME} im Startmenü"

; Desktop section
LangString lng_Desktop $(LANG_GERMAN) "Desktop"

; Desktop section description
LangString lng_DesktopDesc $(LANG_GERMAN) "Erstellt eine Verknüpfung zu ${PRODUCT_NAME} auf dem Desktop"

; Quick launch section
LangString lng_Quicklaunch $(LANG_GERMAN) "Schnellstartleiste"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_GERMAN) "Erstellt eine Verknüpfung zu ${PRODUCT_NAME} auf der Schnellstartleiste"

; File type association for editing
LangString lng_SVGWriter    ${LANG_GERMAN}  "Öffne SVG Dateien mit ${PRODUCT_NAME}"

;LangString lng_UseAs ${LANG_GERMAN} "Select ${PRODUCT_NAME} as default application for:"
LangString lng_SVGWriterDesc    ${LANG_GERMAN}  "Wählen Sie ${PRODUCT_NAME} als Standardanwendung für SVG Dateien"

; Context Menu
LangString lng_ContextMenu ${LANG_GERMAN} "Kontext-Menü"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_GERMAN} "Fügt ${PRODUCT_NAME} in das Kontext-Menü für SVG Dateien ein"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_GERMAN} "Persönliche Inkscape-Vorgaben löschen"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_GERMAN} "Löscht verbliebene persönliche Inkscape-Vorgaben einer vorhergehenden Version"


; Additional Files section
LangString lng_Addfiles $(LANG_GERMAN) "weitere Dateien"

; additional files section dscription
LangString lng_AddfilesDesc $(LANG_GERMAN) "weitere Dateien"

; Examples section
LangString lng_Examples $(LANG_GERMAN) "Beispiele"

; Examples section dscription
LangString lng_ExamplesDesc $(LANG_GERMAN) "Beispiele mit ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_GERMAN) "Tutorials"

; Tutorials section dscription
LangString lng_TutorialsDesc $(LANG_GERMAN) "Tutorials für die Benutzung mit ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_GERMAN) "Übersetzungen"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_GERMAN) "Installiert verschiedene Übersetzungen für ${PRODUCT_NAME}"

LangString lng_am $(LANG_GERMAN) "am  Amharisch"
LangString lng_ar $(LANG_GERMAN) "ar  Arabisch"
LangString lng_az $(LANG_GERMAN) "az  Aserbaidschanisch"
LangString lng_be $(LANG_GERMAN) "be  Weißrussisch"
LangString lng_bg $(LANG_GERMAN) "bg  Bulgarisch"
LangString lng_bn $(LANG_GERMAN) "bn  Bengalisch"
LangString lng_br $(LANG_GERMAN) "br  Bretonisch"
LangString lng_ca $(LANG_GERMAN) "ca  Katalanisch"
LangString lng_ca@valencia $(LANG_GERMAN) "ca@valencia  Valencianisch Katalanisch"
LangString lng_cs $(LANG_GERMAN) "cs  Tschechisch"
LangString lng_da $(LANG_GERMAN) "da  Dänisch"
LangString lng_de $(LANG_GERMAN) "de  Deutsch"
LangString lng_dz $(LANG_GERMAN) "dz  Dzongkha"
LangString lng_el $(LANG_GERMAN) "el  Griechisch"
LangString lng_en $(LANG_GERMAN) "en  Englisch"
LangString lng_en_AU $(LANG_GERMAN) "en_AU Australisch Englisch"
LangString lng_en_CA $(LANG_GERMAN) "en_CA  Kanadisch Englisch"
LangString lng_en_GB $(LANG_GERMAN) "en_GB  Britisch Englisch"
LangString lng_en_US@piglatin $(LANG_GERMAN) "en_US@piglatin Pig Latin"
LangString lng_eo $(LANG_GERMAN) "eo  Esperanto"
LangString lng_es $(LANG_GERMAN) "es  Spanisch"
LangString lng_es_MX $(LANG_GERMAN) "es_MX  Spanisch-Mexio"
LangString lng_et $(LANG_GERMAN) "et  Estonisch"
LangString lng_eu $(LANG_GERMAN) "eu  Baskisch"
LangString lng_fi $(LANG_GERMAN) "fi  Finnisch"
LangString lng_fr $(LANG_GERMAN) "fr  Französisch"
LangString lng_ga $(LANG_GERMAN) "ga  Irisch"
LangString lng_gl $(LANG_GERMAN) "gl  Galizisch"
LangString lng_he $(LANG_GERMAN) "he  Hebräisch"
LangString lng_hr $(LANG_GERMAN) "hr  Kroatisch"
LangString lng_hu $(LANG_GERMAN) "hu  Ungarisch"
LangString lng_id $(LANG_GERMAN) "id  Indonesian"
LangString lng_it $(LANG_GERMAN) "it  Italienisch"
LangString lng_ja $(LANG_GERMAN) "ja  Japanisch"
LangString lng_km $(LANG_GERMAN) "km  Khmer"
LangString lng_ko $(LANG_GERMAN) "ko  Koreanisch"
LangString lng_lt $(LANG_GERMAN) "lt  Litauisch"
LangString lng_mn $(LANG_GERMAN) "mn  Mongolisch"
LangString lng_mk $(LANG_GERMAN) "mk  Mazedonisch"
LangString lng_ne $(LANG_GERMAN) "ne  Nepalisch"
LangString lng_nb $(LANG_GERMAN) "nb  Norwegisch-Bokmal"
LangString lng_nl $(LANG_GERMAN) "nl  Holländisch"
LangString lng_nn $(LANG_GERMAN) "nn  Nynorsk-Norwegisch"
LangString lng_pa $(LANG_GERMAN) "pa  Panjabi"
LangString lng_pl $(LANG_GERMAN) "po  Polnisch"
LangString lng_pt $(LANG_GERMAN) "pt  Portugiesisch"
LangString lng_pt_BR $(LANG_GERMAN) "pt_BR  Portugiesisch Brazilien"
LangString lng_ro $(LANG_GERMAN) "ro  Rumänisch"
LangString lng_ru $(LANG_GERMAN) "ru  Russisch"
LangString lng_rw $(LANG_GERMAN) "rw  Kinyarwanda"
LangString lng_sk $(LANG_GERMAN) "sk  Slowakisch"
LangString lng_sl $(LANG_GERMAN) "sl  Slowenisch"
LangString lng_sq $(LANG_GERMAN) "sq  Albanisch"
LangString lng_sr $(LANG_GERMAN) "sr  Serbisch"
LangString lng_sr@latin $(LANG_GERMAN) "sr@latin Serbisch mit lat. Buchstaben"
LangString lng_sv $(LANG_GERMAN) "sv  Schwedisch"
LangString lng_th $(LANG_GERMAN) "th  Thai"
LangString lng_tr $(LANG_GERMAN) "tr  Türkisch"
LangString lng_uk $(LANG_GERMAN) "uk  Ukrainisch"
LangString lng_vi $(LANG_GERMAN) "vi  Vietnamesisch"
LangString lng_zh_CN $(LANG_GERMAN) "zh_CH  Chinesisch (vereinfacht)"
LangString lng_zh_TW $(LANG_GERMAN) "zh_TW  Chinesisch (traditionell)"


; uninstallation options
;LangString lng_UInstOpt   ${LANG_GERMAN} "Uninstallation Options"
LangString lng_UInstOpt   ${LANG_GERMAN} "Deinstallations Optionen"

; uninstallation options subtitle
;LangString lng_UInstOpt1  ${LANG_GERMAN} "Please make your choices for additional options"
LangString lng_UInstOpt1  ${LANG_GERMAN} "Bitte wählen Sie die optionalen Deinstalltionsparameter"

; Ask to purge the personal preferences
;LangString lng_PurgePrefs ${LANG_GERMAN} "Keep Inkscape preferences"
LangString lng_PurgePrefs ${LANG_GERMAN}  "behalte persönliche Inkscape-Vorgaben"

LangString lng_RETRY_CANCEL_DESC ${LANG_GERMAN} "$\n$\nWiederholen um fortzufahren oder Abbrechen zum sofortigen Beenden."

LangString lng_ClearDirectoryBefore ${LANG_GERMAN} "${PRODUCT_NAME} kann nur in einem leeren Verzeichnis installiert werden. $INSTDIR ist nicht leer. Bitte löschen Sie den verzeichnisinhalt und versuchen Sie es erneut!$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_GERMAN} "Datei $INSTDIR\uninstall.log nicht gefunden!$\r$\nBitte deinstallieren Sie selbst durch Löschen von $INSTDIR!"

LangString lng_FileChanged ${LANG_GERMAN} "Die Datei $filename wurde nach der Installation geändert.$\r$\nMöchten Sie trotzden diese Datei löschen?"

LangString lng_Yes ${LANG_GERMAN} "Ja"

LangString lng_AlwaysYes ${LANG_GERMAN} "immer mit Ja antworten"

LangString lng_No ${LANG_GERMAN} "Nein"

LangString lng_AlwaysNo ${LANG_GERMAN} "immer mit Nein antworten"

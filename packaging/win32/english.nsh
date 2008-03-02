
; #######################################
; english.nsh
; english language strings for inkscape installer
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

!insertmacro MUI_LANGUAGE "English"

; Product name
LangString lng_Caption   ${LANG_ENGLISH} "${PRODUCT_NAME} -- Open Source Scalable Vector Graphics Editor"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_ENGLISH} "Next >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_ENGLISH} "$(^Name) is released under the GNU General Public License (GPL). The license is provided here for information purposes only. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_ENGLISH} "Inkscape has been installed by user $0.$\r$\nIf you continue you might not complete successfully!$\r$\nPlease log in as $0 and try again."

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_ENGLISH} "$R1 has already been installed. $\nDo you want to remove the previous version before installing $(^Name) ?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_ENGLISH} "$\n$\nPress OK to continue or press CANCEL to abort."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_ENGLISH} "You do not have administrator privileges.$\r$\nInstalling Inkscape for all users might not complete successfully.$\r$\nUncheck the 'for all users' option."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_ENGLISH} "Inkscape is known not to run under Windows 95/98/ME!$\r$\nPlease check the official website for detailed information."

; Full install type
LangString lng_Full $(LANG_ENGLISH) "Full"

; Optimal install type
LangString lng_Optimal $(LANG_ENGLISH) "Optimal"

; Minimal install type
LangString lng_Minimal $(LANG_ENGLISH) "Minimal"

; Core install section
LangString lng_Core $(LANG_ENGLISH) "${PRODUCT_NAME} SVG Editor (required)"

; Core install section description
LangString lng_CoreDesc $(LANG_ENGLISH) "Core ${PRODUCT_NAME} files and dlls"

; GTK+ install section
LangString lng_GTKFiles $(LANG_ENGLISH) "GTK+ Runtime Environment (required)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_ENGLISH) "A multi-platform GUI toolkit, used by ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_ENGLISH) "Shortcuts"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_ENGLISH) "Shortcuts for starting ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_ENGLISH) "for all users"

; All user install section description
LangString lng_AlluserDesc $(LANG_ENGLISH) "Install this application for anyone who uses this computer (all users)"

; Desktop section
LangString lng_Desktop $(LANG_ENGLISH) "Desktop"

; Desktop section description
LangString lng_DesktopDesc $(LANG_ENGLISH) "Create a shortcut to ${PRODUCT_NAME} on the Desktop"

; Start Menu  section
LangString lng_Startmenu $(LANG_ENGLISH) "Start Menu"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_ENGLISH) "Create a Start Menu entry for ${PRODUCT_NAME}"

; Quick launch section
LangString lng_Quicklaunch $(LANG_ENGLISH) "Quick Launch"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_ENGLISH) "Create a shortcut to ${PRODUCT_NAME} on the Quick Launch toolbar"

; File type association for editing
LangString lng_SVGWriter ${LANG_ENGLISH} "Open SVG files with ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_ENGLISH} "Select ${PRODUCT_NAME} as default editor for SVG files"

; Context Menu
LangString lng_ContextMenu ${LANG_ENGLISH} "Context Menu"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_ENGLISH} "Add ${PRODUCT_NAME} into the Context Menu for SVG files"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_ENGLISH} "Delete personal preferences"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_ENGLISH} "Delete personal preferences leftover from previous installations"


; Additional files section
LangString lng_Addfiles $(LANG_ENGLISH) "Additional Files"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_ENGLISH) "Additional Files"

; Examples section
LangString lng_Examples $(LANG_ENGLISH) "Examples"

; Examples section description
LangString lng_ExamplesDesc $(LANG_ENGLISH) "Examples using ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_ENGLISH) "Tutorials"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_ENGLISH) "Tutorials using ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_ENGLISH) "Translations"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_ENGLISH) "Install various translations for ${PRODUCT_NAME}"

LangString lng_am $(LANG_ENGLISH) "am  Amharic"
LangString lng_ar $(LANG_ENGLISH) "ar  Arabic"
LangString lng_az $(LANG_ENGLISH) "az  Azerbaijani"
LangString lng_be $(LANG_ENGLISH) "be  Byelorussian"
LangString lng_bg $(LANG_ENGLISH) "bg  Bulgarian"
LangString lng_bn $(LANG_ENGLISH) "bn  Bengali"
LangString lng_br $(LANG_ENGLISH) "br  Breton"
LangString lng_ca $(LANG_ENGLISH) "ca  Catalan"
LangString lng_ca@valencia $(LANG_ENGLISH) "ca@valencia  Valencian Catalan"
LangString lng_cs $(LANG_ENGLISH) "cs  Czech"
LangString lng_da $(LANG_ENGLISH) "da  Danish"
LangString lng_de $(LANG_ENGLISH) "de  German"
LangString lng_dz $(LANG_ENGLISH) "dz  Dzongkha"
LangString lng_el $(LANG_ENGLISH) "el  Greek"
LangString lng_en $(LANG_ENGLISH) "en  English"
LangString lng_en_AU $(LANG_ENGLISH) "en_AU Australian English"
LangString lng_en_CA $(LANG_ENGLISH) "en_CA Canadian English"
LangString lng_en_GB $(LANG_ENGLISH) "en_GB British English"
LangString lng_en_US@piglatin $(LANG_ENGLISH) "en_US@piglatin Pig Latin"
LangString lng_eo $(LANG_ENGLISH) "eo  Esperanto"
LangString lng_es $(LANG_ENGLISH) "es  Spanish"
LangString lng_es_MX $(LANG_ENGLISH) "es_MX  Mexican Spanish"
LangString lng_et $(LANG_ENGLISH) "et  Estonian"
LangString lng_eu $(LANG_ENGLISH) "eu  Basque"
LangString lng_fi $(LANG_ENGLISH) "fi  Finnish"
LangString lng_fr $(LANG_ENGLISH) "fr  French"
LangString lng_ga $(LANG_ENGLISH) "ga  Irish"
LangString lng_gl $(LANG_ENGLISH) "gl  Gallegan"
LangString lng_he $(LANG_ENGLISH) "he  Hebrew"
LangString lng_hr $(LANG_ENGLISH) "hr  Croatian"
LangString lng_hu $(LANG_ENGLISH) "hu  Hungarian"
LangString lng_id $(LANG_ENGLISH) "id  Indonesian"
LangString lng_it $(LANG_ENGLISH) "it  Italian"
LangString lng_ja $(LANG_ENGLISH) "ja  Japanese"
LangString lng_km $(LANG_ENGLISH) "km  Khmer"
LangString lng_ko $(LANG_ENGLISH) "ko  Korean"
LangString lng_lt $(LANG_ENGLISH) "lt  Lithuanian"
LangString lng_mk $(LANG_ENGLISH) "mk  Macedonian"
LangString lng_mn $(LANG_ENGLISH) "mn  Mongolian"
LangString lng_ne $(LANG_ENGLISH) "ne  Nepali"
LangString lng_nb $(LANG_ENGLISH) "nb  Norwegian Bokmål"
LangString lng_nl $(LANG_ENGLISH) "nl  Dutch"
LangString lng_nn $(LANG_ENGLISH) "nn  Norwegian Nynorsk"
LangString lng_pa $(LANG_ENGLISH) "pa  Panjabi"
LangString lng_pl $(LANG_ENGLISH) "po  Polish"
LangString lng_pt $(LANG_ENGLISH) "pt  Portuguese"
LangString lng_pt_BR $(LANG_ENGLISH) "pt_BR Brazilian Portuguese"
LangString lng_ro $(LANG_ENGLISH) "ro  Romanian"
LangString lng_ru $(LANG_ENGLISH) "ru  Russian"
LangString lng_rw $(LANG_ENGLISH) "rw  Kinyarwanda"
LangString lng_sk $(LANG_ENGLISH) "sk  Slovak"
LangString lng_sl $(LANG_ENGLISH) "sl  Slovenian"
LangString lng_sq $(LANG_ENGLISH) "sq  Albanian"
LangString lng_sr $(LANG_ENGLISH) "sr  Serbian"
LangString lng_sr@latin $(LANG_ENGLISH) "sr@latin  Serbian in Latin script"
LangString lng_sv $(LANG_ENGLISH) "sv  Swedish"
LangString lng_th $(LANG_ENGLISH) "th  Thai"
LangString lng_tr $(LANG_ENGLISH) "tr  Turkish"
LangString lng_uk $(LANG_ENGLISH) "uk  Ukrainian"
LangString lng_vi $(LANG_ENGLISH) "vi  Vietnamese"
LangString lng_zh_CN $(LANG_ENGLISH) "zh_CH  Simplifed Chinese"
LangString lng_zh_TW $(LANG_ENGLISH) "zh_TW  Traditional Chinese"




; uninstallation options
LangString lng_UInstOpt   ${LANG_ENGLISH} "Uninstallation Options"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_ENGLISH} "Please make your choices for additional options"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_ENGLISH} "Keep personal Preferences"

LangString lng_RETRY_CANCEL_DESC ${LANG_ENGLISH} "$\n$\nPress RETRY to continue or press CANCEL to abort."

LangString lng_ClearDirectoryBefore ${LANG_ENGLISH} "${PRODUCT_NAME} must be installed in an empty directory. $INSTDIR is not empty. Please clear this directory first!$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_ENGLISH} "$INSTDIR\uninstall.log not found!$\r$\nPlease uninstall by clearing directory $INSTDIR yourself!"

LangString lng_FileChanged ${LANG_ENGLISH} "The file $filename has been changed after installation.$\r$\nDo you still want to delete that file?"

LangString lng_Yes ${LANG_ENGLISH} "Yes"

LangString lng_AlwaysYes ${LANG_ENGLISH} "always answer Yes"

LangString lng_No ${LANG_ENGLISH} "No"

LangString lng_AlwaysNo ${LANG_ENGLISH} "always answer No"

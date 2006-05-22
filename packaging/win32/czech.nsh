; #######################################
; czechh.nsh
; czechh language strings for inkscape installer
; windows code page: 1029
; Authors:
; Michal Kraus Michal.Kraus@wige-mic.cz
;
!insertmacro MUI_LANGUAGE "Czech"

; Product name
LangString lng_Caption   ${LANG_CZECH} "${PRODUCT_NAME} -- Open Source Scalable Vector Graphics Editor"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_CZECH} "Další >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_CZECH} "$(^Name) je vydáván pod General Public License (GPL). Licenèní ujednání je zde pouze z informaèních dùvodù. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_CZECH} "Inkscape has been installed by user $0.  If you continue you might not complete successfully!  Please log in as $0 and try again."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_CZECH} "You do not have administrator privileges.  Installing Inkscape for all users might not complete successfully.  Uncheck option 'For All Users'."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_CZECH} "Inkscape is known not to run under Windows 95/98/ME!\n\nPlease check the official website for detailed information."

; Full install type
LangString lng_Full $(LANG_CZECH) "Plná"

; Optimal install type
LangString lng_Optimal $(LANG_CZECH) "Optimální"

; Minimal install type
LangString lng_Minimal $(LANG_CZECH) "Minimální"

; Core install section
LangString lng_Core $(LANG_CZECH) "${PRODUCT_NAME} SVG editor (nutné)"

; Core install section description
LangString lng_CoreDesc $(LANG_CZECH) "Core ${PRODUCT_NAME} soubory a dll"

; GTK+ install section
LangString lng_GTKFiles $(LANG_CZECH) "GTK+ bìhové prostøedí (nutné)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_CZECH) "Multiplatformní sada uživatelského rozhraní, použitého v ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_CZECH) "Zástupci"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_CZECH) "Zástupci pro start ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_CZECH) "pro všechny uživatele"

; All user install section description
LangString lng_AlluserDesc $(LANG_CZECH) "Instalovat aplikaci pro kohokoliv, kdo používá tento poèítaè.(všichni uživatelé)"

; Desktop section
LangString lng_Desktop $(LANG_CZECH) "Desktop"

; Desktop section description
LangString lng_DesktopDesc $(LANG_CZECH) "Vztvoøit zástupce ${PRODUCT_NAME} na ploše"

; Start Menu  section
LangString lng_Startmenu $(LANG_CZECH) "Startovní nabídka"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_CZECH) "Vytvoøit pro ${PRODUCT_NAME} položku ve startovní nabídce"

; Quick launch section
LangString lng_Quicklaunch $(LANG_CZECH) "Panel rychlého spuštìní"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_CZECH) "Vytvoøit pro ${PRODUCT_NAME} zástupce na panelu rychlého spuštìní"

; File type association for editing
LangString lng_SVGWriter ${LANG_CZECH} "Otvírat SVG soubory v ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_CZECH} "Vybrat ${PRODUCT_NAME} jako výchozí editor pro SVG soubory"

; Context Menu
LangString lng_ContextMenu ${LANG_CZECH} "Kontextová nabídka"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_CZECH} "Pøidat ${PRODUCT_NAME} do kontextové nabídky pro SVG soubory"


; Additional files section
LangString lng_Addfiles $(LANG_CZECH) "Další soubory"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_CZECH) "Další soubory"

; Examples section
LangString lng_Examples $(LANG_CZECH) "Pøíklady"

; Examples section description
LangString lng_ExamplesDesc $(LANG_CZECH) "Pøílady používání ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_CZECH) "Prùvodci"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_CZECH) "Prùvodci funkcemi ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_CZECH) "Jazykové sady"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_CZECH) "Nainstalovat další jazykové sady ${PRODUCT_NAME}"

LangString lng_am $(LANG_CZECH) "am  Amharic"
LangString lng_az $(LANG_CZECH) "az  Azerbaijani"
LangString lng_be $(LANG_CZECH) "be  Byelorussian"
LangString lng_ca $(LANG_CZECH) "ca  Catalan"
LangString lng_cs $(LANG_CZECH) "cs  Czech"
LangString lng_da $(LANG_CZECH) "da  Danish"
LangString lng_de $(LANG_CZECH) "de  German"
LangString lng_el $(LANG_CZECH) "el  Greek"
LangString lng_en $(LANG_CZECH) "en  English"
LangString lng_es $(LANG_CZECH) "es  Spanish"
LangString lng_es_MX $(LANG_CZECH) "es_MX  Mexican Spanish"
LangString lng_et $(LANG_CZECH) "es  Estonian"
LangString lng_fr $(LANG_CZECH) "fr  French"
LangString lng_ga $(LANG_CZECH) "ga  Irish"
LangString lng_gl $(LANG_CZECH) "gl  Gallegan"
LangString lng_hu $(LANG_CZECH) "hu  Hungarian"
LangString lng_it $(LANG_CZECH) "it  Italian"
LangString lng_ja $(LANG_CZECH) "ja  Japanese"
LangString lng_ko $(LANG_CZECH) "ko  Korean"
LangString lng_lt $(LANG_CZECH) "lt  Lithuanian"
LangString lng_mk $(LANG_CZECH) "mk  Macedonian"
LangString lng_nb $(LANG_CZECH) "nb  Norwegian Bokmal"
LangString lng_nl $(LANG_CZECH) "nl  Dutch"
LangString lng_nn $(LANG_CZECH) "nn  Norwegian Nynorsk"
LangString lng_pa $(LANG_CZECH) "pa  Panjabi"
LangString lng_pl $(LANG_CZECH) "po  Polish"
LangString lng_pt $(LANG_CZECH) "pt  Portuguese"
LangString lng_pt_BR $(LANG_CZECH) "pt_BR Brazilian Portuguese"
LangString lng_ru $(LANG_CZECH) "ru  Russian"
LangString lng_sk $(LANG_CZECH) "sk  Slovak"
LangString lng_sl $(LANG_CZECH) "sl  Slovenian"
LangString lng_sr $(LANG_CZECH) "sr  Serbian"
LangString lng_sr@Latn $(LANG_CZECH) "sr@Latn  Serbian in Latin script"
LangString lng_sv $(LANG_CZECH) "sv  Swedish"
LangString lng_tr $(LANG_CZECH) "tr  Turkish"
LangString lng_uk $(LANG_CZECH) "uk  Ukrainian"
LangString lng_zh_CN $(LANG_CZECH) "zh_CH  Simplifed Chinese"
LangString lng_zh_TW $(LANG_CZECH) "zh_TW  Traditional Chinese"




; uninstallation options
LangString lng_UInstOpt   ${LANG_CZECH} "Volby odinstalace"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_CZECH} "Vyberte prosím další nastavení"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_CZECH} "Ponechat osobní nastavení"
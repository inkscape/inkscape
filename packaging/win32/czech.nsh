; #######################################
; czech.nsh
; czech language strings for inkscape installer
; windows code page: 1250
; Authors:
; Michal Kraus Michal.Kraus@wige-mic.cz (original translation)
; Josef VybÌral josef.vybiral@gmail.com (update for Inkscape 0.44)
;
!insertmacro MUI_LANGUAGE "Czech"

; Product name
LangString lng_Caption   ${LANG_CZECH} "${PRODUCT_NAME} -- Open Source Scalable Vector Graphics Editor"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_CZECH} "DalöÌ >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_CZECH} "$(^Name) je vyd·v·n pod General Public License (GPL). LicenËnÌ ujedn·nÌ je zde pouze z informaËnÌch d˘vod˘. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_CZECH} "Inkscape byl nainstalov·n uûivatelem $0.$\r$\nInstalace nemusÌ b˝t dokonËena spr·vnÏ pokud v nÌ budete pokraËovat!$\r$\nProsÌm p¯ihlaste se jako $0 a spusùte instalaci znovu."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_CZECH} "Nem·te administr·torsk· opr·vnÏnÌ.$\r$\nInstalace Inkscape pro vöechny uûivatele nemusÌ b˝t dokonËena ˙spÏönÏ.$\r$\nZruöte oznaËenÌ volby 'Pro vöechny uûivatele'."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_CZECH} "Inkscape nebÏûÌ na Windows 95/98/ME!$\r$\nPro podrobnÏjöÌ informace se prosÌm obraùte na ofici·lnÌ webovÈ str·nky."

; Full install type
LangString lng_Full $(LANG_CZECH) "Pln·"

; Optimal install type
LangString lng_Optimal $(LANG_CZECH) "Optim·lnÌ"

; Minimal install type
LangString lng_Minimal $(LANG_CZECH) "Minim·lnÌ"

; Core install section
LangString lng_Core $(LANG_CZECH) "${PRODUCT_NAME} SVG editor (nutnÈ)"

; Core install section description
LangString lng_CoreDesc $(LANG_CZECH) "Soubory a knihovny ${PRODUCT_NAME}"

; GTK+ install section
LangString lng_GTKFiles $(LANG_CZECH) "GTK+ bÏhovÈ prost¯edÌ (nutnÈ)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_CZECH) "MultiplatformnÌ sada uûivatelskÈho rozhranÌ, pouûitÈho v ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_CZECH) "Z·stupci"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_CZECH) "Z·stupci pro start ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_CZECH) "pro vöechny uûivatele"

; All user install section description
LangString lng_AlluserDesc $(LANG_CZECH) "Instalovat aplikaci pro kohokoliv, kdo pouûÌv· tento poËÌtaË.(vöichni uûivatelÈ)"

; Desktop section
LangString lng_Desktop $(LANG_CZECH) "Plocha"

; Desktop section description
LangString lng_DesktopDesc $(LANG_CZECH) "Vytvo¯it z·stupce ${PRODUCT_NAME} na ploöe"

; Start Menu  section
LangString lng_Startmenu $(LANG_CZECH) "NabÌdka Start"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_CZECH) "Vytvo¯it pro ${PRODUCT_NAME} poloûku ve startovnÌ nabÌdce"

; Quick launch section
LangString lng_Quicklaunch $(LANG_CZECH) "Panel rychlÈho spuötÏnÌ"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_CZECH) "Vytvo¯it pro ${PRODUCT_NAME} z·stupce na panelu rychlÈho spuötÏnÌ"

; File type association for editing
LangString lng_SVGWriter ${LANG_CZECH} "OtvÌrat SVG soubory v ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_CZECH} "Vybrat ${PRODUCT_NAME} jako v˝chozÌ editor pro SVG soubory"

; Context Menu
LangString lng_ContextMenu ${LANG_CZECH} "Kontextov· nabÌdka"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_CZECH} "P¯idat ${PRODUCT_NAME} do kontextovÈ nabÌdky pro SVG soubory"


; Additional files section
LangString lng_Addfiles $(LANG_CZECH) "DalöÌ soubory"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_CZECH) "DalöÌ soubory"

; Examples section
LangString lng_Examples $(LANG_CZECH) "P¯Ìklady"

; Examples section description
LangString lng_ExamplesDesc $(LANG_CZECH) "P¯Ìlady pouûÌv·nÌ ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_CZECH) "Pr˘vodci"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_CZECH) "Pr˘vodci funkcemi ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_CZECH) "JazykovÈ sady"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_CZECH) "Nainstalovat dalöÌ jazykovÈ sady ${PRODUCT_NAME}"

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
LangString lng_UInstOpt1  ${LANG_CZECH} "Vyberte prosÌm dalöÌ nastavenÌ"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_CZECH} "Ponechat osobnÌ nastavenÌ"

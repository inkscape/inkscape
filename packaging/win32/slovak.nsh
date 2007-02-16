; #######################################
; slovak.nsh
; slovak language strings for inkscape installer
; windows code page: 1250
; Authors:
; helix84 helix84@gmail.com (translation for Inkscape 0.44)
;
; 27 july 2006 new languages en_CA, en_GB, fi, hr, mn, ne, rw, sq
; 11 august 2006 new languages dz bg
; 24 october 2006 new languages en_US@piglatin, th
; 3rd December 2006 new languages eu km
; 14th December 2006 new lng_DeletePrefs, lng_DeletePrefsDesc, lng_WANT_UNINSTALL_BEFORE and lng_OK_CANCEL_DESC
; february 15 2007 new language bn, en_AU, eo, id, ro

!insertmacro MUI_LANGUAGE "Slovak"

; Product name
LangString lng_Caption   ${LANG_SLOVAK} "${PRODUCT_NAME} -- Open Source Scalable Vector Graphics Editor"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_SLOVAK} "œalej >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_SLOVAK} "$(^Name) je moûnÈ öÌriù za podmienok General Public License (GPL). LicenËn· zmluva je tu len pre informaËnÈ ˙Ëely. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_SLOVAK} "Inkscape nainötaloval pouûÌvateæ $0.$\r$\nInötal·cia nemusÌ spr·vne skonËiù, ak v nej budete pokraËovaù!$\r$\nProsÌm, prihl·ste sa ako $0 a spustite inötal·ciu znova."

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_SLOVAK} "$R1 has already been installed. $\nDo you want to remove the previous version before installing $(^Name) ?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_SLOVAK} "$\n$\nPress OK to continue or press CANCEL to abort."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_SLOVAK} "Nem·te administr·torskÈ pr·va.$\r$\nInötal·cia Inkscape pre vöetk˝ch pouûÌvateæov nemusÌ skonËiù ˙speöne.$\r$\nZruöte oznaËenie voæby 'Pre vöetk˝ch pouûÌvateæov'."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_SLOVAK} "Inkscape nebÏûÌ na Windows 95/98/ME!$\r$\nPodrobnejöie inform·cie n·jdete na ofici·lnom webe."

; Full install type
LangString lng_Full $(LANG_SLOVAK) "Pln·"

; Optimal install type
LangString lng_Optimal $(LANG_SLOVAK) "Optim·lna"

; Minimal install type
LangString lng_Minimal $(LANG_SLOVAK) "Minim·lna"

; Core install section
LangString lng_Core $(LANG_SLOVAK) "${PRODUCT_NAME} SVG editor (povinnÈ)"

; Core install section description
LangString lng_CoreDesc $(LANG_SLOVAK) "S˙bory a kniûnice ${PRODUCT_NAME}"

; GTK+ install section
LangString lng_GTKFiles $(LANG_SLOVAK) "GTK+ runtime environment (povinnÈ)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_SLOVAK) "Multiplatformov· sada pouûÌvateæskÈho rozhrania pouûitÈho v ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_SLOVAK) "Z·stupcovia"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_SLOVAK) "Z·stupcovia pre ötart ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_SLOVAK) "pre vöetk˝ch pouûÌvateæov"

; All user install section description
LangString lng_AlluserDesc $(LANG_SLOVAK) "Inötalovaù aplik·ciu pre kohokoævek, kto pouûÌva tento poËÌtaË. (vöetci pouûÌvatelia)"

; Desktop section
LangString lng_Desktop $(LANG_SLOVAK) "Plocha"

; Desktop section description
LangString lng_DesktopDesc $(LANG_SLOVAK) "Vytvo¯it z·stupcu ${PRODUCT_NAME} na ploche"

; Start Menu  section
LangString lng_Startmenu $(LANG_SLOVAK) "Ponuka ätart"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_SLOVAK) "Vytvoriù pre ${PRODUCT_NAME} poloûku ve ponuke ätart"

; Quick launch section
LangString lng_Quicklaunch $(LANG_SLOVAK) "Panel r˝chlÈho spustenia"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_SLOVAK) "Vytvoriù pre ${PRODUCT_NAME} z·stupcu v paneli r˝chleho spustenia"

; File type association for editing
LangString lng_SVGWriter ${LANG_SLOVAK} "Otv·raù SVG s˙bory v ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_SLOVAK} "Vybraù ${PRODUCT_NAME} ako v˝chodzÌ editor pre SVG soubory"

; Context Menu
LangString lng_ContextMenu ${LANG_SLOVAK} "KontextovÈ menu"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_SLOVAK} "Pridaù ${PRODUCT_NAME} do kontextovÈho menu pre SVG s˙bory"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_SLOVAK} "Delete personal preferences"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_SLOVAK} "Delete personal preferences leftover from previous installations"


; Additional files section
LangString lng_Addfiles $(LANG_SLOVAK) "œalöie s˙bory"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_SLOVAK) "œalöie s˙bory"

; Examples section
LangString lng_Examples $(LANG_SLOVAK) "PrÌklady"

; Examples section description
LangString lng_ExamplesDesc $(LANG_SLOVAK) "PrÌlady pouûÌv·nia ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_SLOVAK) "Sprievodcovia"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_SLOVAK) "Sprievodcovia funkciami ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_SLOVAK) "JazykovÈ sady"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_SLOVAK) "Nainötalovaù Ôalöie jazykovÈ sady ${PRODUCT_NAME}"

LangString lng_am $(LANG_SLOVAK) "am  Amharic"
LangString lng_az $(LANG_SLOVAK) "az  Azerbaijani"
LangString lng_be $(LANG_SLOVAK) "be  Byelorussian"
LangString lng_bg $(LANG_SLOVAK) "bg  Bulgarian"
LangString lng_bn $(LANG_SLOVAK) "bn  Bengali"
LangString lng_ca $(LANG_SLOVAK) "ca  Catalan"
LangString lng_cs $(LANG_SLOVAK) "cs  Czech"
LangString lng_da $(LANG_SLOVAK) "da  Danish"
LangString lng_de $(LANG_SLOVAK) "de  German"
LangString lng_dz $(LANG_SLOVAK) "dz  Dzongkha"
LangString lng_el $(LANG_SLOVAK) "el  Greek"
LangString lng_en $(LANG_SLOVAK) "en  English"
LangString lng_en_AU $(LANG_SLOVAK) "en_AU Australian English"
LangString lng_en_CA $(LANG_SLOVAK) "en_CA Canadian English"
LangString lng_en_GB $(LANG_SLOVAK) "en_GB British English"
LangString lng_en_US@piglatin $(LANG_SLOVAK) "en_US@piglatin Pig Latin"
LangString lng_eo $(LANG_SLOVAK) "eo  Esperanto"
LangString lng_es $(LANG_SLOVAK) "es  Spanish"
LangString lng_es_MX $(LANG_SLOVAK) "es_MX  Mexican Spanish"
LangString lng_et $(LANG_SLOVAK) "et  Estonian"
LangString lng_eu $(LANG_SLOVAK) "eu  Basque"
LangString lng_fi $(LANG_SLOVAK) "fi  Finish"
LangString lng_fr $(LANG_SLOVAK) "fr  French"
LangString lng_ga $(LANG_SLOVAK) "ga  Irish"
LangString lng_gl $(LANG_SLOVAK) "gl  Gallegan"
LangString lng_hr $(LANG_SLOVAK) "hr  Croatian"
LangString lng_hu $(LANG_SLOVAK) "hu  Hungarian"
LangString lng_id $(LANG_SLOVAK) "id  Indonesian"
LangString lng_it $(LANG_SLOVAK) "it  Italian"
LangString lng_ja $(LANG_SLOVAK) "ja  Japanese"
LangString lng_km $(LANG_SLOVAK) "km  Khmer"
LangString lng_ko $(LANG_SLOVAK) "ko  Korean"
LangString lng_lt $(LANG_SLOVAK) "lt  Lithuanian"
LangString lng_mk $(LANG_SLOVAK) "mk  Macedonian"
LangString lng_mn $(LANG_SLOVAK) "mn  Mongolian"
LangString lng_ne $(LANG_SLOVAK) "ne  Nepali"
LangString lng_nb $(LANG_SLOVAK) "nb  Norwegian Bokmal"
LangString lng_nl $(LANG_SLOVAK) "nl  Dutch"
LangString lng_nn $(LANG_SLOVAK) "nn  Norwegian Nynorsk"
LangString lng_pa $(LANG_SLOVAK) "pa  Panjabi"
LangString lng_pl $(LANG_SLOVAK) "po  Polish"
LangString lng_pt $(LANG_SLOVAK) "pt  Portuguese"
LangString lng_pt_BR $(LANG_SLOVAK) "pt_BR Brazilian Portuguese"
LangString lng_ro $(LANG_SLOVAK) "ro  Romanian"
LangString lng_ru $(LANG_SLOVAK) "ru  Russian"
LangString lng_rw $(LANG_SLOVAK) "rw  Kinyarwanda"
LangString lng_sk $(LANG_SLOVAK) "sk  Slovak"
LangString lng_sl $(LANG_SLOVAK) "sl  Slovenian"
LangString lng_sq $(LANG_SLOVAK) "sq  Albanian"
LangString lng_sr $(LANG_SLOVAK) "sr  Serbian"
LangString lng_sr@Latn $(LANG_SLOVAK) "sr@Latn  Serbian in Latin script"
LangString lng_sv $(LANG_SLOVAK) "sv  Swedish"
LangString lng_th $(LANG_SLOVAK) "th  Thai"
LangString lng_tr $(LANG_SLOVAK) "tr  Turkish"
LangString lng_uk $(LANG_SLOVAK) "uk  Ukrainian"
LangString lng_vi $(LANG_SLOVAK) "vi  Vietnamese"
LangString lng_zh_CN $(LANG_SLOVAK) "zh_CH  Simplifed Chinese"
LangString lng_zh_TW $(LANG_SLOVAK) "zh_TW  Traditional Chinese"




; uninstallation options
LangString lng_UInstOpt   ${LANG_SLOVAK} "Moûnosti deinötal·cie"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_SLOVAK} "Zvoæte prosÌm Ôalöie moûnosti"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_SLOVAK} "Ponechaù osobnÈ nastavenia"

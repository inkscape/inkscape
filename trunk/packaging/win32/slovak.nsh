; #######################################
; slovak.nsh
; slovak language strings for inkscape installer
; windows code page: 1250
; Authors:
; Ivan Mas·r <helix84@centrum.sk> (translation for Inkscape 0.44, 0.45, 0.46)
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

!insertmacro MUI_LANGUAGE "Slovak"

; Product name
LangString lng_Caption   ${LANG_SLOVAK} "${PRODUCT_NAME} -- Open source editor SVG grafiky"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_SLOVAK} "œalej >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_SLOVAK} "$(^Name) je moûnÈ öÌriù za podmienok General Public License (GPL). LicenËn· zmluva je tu len pre informaËnÈ ˙Ëely. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_SLOVAK} "Inkscape nainötaloval pouûÌvateæ $0.$\r$\nInötal·cia nemusÌ spr·vne skonËiù, ak v nej budete pokraËovaù!$\r$\nProsÌm, prihl·ste sa ako $0 a spustite inötal·ciu znova."

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_SLOVAK} "$R1 uû je nainötalovan˝. $\nChcete odstr·niù predch·dzaj˙cu verziu predt˝m, neû nainötalujete $(^Name) ?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_SLOVAK} "$\n$\nPokraËujte stlaËenÌm OK alebo zruöte inötal·ciu stlaËenÌm Zruöiù."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_SLOVAK} "Nem·te pr·va spr·vcu.$\r$\nInötal·cia Inkscape pre vöetk˝ch pouûÌvateæov nemusÌ skonËiù ˙speöne.$\r$\nZruöte oznaËenie voæby ÑPre vöetk˝ch pouûÌvateæovì."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_SLOVAK} "Inkscape nebeûÌ na Windows 95/98/ME!$\r$\nPodrobnejöie inform·cie n·jdete na ofici·lnom webe."

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
LangString lng_Quicklaunch $(LANG_SLOVAK) "Panel r˝chleho spustenia"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_SLOVAK) "Vytvoriù pre ${PRODUCT_NAME} z·stupcu v paneli r˝chleho spustenia"

; File type association for editing
LangString lng_SVGWriter ${LANG_SLOVAK} "Otv·raù SVG s˙bory v ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_SLOVAK} "Vybraù ${PRODUCT_NAME} ako ötandardn˝ editor pre SVG s˙bory"

; Context Menu
LangString lng_ContextMenu ${LANG_SLOVAK} "Kontextov· ponuka"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_SLOVAK} "Pridaù ${PRODUCT_NAME} do kontextovÈho menu pre SVG s˙bory"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_SLOVAK} "Zmazaù osobnÈ nastavenia"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_SLOVAK} "Zmazaù osobnÈ nastavenia ponechanÈ z predch·dzaj˙cich inötal·ciÌ"


; Additional files section
LangString lng_Addfiles $(LANG_SLOVAK) "œalöie s˙bory"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_SLOVAK) "œalöie s˙bory"

; Examples section
LangString lng_Examples $(LANG_SLOVAK) "PrÌklady"

; Examples section description
LangString lng_ExamplesDesc $(LANG_SLOVAK) "PrÌklady pouûÌvania ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_SLOVAK) "Sprievodcovia"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_SLOVAK) "Sprievodcovia funkciami ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_SLOVAK) "JazykovÈ sady"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_SLOVAK) "Nainötalovaù Ôalöie jazykovÈ sady ${PRODUCT_NAME}"

LangString lng_am $(LANG_SLOVAK) "am  amharËina"
LangString lng_ar $(LANG_SLOVAK) "ar  arabËina"
LangString lng_az $(LANG_SLOVAK) "az  azerbajdûanËina"
LangString lng_be $(LANG_SLOVAK) "be  bieloruötina"
LangString lng_bg $(LANG_SLOVAK) "bg  bulharËina"
LangString lng_bn $(LANG_SLOVAK) "bn  beng·lËina"
LangString lng_br $(LANG_SLOVAK) "br  bretÛnËina"
LangString lng_ca $(LANG_SLOVAK) "ca  katal·nËina"
LangString lng_ca@valencia $(LANG_SLOVAK) "ca@valencia  valencijËina"
LangString lng_cs $(LANG_SLOVAK) "cs  Ëeötina"
LangString lng_da $(LANG_SLOVAK) "da  d·nËina"
LangString lng_de $(LANG_SLOVAK) "de  nemËina"
LangString lng_dz $(LANG_SLOVAK) "dz  dzongk‰"
LangString lng_el $(LANG_SLOVAK) "el  grÈËtina"
LangString lng_en $(LANG_SLOVAK) "en  angliËtina"
LangString lng_en_AU $(LANG_SLOVAK) "en_AU angliËtina (Austr·lia)"
LangString lng_en_CA $(LANG_SLOVAK) "en_CA angliËtina (Kanada)"
LangString lng_en_GB $(LANG_SLOVAK) "en_GB angliËtina (SpojenÈ kr·æovstvo)"
LangString lng_en_US@piglatin $(LANG_SLOVAK) "en_US@piglatin Pig Latin"
LangString lng_eo $(LANG_SLOVAK) "eo  esperanto"
LangString lng_es $(LANG_SLOVAK) "es  öpanielËina"
LangString lng_es_MX $(LANG_SLOVAK) "es_MX  öpanielËina (Mexiko)"
LangString lng_et $(LANG_SLOVAK) "et  estÛnËina"
LangString lng_eu $(LANG_SLOVAK) "eu  baskiËtina"
LangString lng_fi $(LANG_SLOVAK) "fi  fÌnËina"
LangString lng_fr $(LANG_SLOVAK) "fr  franc˙zötina"
LangString lng_ga $(LANG_SLOVAK) "ga  ÌrËina"
LangString lng_gl $(LANG_SLOVAK) "gl  galÌcijËina"
LangString lng_he $(LANG_SLOVAK) "he  hebrejËina"
LangString lng_hr $(LANG_SLOVAK) "hr  chorv·tËina"
LangString lng_hu $(LANG_SLOVAK) "hu  maÔarËina"
LangString lng_id $(LANG_SLOVAK) "id  indonÈzötina"
LangString lng_it $(LANG_SLOVAK) "it  talianËina"
LangString lng_ja $(LANG_SLOVAK) "ja  japonËina"
LangString lng_km $(LANG_SLOVAK) "km  khmÈrËina"
LangString lng_ko $(LANG_SLOVAK) "ko  kÛrejËina"
LangString lng_lt $(LANG_SLOVAK) "lt  litovËina"
LangString lng_mk $(LANG_SLOVAK) "mk  macedÛnËina"
LangString lng_mn $(LANG_SLOVAK) "mn  mongolËina"
LangString lng_ne $(LANG_SLOVAK) "ne  nep·lËina"
LangString lng_nb $(LANG_SLOVAK) "nb  nÛrsky bokmal"
LangString lng_nl $(LANG_SLOVAK) "nl  holandËina"
LangString lng_nn $(LANG_SLOVAK) "nn  nÛrsky nynorsk"
LangString lng_pa $(LANG_SLOVAK) "pa  pandû·bËina"
LangString lng_pl $(LANG_SLOVAK) "po  poæötina"
LangString lng_pt $(LANG_SLOVAK) "pt  portugalËina"
LangString lng_pt_BR $(LANG_SLOVAK) "pt_BR portugalËina (BrazÌlia)"
LangString lng_ro $(LANG_SLOVAK) "ro  rumunËina"
LangString lng_ru $(LANG_SLOVAK) "ru  ruötina"
LangString lng_rw $(LANG_SLOVAK) "rw  rwandËina"
LangString lng_sk $(LANG_SLOVAK) "sk  slovenËina"
LangString lng_sl $(LANG_SLOVAK) "sl  slovinËina"
LangString lng_sq $(LANG_SLOVAK) "sq  alb·nËina"
LangString lng_sr $(LANG_SLOVAK) "sr  srbËina"
LangString lng_sr@latin $(LANG_SLOVAK) "sr@latin  srbËina (latinka)"
LangString lng_sv $(LANG_SLOVAK) "sv  övÈdËina"
LangString lng_th $(LANG_SLOVAK) "th  thajËina"
LangString lng_tr $(LANG_SLOVAK) "tr  tureËtina"
LangString lng_uk $(LANG_SLOVAK) "uk  ukrajinËina"
LangString lng_vi $(LANG_SLOVAK) "vi  Vietnamese"
LangString lng_zh_CN $(LANG_SLOVAK) "zh_CH  ËÌnötina (zjednoduöen·)"
LangString lng_zh_TW $(LANG_SLOVAK) "zh_TW  ËÌnötina (tradiËn·)"




; uninstallation options
LangString lng_UInstOpt   ${LANG_SLOVAK} "Moûnosti deinötal·cie"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_SLOVAK} "Zvoæte prosÌm Ôalöie moûnosti"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_SLOVAK} "Ponechaù osobnÈ nastavenia"

LangString lng_RETRY_CANCEL_DESC ${LANG_SLOVAK} "$\n$\nPokraËujte stalËenÌm Znovu alebo ukonËite stlaËenÌm Zruöiù."

LangString lng_ClearDirectoryBefore ${LANG_SLOVAK} "${PRODUCT_NAME} musÌ byù nainötalovan˝ do pr·zdneho adres·ra. Adres·r $INSTDIR nie je pr·zdny. ProsÌm, najprv tento adres·r vyËistite!$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_SLOVAK} "$INSTDIR\uninstall.log nebol n·jden˝!$\r$\nProsÌm, odinötalujte ruËn˝m vyËistenÌm adres·ra $INSTDIR !"

LangString lng_FileChanged ${LANG_SLOVAK} "S˙bor $filename sa po inötal·cii zmenil.$\r$\nChcete ho napriek tomu vymazaù?"

LangString lng_Yes ${LANG_SLOVAK} "¡no"

LangString lng_AlwaysYes ${LANG_SLOVAK} "¡no vöetky"

LangString lng_No ${LANG_SLOVAK} "Nie"

LangString lng_AlwaysNo ${LANG_SLOVAK} "Nie vöetky"

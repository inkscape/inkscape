; #######################################
; breton.nsh
; breton language strings for inkscape installer
; windows code page: 1252
; Authors:
; Adib Taraben theAdib@googlemail.com
; matiphas matiphas@free.fr
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

!insertmacro MUI_LANGUAGE "Breton"

; Product name
LangString lng_Caption   ${LANG_BRETON} "${PRODUCT_NAME} - Embanner sturiadel SVG dieub"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_BRETON} "War-lerc'h >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_BRETON} "Skignet eo $(^Name) dindan al lañvaz foran hollek (GPL) GNU. Pourchaset eo al lañvaz amañ evit reiñ keloù deoc'h nemetken. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_BRETON} "Staliet eo bet Inkscape gant an arveriad $0.$\r$\nMar kendalc'hot gant ar staliañ e vo siek marteze !$\r$\nEn em gennaskit evel $0 ha klaskit en-dro."

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_BRETON} "Staliet eo bet $R1 endeo. $\nFellout a ra deoc'h dilemel ar staliadenn gent a-raok staliañ $(^Name) ?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_BRETON} "$\n$\nKlikañ war OK evit kenderc'hel pe CANCEL evit dilezel."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_BRETON} "N'hoc'h eus ket brientoù an ardoer.$\r$\nSiek e c'hallfe bezañ staliadenn Inkscape evit an holl arveriaded.$\r$\nDigevaskit, mar plij, an dibarzh 'evit an holl arveriaded'."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_BRETON} "N'eo ket skoret Inkscape gant Windows 95/98/ME!$\r$\nKit war al lec'hienn gefridiel evit gouzout hiroc'h."

; Full install type
LangString lng_Full $(LANG_BRETON) "Klok"

; Optimal install type
LangString lng_Optimal $(LANG_BRETON) "Gwellek"

; Minimal install type
LangString lng_Minimal $(LANG_BRETON) "Izek"

; Core install section
LangString lng_Core $(LANG_BRETON) "Embanner SVG ${PRODUCT_NAME} (ret)"

; Core install section description
LangString lng_CoreDesc $(LANG_BRETON) "Restroù rekis ${PRODUCT_NAME} ha dlloù"

; GTK+ install section
LangString lng_GTKFiles $(LANG_BRETON) "Amva GTK+ (ret)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_BRETON) "Ur voestad binvioù liessavelennoù evit ketalioù kevregat, arveret gant ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_BRETON) "Berradennoù"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_BRETON) "Berradennoù evit loc'hañ ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_BRETON) "Evit an holl arveriaded"

; All user install section description
LangString lng_AlluserDesc $(LANG_BRETON) "Staliañ ar meziant-mañ evit holl arveriaded an urzhiataer-mañ"

; Desktop section
LangString lng_Desktop $(LANG_BRETON) "Burev"

; Desktop section description
LangString lng_DesktopDesc $(LANG_BRETON) "Krouiñ ur verradenn davit ${PRODUCT_NAME} war ar burev"

; Start Menu  section
LangString lng_Startmenu $(LANG_BRETON) "Lañser Loc'hañ"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_BRETON) "Krouiñ un enankad ${PRODUCT_NAME} war al lañser Loc'hañ"

; Quick launch section
LangString lng_Quicklaunch $(LANG_BRETON) "Loc'hañ trumm"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_BRETON) " Krouiñ ur verradenn davit ${PRODUCT_NAME} er varrennad loc'hañ trumm"

; File type association for editing
LangString lng_SVGWriter ${LANG_BRETON} "Digeriñ ar restroù SVG gant ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_BRETON} "Dibab ${PRODUCT_NAME} evel embanner dre ziouer evit ar restroù SVG"

; Context Menu
LangString lng_ContextMenu ${LANG_BRETON} "Lañser kemperzhel"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_BRETON} "Ouzhpennañ ${PRODUCT_NAME} war lañser kemperzhel ar restroù SVG"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_BRETON} "Diverkañ ar gwellvezioù personel"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_BRETON} "Diverkañ ar gwellvezioù personel chomet gant ar staliadurioù kent"


; Additional files section
LangString lng_Addfiles $(LANG_BRETON) "Restroù ouzhpenn"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_BRETON) "Restroù ouzhpenn"

; Examples section
LangString lng_Examples $(LANG_BRETON) "Skouerioù"

; Examples section description
LangString lng_ExamplesDesc $(LANG_BRETON) "Skouerioù arver eus ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_BRETON) "Skridoù kelenn"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_BRETON) " Skridoù kelenn diwar-benn arver ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_BRETON) "Troidigezhioù"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_BRETON) "Staliañ troidigezhioù evit ${PRODUCT_NAME}"

LangString lng_am $(LANG_BRETON) "am  Amhareg"
LangString lng_ar $(LANG_BRETON) "ar  Arabeg"
LangString lng_az $(LANG_BRETON) "az  Azerbaidjaneg"
LangString lng_be $(LANG_BRETON) "be  Bieloruseg"
LangString lng_bg $(LANG_BRETON) "bg  Bulgareg"
LangString lng_bn $(LANG_BRETON) "bn  Bengalieg"
LangString lng_br $(LANG_BRETON) "br  Brezhoneg"
LangString lng_ca $(LANG_BRETON) "ca  Katalaneg"
LangString lng_ca@valencia $(LANG_BRETON) "ca@valencia  Katalaneg Valensia"
LangString lng_cs $(LANG_BRETON) "cs  Tchekeg"
LangString lng_da $(LANG_BRETON) "da  Daneg"
LangString lng_de $(LANG_BRETON) "de  Alamaneg"
LangString lng_dz $(LANG_BRETON) "dz  Dzongkheg"
LangString lng_el $(LANG_BRETON) "el  Gresianeg"
LangString lng_en $(LANG_BRETON) "en  Saozneg"
LangString lng_en_AU $(LANG_BRETON) "en_AU Saozneg Australia"
LangString lng_en_CA $(LANG_BRETON) "en_CA Saozneg Kanada"
LangString lng_en_GB $(LANG_BRETON) "en_GB Saozneg Breizh-Veur"
LangString lng_en_US@piglatin $(LANG_BRETON) "en_US@piglatin Pig Latin"
LangString lng_eo $(LANG_BRETON) "eo  Esperanto"
LangString lng_es $(LANG_BRETON) "es  Spagnoleg"
LangString lng_es_MX $(LANG_BRETON) "es_MX  Spagnoleg Mec'hiko"
LangString lng_et $(LANG_BRETON) "et  Estoneg"
LangString lng_eu $(LANG_BRETON) "eu  Euskareg"
LangString lng_fi $(LANG_BRETON) "fi  Finneg"
LangString lng_fr $(LANG_BRETON) "fr  Galleg"
LangString lng_he $(LANG_BRETON) "he  Hebraeg"
LangString lng_ga $(LANG_BRETON) "ga  Iwerzhoneg"
LangString lng_gl $(LANG_BRETON) "gl  Galisianeg"
LangString lng_hr $(LANG_BRETON) "hr  Kroateg"
LangString lng_hu $(LANG_BRETON) "hu  Hungareg"
LangString lng_id $(LANG_BRETON) "id  Indoneseg"
LangString lng_it $(LANG_BRETON) "it  Italianeg"
LangString lng_ja $(LANG_BRETON) "ja  Japoneg"
LangString lng_km $(LANG_BRETON) "km  Khmereg"
LangString lng_ko $(LANG_BRETON) "ko  Koreaneg"
LangString lng_lt $(LANG_BRETON) "lt  Lituaneg"
LangString lng_mk $(LANG_BRETON) "mk  Makédonieg"
LangString lng_mn $(LANG_BRETON) "mn  Mongoleg"
LangString lng_ne $(LANG_BRETON) "ne  Nepalieg"
LangString lng_nb $(LANG_BRETON) "nb  Norvegeg Bokmal"
LangString lng_nl $(LANG_BRETON) "nl  Nederlandeg"
LangString lng_nn $(LANG_BRETON) "nn  Norvegeg Ninorsk"
LangString lng_pa $(LANG_BRETON) "pa  Pendjabeg"
LangString lng_pl $(LANG_BRETON) "po  Poloneg"
LangString lng_pt $(LANG_BRETON) "pt  Portugaleg"
LangString lng_pt_BR $(LANG_BRETON) "pt_BR Portugaleg Brazil"
LangString lng_ro $(LANG_BRETON) "ro  Romanieg"
LangString lng_ru $(LANG_BRETON) "ru  Ruseg"
LangString lng_rw $(LANG_BRETON) "rw  Kinyarwandeg"
LangString lng_sk $(LANG_BRETON) "sk  Sloveg"
LangString lng_sl $(LANG_BRETON) "sl  Sloveneg"
LangString lng_sq $(LANG_BRETON) "sq  Albaneg"
LangString lng_sr $(LANG_BRETON) "sr  Serbeg"
LangString lng_sr@latin $(LANG_BRETON) "sr@latin  Serbeg skrivet mod latin"
LangString lng_sv $(LANG_BRETON) "sv  Swedeg"
LangString lng_th $(LANG_BRETON) "th  Thaieg"
LangString lng_tr $(LANG_BRETON) "tr  Turkeg"
LangString lng_uk $(LANG_BRETON) "uk  Ukrenieg"
LangString lng_vi $(LANG_BRETON) "vi  Vietnameg"
LangString lng_zh_CN $(LANG_BRETON) "zh_CH  Sineg  eeunaet"
LangString lng_zh_TW $(LANG_BRETON) "zh_TW  Sineg hengounel"




; uninstallation options
LangString lng_UInstOpt   ${LANG_BRETON} "Dibarzhioù distaliañ"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_BRETON} "Dibabit e-touez an dibarzhioù ouzhpenn"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_BRETON} "Mirout ar gwellvezioù personel"

LangString lng_RETRY_CANCEL_DESC ${LANG_BRETON} "$\n$\nPouezañ war RETRY evit kenderc'hel pe  CANCEL evit dilezel."

LangString lng_ClearDirectoryBefore ${LANG_BRETON} "${PRODUCT_NAME} a rank bezañ staliet e-barzh ur c'havlec'hiad goullo. $INSTDIR n'eo ket goullo. Mar plij, skarzhit ar c'havlec'hiad-mañ da gentañ penn !$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_BRETON} "$INSTDIR\uninstall.log n'eo ket bet kavet !$\r$\nMar plij, distaliit dre skarzhañ ar c'havlec'hiad $INSTDIR hoc'h unan !"

LangString lng_FileChanged ${LANG_BRETON} "Kemmet eo bet ar restr $filename goude ar staliañ.$\r$\nFellout a ra deoc'h diverkañ ar restr-mañ ?"

LangString lng_Yes ${LANG_BRETON} "Ya"

LangString lng_AlwaysYes ${LANG_BRETON} "respont YA atav"

LangString lng_No ${LANG_BRETON} "Ket"

LangString lng_AlwaysNo ${LANG_BRETON} "respont KET atav"

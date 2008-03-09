; #######################################
; slovenian.nsh
; slovenian language strings for inkscape installer
; windows code page: 1250
; Authors:
; Martin Srebotnjak
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

; !insertmacro MUI_LANGUAGE "Slovenšèina"
!insertmacro MUI_LANGUAGE "Slovenian"

; Product name
LangString lng_Caption   ${LANG_SLOVENIAN} "${PRODUCT_NAME} -- Odprtokodni urejevalnik vektorskih slik"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_SLOVENIAN} "Naprej >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_SLOVENIAN} "$(^Name) je izdan pod licenco GNU General Public License (GPL). Priložena licenca služi le v informativne namene. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_SLOVENIAN} "Uporabnik $0 je že namestil Inkscape.$\r$\nÈe nadaljujete, namestitev morda ne bo uspešno dokonèana!$\r$\nPonovno se prijavite kot $0 in poskusite znova."

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_SLOVENIAN} "$R1 je že namešèen. $\nŽelite odstraniti predhodno razlièico, preden namestite $(^Name) ?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_SLOVENIAN} "$\n$\nPritisnite V redu za nadaljevanje ali Preklièi za preklic."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_SLOVENIAN} "Nimate skrbniških pravic.$\r$\nNamešèanje programa Inkscape za vse uporabnike se morda ne bo konèala uspešno.$\r$\nIzklopite možnost 'za vse uporabnike'."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_SLOVENIAN} "Za Inkscape velja, da ne teèe v okoljih Windows 95/98/ME!$\r$\nProsim, oglejte si uradno spletno stran za podrobnejše informacije."

; Full install type
LangString lng_Full $(LANG_SLOVENIAN) "Polna"

; Optimal install type
LangString lng_Optimal $(LANG_SLOVENIAN) "Optimalna"

; Minimal install type
LangString lng_Minimal $(LANG_SLOVENIAN) "Minimalna"

; Core install section
LangString lng_Core $(LANG_SLOVENIAN) "Urejevalnik SVG ${PRODUCT_NAME} (obvezno)"

; Core install section description
LangString lng_CoreDesc $(LANG_SLOVENIAN) "Osnovne datoteke in dll-ji ${PRODUCT_NAME}"

; GTK+ install section
LangString lng_GTKFiles $(LANG_SLOVENIAN) "Izvajalno okolje GTK+ (obvezno)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_SLOVENIAN) "Veèplatformna osnova za uporabniški vmesnik, ki ga uporablja tudi ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_SLOVENIAN) "Bližnjice"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_SLOVENIAN) "Bližnjice za zagon ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_SLOVENIAN) "Za vse uporabnike"

; All user install section description
LangString lng_AlluserDesc $(LANG_SLOVENIAN) "Namesti program za vse, ki uporabljajo ta raèunalnik (vsi uporabniki)"

; Desktop section
LangString lng_Desktop $(LANG_SLOVENIAN) "Namizje"

; Desktop section description
LangString lng_DesktopDesc $(LANG_SLOVENIAN) "Ustvari bližnjico do ${PRODUCT_NAME} na namizju"

; Start Menu  section
LangString lng_Startmenu $(LANG_SLOVENIAN) "Meni Start"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_SLOVENIAN) "Ustvari vnos ${PRODUCT_NAME} v meniju Start"

; Quick launch section
LangString lng_Quicklaunch $(LANG_SLOVENIAN) "Hitri zagon"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_SLOVENIAN) "Ustvari bližnjico ${PRODUCT_NAME} na orodni vrstici Hitri zagon"

; File type association for editing
LangString lng_SVGWriter ${LANG_SLOVENIAN} "Odpiraj datoteke SVG z ${PRODUCT_NAME}-om"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_SLOVENIAN} "Izbere ${PRODUCT_NAME} kot privzeti urejevalnik za datoteke SVG"

; Context Menu
LangString lng_ContextMenu ${LANG_SLOVENIAN} "Pojavni meni"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_SLOVENIAN} "Doda ${PRODUCT_NAME} v pojavni meni za datoteke SVG"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_SLOVENIAN} "Izbriši osebne nastavitve"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_SLOVENIAN} "Izbriši osebne nastavitve, ki so ostale od predhodno namešèene razlièice"


; Additional files section
LangString lng_Addfiles $(LANG_SLOVENIAN) "Dodatne datoteke"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_SLOVENIAN) "Dodatne datoteke"

; Examples section
LangString lng_Examples $(LANG_SLOVENIAN) "Primeri"

; Examples section description
LangString lng_ExamplesDesc $(LANG_SLOVENIAN) "Primeri uporabe ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_SLOVENIAN) "Vodièi s primeri"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_SLOVENIAN) "Vodièi s primeri uporabe ${PRODUCT_NAME}-a"


; Languages section
LangString lng_Languages $(LANG_SLOVENIAN) "Prevodi"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_SLOVENIAN) "Namesti razliène prevode za ${PRODUCT_NAME}"

LangString lng_am $(LANG_SLOVENIAN) "amharski (am)"
LangString lng_ar $(LANG_SLOVENIAN) "arabski (ar)"
LangString lng_az $(LANG_SLOVENIAN) "azerbajdžanski (az)"
LangString lng_be $(LANG_SLOVENIAN) "beloruski (be)"
LangString lng_bg $(LANG_SLOVENIAN) "bolgarski (bg)"
LangString lng_bn $(LANG_SLOVENIAN) "bengalski (bn)"
LangString lng_br $(LANG_SLOVENIAN) "bretonski (br)"
LangString lng_ca $(LANG_SLOVENIAN) "katalonski (ca)"
LangString lng_ca@valencia $(LANG_SLOVENIAN) "valencijski katalonski (ca@valencia)"
LangString lng_cs $(LANG_SLOVENIAN) "èeški (cs)"
LangString lng_da $(LANG_SLOVENIAN) "danski (da)"
LangString lng_de $(LANG_SLOVENIAN) "nemški (de)"
LangString lng_dz $(LANG_SLOVENIAN) "džongški (dz)"
LangString lng_el $(LANG_SLOVENIAN) "grški (el)"
LangString lng_en $(LANG_SLOVENIAN) "angleški (en)"
LangString lng_en_AU $(LANG_SLOVENIAN) "avstralsko angleški (en_AU)"
LangString lng_en_CA $(LANG_SLOVENIAN) "kanadsko angleški (en_CA)"
LangString lng_en_GB $(LANG_SLOVENIAN) "britansko angleški (en_GB)"
LangString lng_en_US@piglatin $(LANG_SLOVENIAN) "obrnjeni angleški zlogi (en_US@piglatin)"
LangString lng_eo $(LANG_SLOVENIAN) "esperanto (eo)"
LangString lng_es $(LANG_SLOVENIAN) "španski (es)"
LangString lng_es_MX $(LANG_SLOVENIAN) "mehiško španski (es_MX)"
LangString lng_et $(LANG_SLOVENIAN) "estonski (et)"
LangString lng_eu $(LANG_SLOVENIAN) "baskovski (eu)"
LangString lng_fi $(LANG_SLOVENIAN) "finski (fi)"
LangString lng_fr $(LANG_SLOVENIAN) "francoski (fr)"
LangString lng_ga $(LANG_SLOVENIAN) "irski (ga)"
LangString lng_gl $(LANG_SLOVENIAN) "galegaški (gl)"
LangString lng_he $(LANG_SLOVENIAN) "hebrejski (he)"
LangString lng_hr $(LANG_SLOVENIAN) "hrvaški (hr)"
LangString lng_hu $(LANG_SLOVENIAN) "madžarski (hu)"
LangString lng_id $(LANG_SLOVENIAN) "indonezijski (id)"
LangString lng_it $(LANG_SLOVENIAN) "italijanski (it)"
LangString lng_ja $(LANG_SLOVENIAN) "japonski (ja)"
LangString lng_km $(LANG_SLOVENIAN) "kmerski (km)"
LangString lng_ko $(LANG_SLOVENIAN) "korejski (ko)"
LangString lng_lt $(LANG_SLOVENIAN) "litovski (lt)"
LangString lng_mk $(LANG_SLOVENIAN) "makedonski (mk)"
LangString lng_mn $(LANG_SLOVENIAN) "mongolski (mn)"
LangString lng_ne $(LANG_SLOVENIAN) "nepalski (ne)"
LangString lng_nb $(LANG_SLOVENIAN) "norveški Bokmal (nb)"
LangString lng_nl $(LANG_SLOVENIAN) "nizozemski (nl)"
LangString lng_nn $(LANG_SLOVENIAN) "norveški Nyorsk (nn)"
LangString lng_pa $(LANG_SLOVENIAN) "pundžabski (pa)"
LangString lng_pl $(LANG_SLOVENIAN) "poljski (po)"
LangString lng_pt $(LANG_SLOVENIAN) "portugalski (pt)"
LangString lng_pt_BR $(LANG_SLOVENIAN) "brazilski portugalski (pt_BR)"
LangString lng_ro $(LANG_SLOVENIAN) "romunski (ro)"
LangString lng_ru $(LANG_SLOVENIAN) "ruski (ru)"
LangString lng_rw $(LANG_SLOVENIAN) "kinjarvandski (rw)"
LangString lng_sk $(LANG_SLOVENIAN) "slovaški (sk)"
LangString lng_sl $(LANG_SLOVENIAN) "slovenski (sl)"
LangString lng_sq $(LANG_SLOVENIAN) "albanski (sq)"
LangString lng_sr $(LANG_SLOVENIAN) "srbski (sr)"
LangString lng_sr@latin $(LANG_SLOVENIAN) "srbski - latinica (sr@latin)"
LangString lng_sv $(LANG_SLOVENIAN) "švedski (sv)"
LangString lng_th $(LANG_SLOVENIAN) "tajski (th)"
LangString lng_tr $(LANG_SLOVENIAN) "turški (tr)"
LangString lng_uk $(LANG_SLOVENIAN) "ukrajinski (uk)"
LangString lng_vi $(LANG_SLOVENIAN) "vietnamski (vi)"
LangString lng_zh_CN $(LANG_SLOVENIAN) "poenostavljeni kitajski (zh_CH)"
LangString lng_zh_TW $(LANG_SLOVENIAN) "tradicionalni kitajski (zh_TW)"




; uninstallation options
LangString lng_UInstOpt   ${LANG_SLOVENIAN} "Možnosti odstranitve"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_SLOVENIAN} "Izberite dodatne možnosti"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_SLOVENIAN} "Ohrani osebne nastavitve"

LangString lng_RETRY_CANCEL_DESC ${LANG_SLOVENIAN} "$\n$\nPritisnite POSKUSI ZNOVA za nadaljevanje ali PREKLIÈI za prekinitev."

LangString lng_ClearDirectoryBefore ${LANG_SLOVENIAN} "${PRODUCT_NAME} mora biti namešèen v prazni mapi. $INSTDIR ni prazna. Najprej poèistite to mapo!$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_SLOVENIAN} "$INSTDIR\uninstall.log ni mogoèe najti!$\r$\nNamestitev odstranite tako, da sami poèistite mapo $INSTDIR!"

LangString lng_FileChanged ${LANG_SLOVENIAN} "Datoteka $filename je bila spremenjena po namestitvi.$\r$\nJo kljub temu želite izbrisati?"

LangString lng_Yes ${LANG_SLOVENIAN} "Da"

LangString lng_AlwaysYes ${LANG_SLOVENIAN} "vedno odgovori z Da"

LangString lng_No ${LANG_SLOVENIAN} "Ne"

LangString lng_AlwaysNo ${LANG_SLOVENIAN} "vedno odgovori z Ne"

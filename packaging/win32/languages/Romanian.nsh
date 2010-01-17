
; #######################################
; romanian.nsh
; Romanian language strings for Inkscape installer
; windows code page: 1250
; Authors:
; Translation: Cristian Secarã <cristi AT secarica DOT ro>
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

!insertmacro MUI_LANGUAGE "Romanian"

; Product name
LangString lng_Caption   ${LANG_ENGLISH} "${PRODUCT_NAME} -- Editor Open Source pentru graficã vectorialã (SVG)"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_ENGLISH} "Înainte >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_ENGLISH} "$(^Name) este publicat sub licenþa publicã generalã GNU (GPL). Licenþa este furnizatã aici numai cu scop informativ. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_ENGLISH} "Inkscape a fost deja instalat de utilizatorul $0.$\r$\nDacã veþi continua, s-ar putea sã nu terminaþi instalarea cu succes !$\r$\nAutentificaþi-vã ca $0 ºi încercaþi din nou."

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_ENGLISH} "$R1 a fost deja instalat. $\nVreþi sã dezinstalaþi versiunea precedentã înainde de a instala $(^Name) ?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_ENGLISH} "$\n$\nApãsaþi butonul OK pentru a continua, sau butonul RENUNÞÃ pentru a opri instalarea."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_ENGLISH} "Nu aveþi privilegii de administrator.$\r$\nInstalarea Inkscape pentru toþi utilizatorii ar putea sã nu se termine cu succes.$\r$\nDebifaþi opþiunea „Pentru toþi utilizatorii”."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_ENGLISH} "Este ºtiut cã Inkscape nu ruleazã sub Windows 95/98/ME !$\r$\nVerificaþi saitul web oficial pentru informaþii detaliate."

; Full install type
LangString lng_Full $(LANG_ENGLISH) "Complet"

; Optimal install type
LangString lng_Optimal $(LANG_ENGLISH) "Optim"

; Minimal install type
LangString lng_Minimal $(LANG_ENGLISH) "Minim"

; Core install section
LangString lng_Core $(LANG_ENGLISH) "${PRODUCT_NAME} SVG Editor (necesar)"

; Core install section description
LangString lng_CoreDesc $(LANG_ENGLISH) "Fiºiere ºi dll-uri indispensabile pentru ${PRODUCT_NAME}"

; GTK+ install section
LangString lng_GTKFiles $(LANG_ENGLISH) "Mediul GTK+ Runtime (necesar)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_ENGLISH) "Kit de instrumente multiplatformã pentru interfeþe grafice, folosit de ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_ENGLISH) "Scurtãturi"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_ENGLISH) "Scurtãturi pentru pornirea ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_ENGLISH) "Pentru toþi utilizatorii"

; All user install section description
LangString lng_AlluserDesc $(LANG_ENGLISH) "Instaleazã aceastã aplicaþie pentru oricine foloseºte acest calculator (toþi utilizatorii)"

; Desktop section
LangString lng_Desktop $(LANG_ENGLISH) "Desktop"

; Desktop section description
LangString lng_DesktopDesc $(LANG_ENGLISH) "Creeazã o scurtãturã cãtre ${PRODUCT_NAME} pe Desktop"

; Start Menu  section
LangString lng_Startmenu $(LANG_ENGLISH) "Meniul Start"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_ENGLISH) "Creeazã o intrare pentru ${PRODUCT_NAME} în meniul Start"

; Quick launch section
LangString lng_Quicklaunch $(LANG_ENGLISH) "Lansare rapidã"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_ENGLISH) "Creeazã o scurtãturã cãtre ${PRODUCT_NAME} pe bara de lansare rapidã"

; File type association for editing
LangString lng_SVGWriter ${LANG_ENGLISH} "Deschidere fiºiere SVG cu ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_ENGLISH} "Selecteazã ${PRODUCT_NAME} ca editor implicit pentru fiºiere SVG"

; Context Menu
LangString lng_ContextMenu ${LANG_ENGLISH} "Meniu contextual"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_ENGLISH} "Adaugã ${PRODUCT_NAME} în meniul contextual pentru fiºiere SVG"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_ENGLISH} "ªtergere preferinþele personale"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_ENGLISH} "ªterge preferinþele personale rãmase de la instalãri precedente"


; Additional files section
LangString lng_Addfiles $(LANG_ENGLISH) "Fiºiere adiþionale"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_ENGLISH) "Fiºiere adiþionale"

; Examples section
LangString lng_Examples $(LANG_ENGLISH) "Exemple"

; Examples section description
LangString lng_ExamplesDesc $(LANG_ENGLISH) "Exemple folosind ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_ENGLISH) "Ghiduri practice"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_ENGLISH) "Ghiduri practice pentru utilizarea ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_ENGLISH) "Traduceri"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_ENGLISH) "Instaleazã diverse traduceri ale interfeþei pentru ${PRODUCT_NAME}"

LangString lng_am $(LANG_ENGLISH) "am  Amharicã"
LangString lng_ar $(LANG_ENGLISH) "ar  Arabã"
LangString lng_az $(LANG_ENGLISH) "az  Azerã"
LangString lng_be $(LANG_ENGLISH) "be  Bielorusã"
LangString lng_bg $(LANG_ENGLISH) "bg  Bulgarã"
LangString lng_bn $(LANG_ENGLISH) "bn  Bengali"
LangString lng_br $(LANG_ENGLISH) "br  Bretonã"
LangString lng_ca $(LANG_ENGLISH) "ca  Catalanã"
LangString lng_ca@valencia $(LANG_ENGLISH) "ca@valencia  Catalanã, Valencian"
LangString lng_cs $(LANG_ENGLISH) "cs  Cehã"
LangString lng_da $(LANG_ENGLISH) "da  Danezã"
LangString lng_de $(LANG_ENGLISH) "de  Germanã"
LangString lng_dz $(LANG_ENGLISH) "dz  Dzongkha"
LangString lng_el $(LANG_ENGLISH) "el  Greacã"
LangString lng_en $(LANG_ENGLISH) "en  Englezã"
LangString lng_en_AU $(LANG_ENGLISH) "en_AU Englezã australianã"
LangString lng_en_CA $(LANG_ENGLISH) "en_CA Englezã canadianã"
LangString lng_en_GB $(LANG_ENGLISH) "en_GB Englezã britanicã"
LangString lng_en_US@piglatin $(LANG_ENGLISH) "en_US@piglatin Pig Latin"
LangString lng_eo $(LANG_ENGLISH) "eo  Esperanto"
LangString lng_es $(LANG_ENGLISH) "es  Spaniolã"
LangString lng_es_MX $(LANG_ENGLISH) "es_MX  Spaniolã mexicanã"
LangString lng_et $(LANG_ENGLISH) "et  Estonianã"
LangString lng_eu $(LANG_ENGLISH) "eu  Bascã"
LangString lng_fi $(LANG_ENGLISH) "fi  Finlandezã"
LangString lng_fr $(LANG_ENGLISH) "fr  Francezã"
LangString lng_ga $(LANG_ENGLISH) "ga  Irlandezã"
LangString lng_gl $(LANG_ENGLISH) "gl  Galicianã"
LangString lng_he $(LANG_ENGLISH) "he  Ebraicã"
LangString lng_hr $(LANG_ENGLISH) "hr  Croatã"
LangString lng_hu $(LANG_ENGLISH) "hu  Maghiarã"
LangString lng_id $(LANG_ENGLISH) "id  Indonezianã"
LangString lng_it $(LANG_ENGLISH) "it  Italianã"
LangString lng_ja $(LANG_ENGLISH) "ja  Japonezã"
LangString lng_km $(LANG_ENGLISH) "km  Khmerã"
LangString lng_ko $(LANG_ENGLISH) "ko  Koreanã"
LangString lng_lt $(LANG_ENGLISH) "lt  Lituanianã"
LangString lng_mk $(LANG_ENGLISH) "mk  Macedoneanã"
LangString lng_mn $(LANG_ENGLISH) "mn  Mongolã"
LangString lng_ne $(LANG_ENGLISH) "ne  Nepalezã"
LangString lng_nb $(LANG_ENGLISH) "nb  Norvegianã cãrturãreascã"
LangString lng_nl $(LANG_ENGLISH) "nl  Olandezã"
LangString lng_nn $(LANG_ENGLISH) "nn  Norvegianã nouã"
LangString lng_pa $(LANG_ENGLISH) "pa  Panjabi"
LangString lng_pl $(LANG_ENGLISH) "po  Polonezã"
LangString lng_pt $(LANG_ENGLISH) "pt  Portughezã"
LangString lng_pt_BR $(LANG_ENGLISH) "pt_BR Portughezã brazilianã"
LangString lng_ro $(LANG_ENGLISH) "ro  Romanânã"
LangString lng_ru $(LANG_ENGLISH) "ru  Rusã"
LangString lng_rw $(LANG_ENGLISH) "rw  Kinyarwanda"
LangString lng_sk $(LANG_ENGLISH) "sk  Slovacã"
LangString lng_sl $(LANG_ENGLISH) "sl  Slovenã"
LangString lng_sq $(LANG_ENGLISH) "sq  Albanezã"
LangString lng_sr $(LANG_ENGLISH) "sr  Sârbã"
LangString lng_sr@latin $(LANG_ENGLISH) "sr@latin  Sârbã (alfabet Latin)"
LangString lng_sv $(LANG_ENGLISH) "sv  Suedezã"
LangString lng_th $(LANG_ENGLISH) "th  Tailandezã"
LangString lng_tr $(LANG_ENGLISH) "tr  Turcã"
LangString lng_uk $(LANG_ENGLISH) "uk  Ucraineanã"
LangString lng_vi $(LANG_ENGLISH) "vi  Vietnamezã"
LangString lng_zh_CN $(LANG_ENGLISH) "zh_CH  Chinezã simplificatã"
LangString lng_zh_TW $(LANG_ENGLISH) "zh_TW  Chinezã tradiþionalã"




; uninstallation options
LangString lng_UInstOpt   ${LANG_ENGLISH} "Opþiuni de dezinstalare"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_ENGLISH} "Alegeþi dintre opþiunile adiþionale"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_ENGLISH} "Pãstreazã preferinþele personale"

LangString lng_RETRY_CANCEL_DESC ${LANG_ENGLISH} "$\n$\nPress RETRY to continue or press CANCEL to abort."

LangString lng_ClearDirectoryBefore ${LANG_ENGLISH} "${PRODUCT_NAME} trebuie sã fie instalat într-un director gol. $INSTDIR nu este gol. Goliþi mai întâi acest director !$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_ENGLISH} "Fiºierul $INSTDIR\uninstall.log nu a fost gãsit !$\r$\nDezinstalaþi prin golirea manualã a $INSTDIR !"

LangString lng_FileChanged ${LANG_ENGLISH} "Fiºierul $filename a fost modificat dupã instalare.$\r$\nTot vreþi sã ºtergeþi acel fiºier ?"

LangString lng_Yes ${LANG_ENGLISH} "Da"

LangString lng_AlwaysYes ${LANG_ENGLISH} "rãspunde totdeauna cu Da"

LangString lng_No ${LANG_ENGLISH} "Nu"

LangString lng_AlwaysNo ${LANG_ENGLISH} "rãspunde totdeauna cu Nu"

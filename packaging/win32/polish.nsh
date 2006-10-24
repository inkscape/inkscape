; #######################################
; polish.nsh
; polish language strings for inkscape installer
; windows code page: 1250
; translator:
; Przemys³aw Loesch p_loesch@poczta.onet.pl
;
; 27 july 2006 new languages en_CA, en_GB, fi, hr, mn, ne, rw, sq
; 11 august 2006 new languages dz bg

!insertmacro MUI_LANGUAGE "Polish"

; Product name
LangString lng_Caption   ${LANG_POLISH} "${PRODUCT_NAME} -- Edytor grafiki wektorowej na licencji Open Source"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_POLISH} "Dalej >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_POLISH} "$(^Name) jest udostêpniony na licencji GNU General Public License (GPL). Tekst licencji jest do³¹czony jedynie w celach informacyjnych. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_POLISH} "Program Inkscape zosta³ zainstalowany przez u¿ytkownika $0.$\r$\nJeœli bêdziesz teraz kontynuowaæ instalacja mo¿e nie zostaæ zakoñczona pomyœlnie!$\r$\nZaloguj siê proszê jako u¿ytkownik $0 i spróbuj ponownie."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_POLISH} "Nie masz uprawnieñ administratora.$\r$\nInstalacja programu Inkscape dla wszystkich u¿ytkowników mo¿e nie zostaæ zakoñczon pomyœlnie.$\r$\nWy³¹cz opcjê 'dla wszystkich u¿ytkowników'."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_POLISH} "Program Inkscape nie dzia³a w systemach Windows 95/98/ME!$\r$\nZobacz na szczegó³owe informacje na ten temat na oficjalnej stronie internetowej programu."

; Full install type
LangString lng_Full $(LANG_POLISH) "Pe³na"

; Optimal install type
LangString lng_Optimal $(LANG_POLISH) "Optymalna"

; Minimal install type
LangString lng_Minimal $(LANG_POLISH) "Minimalna"

; Core install section
LangString lng_Core $(LANG_POLISH) "${PRODUCT_NAME} Edytor SVG (wymagane)"

; Core install section description
LangString lng_CoreDesc $(LANG_POLISH) "Podstawowe pliki i sterowniki dll dla ${PRODUCT_NAME} "

; GTK+ install section
LangString lng_GTKFiles $(LANG_POLISH) "Œrodowisko pracy GTK+ (wymagane)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_POLISH) "Wieloplatformowe œrodowisko graficzne, z którego korzysta ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_POLISH) "Skróty"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_POLISH) "Skróty do uruchamiania ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_POLISH) "dla wszystkich u¿ytkowników"

; All user install section description
LangString lng_AlluserDesc $(LANG_POLISH) "Zainstaluj program dla wszystkich u¿ytkowników komputera"

; Desktop section
LangString lng_Desktop $(LANG_POLISH) "Skrót na pulpicie"

; Desktop section description
LangString lng_DesktopDesc $(LANG_POLISH) "Utwórz skrót na pulpicie do uruchamiania ${PRODUCT_NAME}"

; Start Menu  section
LangString lng_Startmenu $(LANG_POLISH) "Menu Start"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_POLISH) "Utwórz skrót w menu Start do uruchamiania ${PRODUCT_NAME}"

; Quick launch section
LangString lng_Quicklaunch $(LANG_POLISH) "Skrót na pasku szybkiego uruchamiania"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_POLISH) "Utwórz skrót do ${PRODUCT_NAME} na pasku szybkiego uruchamiania"

; File type association for editing
LangString lng_SVGWriter ${LANG_POLISH} "Otwieraj pliki SVG za pomoc¹ ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_POLISH} "Wybierz ${PRODUCT_NAME} jako domyœlny edytor dla plików SVG"

; Context Menu
LangString lng_ContextMenu ${LANG_POLISH} "Menu kontekstowe"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_POLISH} "Dodaj ${PRODUCT_NAME} do menu kontekstowego dla plików SVG"


; Additional files section
LangString lng_Addfiles $(LANG_POLISH) "Dodatkowe pliki"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_POLISH) "Dodatkowe pliki"

; Examples section
LangString lng_Examples $(LANG_POLISH) "Przyk³ady"

; Examples section description
LangString lng_ExamplesDesc $(LANG_POLISH) "Przyk³adowe pliki dla ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_POLISH) "Poradniki"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_POLISH) "Pliki z poradami do korzystania z ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_POLISH) "T³umaczenia"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_POLISH) "Zainstaluj wybrane t³umaczenia interfejsu dla ${PRODUCT_NAME}"

LangString lng_am $(LANG_POLISH) "am  Amharic"
LangString lng_az $(LANG_POLISH) "az  Azerbaijani"
LangString lng_be $(LANG_POLISH) "be  Byelorussian"
LangString lng_bg $(LANG_POLISH) "bg  Bulgarian"
LangString lng_ca $(LANG_POLISH) "ca  Catalan"
LangString lng_cs $(LANG_POLISH) "cs  Czech"
LangString lng_da $(LANG_POLISH) "da  Danish"
LangString lng_de $(LANG_POLISH) "de  German"
LangString lng_dz $(LANG_POLISH) "dz  Dzongkha"
LangString lng_el $(LANG_POLISH) "el  Greek"
LangString lng_en $(LANG_POLISH) "en  English"
LangString lng_en_CA $(LANG_POLISH) "en_CA Canadian English"
LangString lng_en_GB $(LANG_POLISH) "en_GB British English"
LangString lng_es $(LANG_POLISH) "es  Spanish"
LangString lng_es_MX $(LANG_POLISH) "es_MX  Mexican Spanish"
LangString lng_et $(LANG_POLISH) "et  Estonian"
LangString lng_fi $(LANG_POLISH) "fi  Finish"
LangString lng_fr $(LANG_POLISH) "fr  French"
LangString lng_ga $(LANG_POLISH) "ga  Irish"
LangString lng_gl $(LANG_POLISH) "gl  Gallegan"
LangString lng_hr $(LANG_POLISH) "hr  Croatian"
LangString lng_hu $(LANG_POLISH) "hu  Hungarian"
LangString lng_it $(LANG_POLISH) "it  Italian"
LangString lng_ja $(LANG_POLISH) "ja  Japanese"
LangString lng_ko $(LANG_POLISH) "ko  Korean"
LangString lng_lt $(LANG_POLISH) "lt  Lithuanian"
LangString lng_mk $(LANG_POLISH) "mk  Macedonian"
LangString lng_mn $(LANG_POLISH) "mn  Mongolian"
LangString lng_ne $(LANG_POLISH) "ne  Nepali"
LangString lng_nb $(LANG_POLISH) "nb  Norwegian Bokmål"
LangString lng_nl $(LANG_POLISH) "nl  Dutch"
LangString lng_nn $(LANG_POLISH) "nn  Norwegian Nynorsk"
LangString lng_pa $(LANG_POLISH) "pa  Panjabi"
LangString lng_pl $(LANG_POLISH) "pl  Polski"
LangString lng_pt $(LANG_POLISH) "pt  Portuguese"
LangString lng_pt_BR $(LANG_POLISH) "pt_BR Brazilian Portuguese"
LangString lng_ru $(LANG_POLISH) "ru  Russian"
LangString lng_rw $(LANG_POLISH) "rw  Kinyarwanda"
LangString lng_sk $(LANG_POLISH) "sk  Slovak"
LangString lng_sl $(LANG_POLISH) "sl  Slovenian"
LangString lng_sq $(LANG_POLISH) "sq  Albanian"
LangString lng_sr $(LANG_POLISH) "sr  Serbian"
LangString lng_sr@Latn $(LANG_POLISH) "sr@Latn  Serbian in Latin script"
LangString lng_sv $(LANG_POLISH) "sv  Swedish"
LangString lng_tr $(LANG_POLISH) "tr  Turkish"
LangString lng_uk $(LANG_POLISH) "uk  Ukrainian"
LangString lng_vi $(LANG_POLISH) "vi  Vietnamese"
LangString lng_zh_CN $(LANG_POLISH) "zh_CH  Simplifed Chinese"
LangString lng_zh_TW $(LANG_POLISH) "zh_TW  Traditional Chinese"




; uninstallation options
LangString lng_UInstOpt   ${LANG_POLISH} "Opcje odinstalowania"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_POLISH} "Dokonaj wyboru spoœród dodatkowych opcji"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_POLISH} "Zachowaj w systemie preferencje u¿ytkownika "

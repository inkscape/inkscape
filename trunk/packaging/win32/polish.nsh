; #######################################
; polish.nsh
; polish language strings for inkscape installer
; windows code page: 1250
; translator:
; Przemys³aw Loesch p_loesch@poczta.onet.pl
; Marcin Floryan marcin.floryan@gmail.com
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

!insertmacro MUI_LANGUAGE "Polish"

; Product name
LangString lng_Caption   ${LANG_POLISH} "${PRODUCT_NAME} -- edytor grafiki wektorowej open source"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_POLISH} "Dalej >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_POLISH} "$(^Name) jest udostêpniony na licencji GNU General Public License (GPL). Tekst licencji jest do³¹czony jedynie w celach informacyjnych. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_POLISH} "Program Inkscape zosta³ zainstalowany przez u¿ytkownika $0.$\r$\nJeœli bêdziesz teraz kontynuowaæ instalacja mo¿e nie zostaæ zakoñczona pomyœlnie!$\r$\nZaloguj siê proszê jako u¿ytkownik $0 i spróbuj ponownie."

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_POLISH} "$R1 jest ju¿ zainstalowany. $\nCzy chcesz usun¹æ poprzedni¹ wersjê przed zainstalowaniem $(^Name) ?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_POLISH} "$\n$\nNaciœnij OK aby kontynuowaæ lub ANULUJ aby przerwaæ."

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

; remove personal preferences
LangString lng_DeletePrefs ${LANG_POLISH} "Usuñ preferencje u¿ytkowników"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_POLISH} "Usuñ preferencje u¿ytkowników które pozosta³y z poprzedniej instalacji"


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
LangString lng_TutorialsDesc $(LANG_POLISH) "Pliki z poradami jak korzystaæ z ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_POLISH) "Wersje jêzykowe"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_POLISH) "T³umaczenia interfejsu ${PRODUCT_NAME} w wybranych jêzykach"

LangString lng_am $(LANG_POLISH) "am  amharski"
LangString lng_ar $(LANG_POLISH) "ar  arabski"
LangString lng_az $(LANG_POLISH) "az  azerski"
LangString lng_be $(LANG_POLISH) "be  bia³oruski"
LangString lng_bg $(LANG_POLISH) "bg  bu³garski"
LangString lng_bn $(LANG_POLISH) "bn  bengalski"
LangString lng_br $(LANG_POLISH) "br  bretoñski"
LangString lng_ca $(LANG_POLISH) "ca  kataloñski"
LangString lng_ca@valencia $(LANG_POLISH) "ca@valencia  kataloñski - dialekt walencki"
LangString lng_cs $(LANG_POLISH) "cs  czeski"
LangString lng_da $(LANG_POLISH) "da  duñski"
LangString lng_de $(LANG_POLISH) "de  niemiecki"
LangString lng_dz $(LANG_POLISH) "dz  dzongka"
LangString lng_el $(LANG_POLISH) "el  grecki"
LangString lng_en $(LANG_POLISH) "en  angielski"
LangString lng_en_AU $(LANG_POLISH) "en_AU angielski (Australia)"
LangString lng_en_CA $(LANG_POLISH) "en_CA angielski (Kanada)"
LangString lng_en_GB $(LANG_POLISH) "en_GB angielski (Wielka Brytania)"
LangString lng_en_US@piglatin $(LANG_POLISH) "en_US@piglatin angielski (Pig Latin)"
LangString lng_eo $(LANG_POLISH) "eo  esperanto"
LangString lng_es $(LANG_POLISH) "es  hiszpañski"
LangString lng_es_MX $(LANG_POLISH) "es_MX  hiszpañski (Meksyk)"
LangString lng_et $(LANG_POLISH) "et  estoñski"
LangString lng_eu $(LANG_POLISH) "eu  baskijski"
LangString lng_fi $(LANG_POLISH) "fi  fiñski"
LangString lng_fr $(LANG_POLISH) "fr  francuski"
LangString lng_ga $(LANG_POLISH) "ga  irlandzki"
LangString lng_gl $(LANG_POLISH) "gl  galicyjski"
LangString lng_he $(LANG_POLISH) "he  hebrajski"
LangString lng_hr $(LANG_POLISH) "hr  chorwacki"
LangString lng_hu $(LANG_POLISH) "hu  wêgierski"
LangString lng_id $(LANG_POLISH) "id  indonezyjski"
LangString lng_it $(LANG_POLISH) "it  w³oski"
LangString lng_ja $(LANG_POLISH) "ja  japoñski"
LangString lng_km $(LANG_POLISH) "km  khmerski"
LangString lng_ko $(LANG_POLISH) "ko  koreañski"
LangString lng_lt $(LANG_POLISH) "lt  litewski"
LangString lng_mk $(LANG_POLISH) "mk  macedoñski"
LangString lng_mn $(LANG_POLISH) "mn  mongolski"
LangString lng_ne $(LANG_POLISH) "ne  nepalski"
LangString lng_nb $(LANG_POLISH) "nb  norweski Bokmål"
LangString lng_nl $(LANG_POLISH) "nl  duñski"
LangString lng_nn $(LANG_POLISH) "nn  norweski Nynorsk"
LangString lng_pa $(LANG_POLISH) "pa  pend¿abski"
LangString lng_pl $(LANG_POLISH) "pl  polski"
LangString lng_pt $(LANG_POLISH) "pt  portugalski"
LangString lng_pt_BR $(LANG_POLISH) "pt_BR portugalski (Brazylia)"
LangString lng_ro $(LANG_POLISH) "ro  rumuñski"
LangString lng_ru $(LANG_POLISH) "ru  rosyjski"
LangString lng_rw $(LANG_POLISH) "rw  ruanda-rundi"
LangString lng_sk $(LANG_POLISH) "sk  s³owacki"
LangString lng_sl $(LANG_POLISH) "sl  s³oweñski"
LangString lng_sq $(LANG_POLISH) "sq  albañski"
LangString lng_sr $(LANG_POLISH) "sr  serbski"
LangString lng_sr@latin $(LANG_POLISH) "sr@latin  serbski (alfabet ³aciñski)"
LangString lng_sv $(LANG_POLISH) "sv  szwedzki"
LangString lng_th $(LANG_POLISH) "th  tajski"
LangString lng_tr $(LANG_POLISH) "tr  turecki"
LangString lng_uk $(LANG_POLISH) "uk  ukraiñski"
LangString lng_vi $(LANG_POLISH) "vi  wietnamski"
LangString lng_zh_CN $(LANG_POLISH) "zh_CH  chiñski uproszczony"
LangString lng_zh_TW $(LANG_POLISH) "zh_TW  chiñski tradycyjny"




; uninstallation options
LangString lng_UInstOpt   ${LANG_POLISH} "Opcje odinstalowania"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_POLISH} "Dokonaj wyboru spoœród dodatkowych opcji"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_POLISH} "Zachowaj w systemie preferencje u¿ytkownika "

LangString lng_RETRY_CANCEL_DESC ${LANG_POLISH} "$\n$\nNaciœnij PONÓW by kontynuowaæ lub ANULUJ aby przerwaæ."

LangString lng_ClearDirectoryBefore ${LANG_POLISH} "${PRODUCT_NAME} musi byæ zainstalowany w pustym folderze. $INSTDIR nie jest pusty. Proszê usuñ jego zawartoœæ!$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_POLISH} "Plik $INSTDIR\uninstall.log nie zosta³ znaleziony!$\r$\nOdinstaluj program rêcznie usuwaj¹c zawartoœæ folderu $INSTDIR!"

LangString lng_FileChanged ${LANG_POLISH} "Plik $filename zosta³ zmieniony po zainstalowaniu.$\r$\nCzy na pewno chcesz usun¹æ ten plik?"

LangString lng_Yes ${LANG_POLISH} "Tak"

LangString lng_AlwaysYes ${LANG_POLISH} "Tak na wszystkie"

LangString lng_No ${LANG_POLISH} "Nie"

LangString lng_AlwaysNo ${LANG_POLISH} "Nie na wszystkie"

; #######################################
; czech.nsh
; czech language strings for inkscape installer
; windows code page: 1250
; Authors:
; Michal Kraus Michal.Kraus@wige-mic.cz (original translation)
; Josef VybÌral josef.vybiral@gmail.com (update for Inkscape 0.44)
;
; 27 july 2006 new languages en_CA, en_GB, fi, hr, mn, ne, rw, sq
; 11 august 2006 new languages dz bg
; 24 october 2006 new languages en_US@piglatin, th

!insertmacro MUI_LANGUAGE "Czech"

; Product name
LangString lng_Caption   ${LANG_CZECH} "${PRODUCT_NAME} -- Open Source Editor äk·lovatelnÈ VektorovÈ Grafiky(SVG)"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_CZECH} "DalöÌ >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_CZECH} "$(^Name) je vyd·v·n pod General Public License (GPL). LicenËnÌ ujedn·nÌ je zde pouze z informaËnÌch d˘vod˘. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_CZECH} "Inkscape byl nainstalov·n uûivatelem $0.$\r$\nInstalace nemusÌ b˝t dokonËena spr·vnÏ pokud v nÌ budete pokraËovat!$\r$\nProsÌm p¯ihlaste se jako $0 a spusùte instalaci znovu."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_CZECH} "Nem·te administr·torsk· opr·vnÏnÌ.$\r$\nInstalace Inkscape pro vöechny uûivatele nemusÌ b˝t ˙spÏönÏ dokonËena.$\r$\nZruöte oznaËenÌ volby 'Pro vöechny uûivatele'."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_CZECH} "Inkscape nebÏûÌ na Windows 95/98/ME!$\r$\nPro podrobnÏjöÌ informace se prosÌm obraùte na ofici·lnÌ webovÈ str·nky."

; Full install type
LangString lng_Full $(LANG_CZECH) "Pln·"

; Optimal install type
LangString lng_Optimal $(LANG_CZECH) "Optim·lnÌ"

; Minimal install type
LangString lng_Minimal $(LANG_CZECH) "Minim·lnÌ"

; Core install section
LangString lng_Core $(LANG_CZECH) "${PRODUCT_NAME} SVG editor (vyûadov·no)"

; Core install section description
LangString lng_CoreDesc $(LANG_CZECH) "Soubory a knihovny ${PRODUCT_NAME}"

; GTK+ install section
LangString lng_GTKFiles $(LANG_CZECH) "GTK+ bÏhovÈ prost¯edÌ (vyûadov·no)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_CZECH) "MultiplatformnÌ sada uûivatelskÈho rozhranÌ, pouûitÈho v ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_CZECH) "Z·stupci"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_CZECH) "Z·stupci pro spuötÏnÌ ${PRODUCT_NAME}"

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
LangString lng_StartmenuDesc $(LANG_CZECH) "Vytvo¯it pro ${PRODUCT_NAME} poloûku v nabÌdce Start"

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
LangString lng_ExamplesDesc $(LANG_CZECH) "P¯Ìklady pouûitÌ ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_CZECH) "Pr˘vodci"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_CZECH) "Pr˘vodci funkcemi ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_CZECH) "JazykovÈ sady"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_CZECH) "Nainstalovat dalöÌ jazykovÈ sady ${PRODUCT_NAME}"

LangString lng_am $(LANG_CZECH) "am  Amharic"
LangString lng_az $(LANG_CZECH) "az  Azerbajdû·nötina"
LangString lng_be $(LANG_CZECH) "be  BÏloruötina"
LangString lng_bg $(LANG_CZECH) "bg  Bulgarian"
LangString lng_ca $(LANG_CZECH) "ca  Katal·nötina"
LangString lng_cs $(LANG_CZECH) "cs  »eötina"
LangString lng_da $(LANG_CZECH) "da  D·nötina"
LangString lng_de $(LANG_CZECH) "de  NÏmËina"
LangString lng_dz $(LANG_CZECH) "dz  Dzongkha"
LangString lng_el $(LANG_CZECH) "el  ÿeËtina"
LangString lng_en $(LANG_CZECH) "en  AngliËina"
LangString lng_en_CA $(LANG_CZECH) "en_CA Kanadsk· AngliËtina"
LangString lng_en_GB $(LANG_CZECH) "en_GB Britsk· AngliËtina"
LangString lng_en_US@piglatin $(LANG_CZECH) "en_US@piglatin Pig Latin"
LangString lng_es $(LANG_CZECH) "es  äpanÏlötina"
LangString lng_es_MX $(LANG_CZECH) "es_MX  Mexick· äpanÏlötina"
LangString lng_et $(LANG_CZECH) "et  Estonötina"
LangString lng_fi $(LANG_CZECH) "fi  Finötina"
LangString lng_fr $(LANG_CZECH) "fr  Francouzötina"
LangString lng_ga $(LANG_CZECH) "ga  Irötina"
LangString lng_gl $(LANG_CZECH) "gl  Gallegan"
LangString lng_hr $(LANG_CZECH) "hr  Chorvatötina"
LangString lng_hu $(LANG_CZECH) "hu  MaÔarötina"
LangString lng_it $(LANG_CZECH) "it  Italötina"
LangString lng_ja $(LANG_CZECH) "ja  Japonötina"
LangString lng_ko $(LANG_CZECH) "ko  Korejötina"
LangString lng_lt $(LANG_CZECH) "lt  Litevötina"
LangString lng_mk $(LANG_CZECH) "mk  Makedonötina"
LangString lng_mn $(LANG_CZECH) "mn  Mongolötina"
LangString lng_ne $(LANG_CZECH) "ne  Nep·lötina"
LangString lng_nb $(LANG_CZECH) "nb  Norötina Bokmal"
LangString lng_nl $(LANG_CZECH) "nl  Holandötina"
LangString lng_nn $(LANG_CZECH) "nn  Norötina Nynorsk"
LangString lng_pa $(LANG_CZECH) "pa  Panjabi"
LangString lng_pl $(LANG_CZECH) "po  Polötina"
LangString lng_pt $(LANG_CZECH) "pt  Portugalötina"
LangString lng_pt_BR $(LANG_CZECH) "pt_BR Brazilsk· Portugalötina"
LangString lng_ru $(LANG_CZECH) "ru  Ruötina"
LangString lng_rw $(LANG_CZECH) "rw  Kinyarwanda"
LangString lng_sk $(LANG_CZECH) "sk  Slovenötina"
LangString lng_sl $(LANG_CZECH) "sl  Slovinötina"
LangString lng_sq $(LANG_CZECH) "sq  Alb·nötina"
LangString lng_sr $(LANG_CZECH) "sr  Srbötina"
LangString lng_sr@Latn $(LANG_CZECH) "sr@Latn  Srbötina v latince"
LangString lng_sv $(LANG_CZECH) "sv  ävÈdötina"
LangString lng_th $(LANG_CZECH) "th  Thai"
LangString lng_tr $(LANG_CZECH) "tr  TureËtina"
LangString lng_uk $(LANG_CZECH) "uk  Ukrajinötina"
LangString lng_vi $(LANG_CZECH) "vi  Vietnamötina"
LangString lng_zh_CN $(LANG_CZECH) "zh_CH  Zjednoduöen· »Ìnötina"
LangString lng_zh_TW $(LANG_CZECH) "zh_TW  TradiËnÌ »Ìnötina"




; uninstallation options
LangString lng_UInstOpt   ${LANG_CZECH} "Volby pro odinstalov·nÌ"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_CZECH} "Vyberte prosÌm dalöÌ nastavenÌ"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_CZECH} "Ponechat osobnÌ nastavenÌ"

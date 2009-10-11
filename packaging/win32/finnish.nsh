; #######################################
; finnish.nsh
; finnish language strings for inkscape installer
; utf-8
; Authors:
; Riku Leino tsoots@gmail.com
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

!insertmacro MUI_LANGUAGE "Finnish"

; Product name
LangString lng_Caption   ${LANG_FINNISH} "${PRODUCT_NAME} -- Avoimen lähdekoodin SVG-muokkain"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_FINNISH} "Seuraava >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_FINNISH} "$(^Name) on julkaistu GNU General Public License (GPL) -lisenssillä. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_FINNISH} "Käyttäjä $0.$\r$\n on asentanut Inkscapen\nJos jatkat, asennus saattaa epäonnistua.!$\r$\nKirjaudu sisään käyttäjänä $0 ja yritä uudestaan."

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_FINNISH} "$R1 on jo asennettu. $\nHaluatko poistaa edellisen version ennen asennusta $(^Name) ?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_FINNISH} "$\n$\nPaina OK jatkaaksi tai Peruuta keskeyttääksesi."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_FINNISH} "Sinulla ei ole pääkäyttäjän oikeuksia.$\r$\nInkscapen asennus kaikille käyttäjille saattaa epäonnistua.$\r$\nÄlä käytä kaikille käyttäjille -ominaisuutta."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_FINNISH} "Inkscape ei toimi käyttöjärjestelmissä Windows 95/98/ME!$\r$\nLisätietoja saat ohjelman kotisivulta."

; Full install type
LangString lng_Full $(LANG_FINNISH) "Täysi"

; Optimal install type
LangString lng_Optimal $(LANG_FINNISH) "Oletus"

; Minimal install type
LangString lng_Minimal $(LANG_FINNISH) "Vähäisin"

; Core install section
LangString lng_Core $(LANG_FINNISH) "${PRODUCT_NAME} SVG-muokkain (pakollinen)"

; Core install section description
LangString lng_CoreDesc $(LANG_FINNISH) "${PRODUCT_NAME} tiedostot ja dll-kirjastot"

; GTK+ install section
LangString lng_GTKFiles $(LANG_FINNISH) "GTK+-ajoympäristö (pakollinen)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_FINNISH) "Usealla alustalla toimiva käyttöliittymäkehys, jota ${PRODUCT_NAME} käyttää"

; shortcuts install section
LangString lng_Shortcuts $(LANG_FINNISH) "Pikakuvakkeet"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_FINNISH) "Pikakuvakkeet Inkscapen käynnistämiseksi"

; All user install section
LangString lng_Alluser $(LANG_FINNISH) "kaikille käyttäjille"

; All user install section description
LangString lng_AlluserDesc $(LANG_FINNISH) "Asenna Inkscape kaikille tämän tietokoneen käyttäjille"

; Desktop section
LangString lng_Desktop $(LANG_FINNISH) "Työpöytä"

; Desktop section description
LangString lng_DesktopDesc $(LANG_FINNISH) "Luo ${PRODUCT_NAME}-pikakuvake työpöydälle"

; Start Menu  section
LangString lng_Startmenu $(LANG_FINNISH) "Käynnistä-valikko"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_FINNISH) "Lisää ${PRODUCT_NAME} Käynnistä-valikkoon"

; Quick launch section
LangString lng_Quicklaunch $(LANG_FINNISH) "Pikakäynnistys"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_FINNISH) "Lisää ${PRODUCT_NAME} pikakäynnistysriville"

; File type association for editing
LangString lng_SVGWriter ${LANG_FINNISH} "Avaa SVG-tiedostot Inkscapella"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_FINNISH} "Tee Inkscapesta oletusmuokkain SVG-tiedostoille"

; Context Menu
LangString lng_ContextMenu ${LANG_FINNISH} "Kontekstivalikko"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_FINNISH} "Lisää ${PRODUCT_NAME} SVG-tiedostojen pikavalikkoon"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_FINNISH} "Poista asetukset"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_FINNISH} "Poista edelliseen versioon tehdyt asetukset"


; Additional files section
LangString lng_Addfiles $(LANG_FINNISH) "Valinnaiset tiedostot"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_FINNISH) "Valinnaiset tiedostot"

; Examples section
LangString lng_Examples $(LANG_FINNISH) "Esimerkit"

; Examples section description
LangString lng_ExamplesDesc $(LANG_FINNISH) "Inkscapen avulla tehdyt esimerkit"

; Tutorials section
LangString lng_Tutorials $(LANG_FINNISH) "Ohjeet"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_FINNISH) "Inkscapen käyttöä opettavat ohjeet (englanniksi)"


; Languages section
LangString lng_Languages $(LANG_FINNISH) "Käännökset"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_FINNISH) "Asenna Inkscapen käännökset"

LangString lng_am $(LANG_FINNISH) "am  amhari"
LangString lng_ar $(LANG_FINNISH) "ar  Arabic"
LangString lng_az $(LANG_FINNISH) "az  azerbaid?ani"
LangString lng_be $(LANG_FINNISH) "be  valkovenäjä"
LangString lng_bg $(LANG_FINNISH) "bg  bulgaria"
LangString lng_bn $(LANG_FINNISH) "bn  Bengali"
LangString lng_br $(LANG_FINNISH) "br  Breton"
LangString lng_ca $(LANG_FINNISH) "ca  katalaani"
LangString lng_ca@valencia $(LANG_FINNISH) "ca@valencia  Valencian Catalan"
LangString lng_cs $(LANG_FINNISH) "cs  t?ekki"
LangString lng_da $(LANG_FINNISH) "da  tanska"
LangString lng_de $(LANG_FINNISH) "de  saksa"
LangString lng_dz $(LANG_FINNISH) "dz  dzongkha"
LangString lng_el $(LANG_FINNISH) "el  kreikka"
LangString lng_en $(LANG_FINNISH) "en  englanti"
LangString lng_en_AU $(LANG_FINNISH) "en_AU Australian English"
LangString lng_en_CA $(LANG_FINNISH) "en_CA Kanadan englanti"
LangString lng_en_GB $(LANG_FINNISH) "en_GB Britti-englanti"
LangString lng_en_US@piglatin $(LANG_FINNISH) "en_US@piglatin kontin kieli (en)"
LangString lng_eo $(LANG_FINNISH) "eo  Esperanto"
LangString lng_es $(LANG_FINNISH) "es  espanja"
LangString lng_es_MX $(LANG_FINNISH) "es_MX  Meksikon espanja"
LangString lng_et $(LANG_FINNISH) "et  eesti"
LangString lng_eu $(LANG_FINNISH) "eu  Basque"
LangString lng_fi $(LANG_FINNISH) "fi  suomi"
LangString lng_fr $(LANG_FINNISH) "fr  ranska"
LangString lng_ga $(LANG_FINNISH) "ga  iiri"
LangString lng_gl $(LANG_FINNISH) "gl  galicia"
LangString lng_he $(LANG_FINNISH) "he  Hebrew"
LangString lng_hr $(LANG_FINNISH) "hr  kroatia"
LangString lng_hu $(LANG_FINNISH) "hu  unkari"
LangString lng_id $(LANG_FINNISH) "id  Indonesian"
LangString lng_it $(LANG_FINNISH) "it  italia"
LangString lng_ja $(LANG_FINNISH) "ja  japani"
LangString lng_km $(LANG_FINNISH) "km  Khmer"
LangString lng_ko $(LANG_FINNISH) "ko  korea"
LangString lng_lt $(LANG_FINNISH) "lt  liettua"
LangString lng_mk $(LANG_FINNISH) "mk  makedonia"
LangString lng_mn $(LANG_FINNISH) "mn  mongolia"
LangString lng_ne $(LANG_FINNISH) "ne  nepali"
LangString lng_nb $(LANG_FINNISH) "nb  kirjanorja"
LangString lng_nl $(LANG_FINNISH) "nl  hollanti"
LangString lng_nn $(LANG_FINNISH) "nn  nykynorja"
LangString lng_pa $(LANG_FINNISH) "pa  punjabi"
LangString lng_pl $(LANG_FINNISH) "po  puola"
LangString lng_pt $(LANG_FINNISH) "pt  portugali"
LangString lng_pt_BR $(LANG_FINNISH) "pt_BR Brasilian portugali"
LangString lng_ro $(LANG_FINNISH) "ro  Romanian"
LangString lng_ru $(LANG_FINNISH) "ru  venäjä"
LangString lng_rw $(LANG_FINNISH) "rw  ruanda"
LangString lng_sk $(LANG_FINNISH) "sk  slovakki"
LangString lng_sl $(LANG_FINNISH) "sl  sloveeni"
LangString lng_sq $(LANG_FINNISH) "sq  albania"
LangString lng_sr $(LANG_FINNISH) "sr  serbia"
LangString lng_sr@latin $(LANG_FINNISH) "sr@latin  serbia (latin)"
LangString lng_sv $(LANG_FINNISH) "sv  ruotsi"
LangString lng_th $(LANG_FINNISH) "th  thai"
LangString lng_tr $(LANG_FINNISH) "tr  turkki"
LangString lng_uk $(LANG_FINNISH) "uk  ukraina"
LangString lng_vi $(LANG_FINNISH) "vi  vietnami"
LangString lng_zh_CN $(LANG_FINNISH) "zh_CH  yksinkertaistettu kiina"
LangString lng_zh_TW $(LANG_FINNISH) "zh_TW  perinteinen kiina"




; uninstallation options
LangString lng_UInstOpt   ${LANG_FINNISH} "Ohjelman poiston asetukset"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_FINNISH} "Valitse haluamasi asetukset ohjelman poistamiseksi"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_FINNISH} "Säilytä omat asetukset"

LangString lng_RETRY_CANCEL_DESC ${LANG_FINNISH} "$\n$\nPaina Yritä uudelleen jatkaaksei tai Peruuta lopettaaksesi."

LangString lng_ClearDirectoryBefore ${LANG_FINNISH} "${PRODUCT_NAME} täytyy asentaa tyhjään kansioon. $INSTDIR ei ole tyhjä. Tyhjennä kansio ensin!$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_FINNISH} "$INSTDIR\uninstall.log ei löytynyt!$\r$\nPoista tyhjentämällä asennuskansio $INSTDIR!"

LangString lng_FileChanged ${LANG_FINNISH} "Tiedosto $filename on muuttunut asennuksen jälkeen.$\r$\nPoistetaanko se siitä huolimatta?"

LangString lng_Yes ${LANG_FINNISH} "Kyllä"

LangString lng_AlwaysYes ${LANG_FINNISH} "Kyllä kaikkiin"

LangString lng_No ${LANG_FINNISH} "Ei"

LangString lng_AlwaysNo ${LANG_FINNISH} "Ei kaikkiin"

; #######################################
; italian.nsh
; italian language strings for inkscape installer
; windows code page: 1040
; Authors:
; Emanuele Mandola earween@gmail.com
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

!insertmacro MUI_LANGUAGE "Italian"

; Product name
LangString lng_Caption   ${LANG_ITALIAN} "${PRODUCT_NAME} -- Editor di grafica vettoriale Open Source"

; Button text "Avanti >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_ITALIAN} "Avanti >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_ITALIAN} "$(^Name) è rilasciato sotto GNU General Public License (GPL). La licenza è fornita solo a scopo informativo. $_CLICK"

; has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_ITALIAN} "Inkscape è stato installato dall'utente $0.  Potrebbe non essere possibile portare a termine l'installazione.   Accedere come $0 e riprovare."

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_ITALIAN} "$R1 è già stata installata. $\nRimuovere la versione precedente prima di installare $(^Name) ?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_ITALIAN} "$\n$\nPremere OK per continuare o Annulla per uscire."

; you have no admin rigths
LangString lng_NO_ADMIN ${LANG_ITALIAN} "Non si posseggono i diritti di amministratore.  Potrebbe non essere possibile installare Inkscape per tutti gli utenti.  Non selezionare l'opzione 'Per Tutti Gli Utenti'"

; win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_ITALIAN} "Inkscape non è compatibile con i sistemi operativi Windows 95/98/ME!\n\nPer ulteriori informazioni consultare il sito ufficiale."

; Full install type
LangString lng_Full $(LANG_ITALIAN) "Completa"

; Optimal install type
LangString lng_Optimal $(LANG_ITALIAN) "Consigliata"

; Minimal install type
LangString lng_Minimal $(LANG_ITALIAN) "Minima"

; Core install section
LangString lng_Core $(LANG_ITALIAN) "${PRODUCT_NAME} SVG Editor (richiesto)"

; Core install section description
LangString lng_CoreDesc $(LANG_ITALIAN) "Core ${PRODUCT_NAME} file e dll"

; GTK+ install section
LangString lng_GTKFiles $(LANG_ITALIAN) "GTK+ Runtime Environment (richiesto)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_ITALIAN) "Librerie grafiche multipiattaforma, usate da ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_ITALIAN) "Collegamenti"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_ITALIAN) "Collegamenti per l'avvio ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_ITALIAN) "Per tutti gli utenti"

; All user install section description
LangString lng_AlluserDesc $(LANG_ITALIAN) "Installa questa applicazione per tutti coloro che usano questo computer (tutti gli utenti)"

; Desktop section
LangString lng_Desktop $(LANG_ITALIAN) "Desktop"

; Desktop section description
LangString lng_DesktopDesc $(LANG_ITALIAN) "Crea un colegamento a ${PRODUCT_NAME} sul Desktop"

; Start Menu  section
LangString lng_Startmenu $(LANG_ITALIAN) "Start Menu"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_ITALIAN) "Crea una cartella in Start Menu per ${PRODUCT_NAME}"

; Quick launch section
LangString lng_Quicklaunch $(LANG_ITALIAN) "Avvio Rapido"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_ITALIAN) "Crea un collegamento a ${PRODUCT_NAME} nella barra di Avvio Rapido"

; File type association for editing
LangString lng_SVGWriter ${LANG_ITALIAN} "Apre i file SVG con ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_ITALIAN} "Imposta ${PRODUCT_NAME} come editor predefinito per i file SVG"

; Context Menu
LangString lng_ContextMenu ${LANG_ITALIAN} "Menu Contestuale"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_ITALIAN} "Aggiunge ${PRODUCT_NAME} nel Menu Contestuale per i file SVG"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_ITALIAN} "Rimuovere impostazioni personali"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_ITALIAN} "Rimuove le impostazioni personali lasciate da installazioni precedenti"


; Additional files section
LangString lng_Addfiles $(LANG_ITALIAN) "File Aggiuntivi"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_ITALIAN) "File Aggiuntivi"

; Examples section
LangString lng_Examples $(LANG_ITALIAN) "Esempi"

; Examples section description
LangString lng_ExamplesDesc $(LANG_ITALIAN) "Esempi d'uso di ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_ITALIAN) "Tutorial"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_ITALIAN) "Tutorial per l'uso di ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_ITALIAN) "Traduzioni"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_ITALIAN) "Installa altre traduzioni per ${PRODUCT_NAME}"

LangString lng_am $(LANG_ITALIAN) "am  Aramaico"
LangString lng_ar $(LANG_ITALIAN) "ar  Arabo"
LangString lng_az $(LANG_ITALIAN) "az  Azerbaigiano"
LangString lng_be $(LANG_ITALIAN) "be  Bielorusso"
LangString lng_bg $(LANG_ITALIAN) "bg  Bulgaro"
LangString lng_bn $(LANG_ITALIAN) "bn  Bengali"
LangString lng_br $(LANG_ITALIAN) "br  Bretone"
LangString lng_ca $(LANG_ITALIAN) "ca  Catalano"
LangString lng_ca@valencia $(LANG_ITALIAN) "ca@valencia  Catalano Valenciano"
LangString lng_cs $(LANG_ITALIAN) "cs  Ceco"
LangString lng_da $(LANG_ITALIAN) "da  Danese"
LangString lng_de $(LANG_ITALIAN) "de  Tedesco"
LangString lng_dz $(LANG_ITALIAN) "dz  Dzongkha"
LangString lng_el $(LANG_ITALIAN) "el  Greco"
LangString lng_en $(LANG_ITALIAN) "en  Inglese"
LangString lng_en_AU $(LANG_ITALIAN) "en_AU Inglese Australiano"
LangString lng_en_CA $(LANG_ITALIAN) "en_CA Inglese Canadese"
LangString lng_en_GB $(LANG_ITALIAN) "en_GB Inglese Britannico"
LangString lng_en_US@piglatin $(LANG_ITALIAN) "en_US@piglatin Pig Latin"
LangString lng_eo $(LANG_ITALIAN) "eo  Esperanto"
LangString lng_es $(LANG_ITALIAN) "es  Spagnolo"
LangString lng_es_MX $(LANG_ITALIAN) "es_MX  Spagnolo Messicano"
LangString lng_et $(LANG_ITALIAN) "et  Estone"
LangString lng_eu $(LANG_ITALIAN) "eu  Basco"
LangString lng_fi $(LANG_ITALIAN) "fi  Finlandese"
LangString lng_fr $(LANG_ITALIAN) "fr  Francese"
LangString lng_ga $(LANG_ITALIAN) "ga  Irlandese"
LangString lng_gl $(LANG_ITALIAN) "gl  Gallese"
LangString lng_he $(LANG_ITALIAN) "he  Ebreo"
LangString lng_hr $(LANG_ITALIAN) "hr  Croato"
LangString lng_hu $(LANG_ITALIAN) "hu  Ungherese"
LangString lng_id $(LANG_ITALIAN) "id  Indonesiano"
LangString lng_it $(LANG_ITALIAN) "it  Italiano"
LangString lng_ja $(LANG_ITALIAN) "ja  Giopponese"
LangString lng_km $(LANG_ITALIAN) "km  Khmer"
LangString lng_ko $(LANG_ITALIAN) "ko  Koreano"
LangString lng_lt $(LANG_ITALIAN) "lt  Lituano"
LangString lng_mk $(LANG_ITALIAN) "mk  Macedone"
LangString lng_mn $(LANG_ITALIAN) "mn  Mongolo"
LangString lng_ne $(LANG_ITALIAN) "ne  Nepali"
LangString lng_nb $(LANG_ITALIAN) "nb  Norvegese Bokmål"
LangString lng_nl $(LANG_ITALIAN) "nl  Olandese"
LangString lng_nn $(LANG_ITALIAN) "nn  Norvegese Nynorsk"
LangString lng_pa $(LANG_ITALIAN) "pa  Panjabi"
LangString lng_pl $(LANG_ITALIAN) "po  Polacco"
LangString lng_pt $(LANG_ITALIAN) "pt  Portoghese"
LangString lng_pt_BR $(LANG_ITALIAN) "pt_BR Portoghese Brasiliano"
LangString lng_ro $(LANG_ITALIAN) "ro  Rumeno"
LangString lng_ru $(LANG_ITALIAN) "ru  Russo"
LangString lng_rw $(LANG_ITALIAN) "rw  Kinyarwanda"
LangString lng_sk $(LANG_ITALIAN) "sk  Slovacco"
LangString lng_sl $(LANG_ITALIAN) "sl  Sloveno"
LangString lng_sq $(LANG_ITALIAN) "sq  Albanese"
LangString lng_sr $(LANG_ITALIAN) "sr  Serbo"
LangString lng_sr@latin $(LANG_ITALIAN) "sr@latin  Serbo in caratteri Latini"
LangString lng_sv $(LANG_ITALIAN) "sv  Svedese"
LangString lng_th $(LANG_ITALIAN) "th  Thai"
LangString lng_tr $(LANG_ITALIAN) "tr  Turco"
LangString lng_uk $(LANG_ITALIAN) "uk  Ucraino"
LangString lng_vi $(LANG_ITALIAN) "vi  Vietnamese"
LangString lng_zh_CN $(LANG_ITALIAN) "zh_CH  Cinese Semplificato"
LangString lng_zh_TW $(LANG_ITALIAN) "zh_TW  Cinese Tradizionale"




; uninstallation options
LangString lng_UInstOpt   ${LANG_ITALIAN} "Opzioni di Disinstallazione"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_ITALIAN} "Scegli ulteriori Opzioni"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_ITALIAN} "Mantieni le Impostazioni personali"

LangString lng_RETRY_CANCEL_DESC ${LANG_ITALIAN} "$\n$\nPremere Riprova per continuare o Annulla per uscire."

LangString lng_ClearDirectoryBefore ${LANG_ITALIAN} "${PRODUCT_NAME} deve essere installato in una cartella vuota. $INSTDIR non è vuota. Prima di procedere occorre rimuoverne il contenuto!$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_ITALIAN} "Impossibile trovare $INSTDIR\uninstall.log !$\r$\nPer continuare la rimozione, cancellare la cartella $INSTDIR a mano."

LangString lng_FileChanged ${LANG_ITALIAN} "Il file $filename è stato modificato rispetto all'ultima installazione.$\r$\nRimuoverlo comunque?"

LangString lng_Yes ${LANG_ITALIAN} "Sì"

LangString lng_AlwaysYes ${LANG_ITALIAN} "rispondere sempre Sì"

LangString lng_No ${LANG_ITALIAN} "No"

LangString lng_AlwaysNo ${LANG_ITALIAN} "rispondere sempre No"

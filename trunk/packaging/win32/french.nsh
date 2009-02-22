; #######################################
; french.nsh
; french language strings for inkscape installer
; windows code page: 1252
; Authors:
; Adib Taraben theAdib@googlemail.com
; matiphas matiphas@free.fr
; Nicolas Dufour nicoduf@yahoo.fr
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
; February 2009 translation update

!insertmacro MUI_LANGUAGE "French"

; Product name
LangString lng_Caption   ${LANG_FRENCH} "${PRODUCT_NAME} -- Editeur vectoriel SVG libre"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_FRENCH} "Suivant >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_FRENCH} "$(^Name) est diffusé sous la licence publique générale (GPL) GNU. La licence est fournie ici pour information uniquement. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_FRENCH} "Inkscape a déjà été installé par l'utilisateur $0.$\r$\nSi vous continuez, l'installation pourrait devenir défectueuse!$\r$\nVeuillez, svp, vous connecter en tant que $0 et essayer de nouveau."

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_FRENCH} "$R1 a déjà été installé. $\nVoulez-vous supprimer la version précédente avant l'installation d' $(^Name) ?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_FRENCH} "$\n$\nCliquer sur OK pour continuer ou CANCEL pour annuler."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_FRENCH} "Vous n'avez pas les privilèges d'administrateur.$\r$\nL'installation d'Inkscape pour tous les utilisateurs pourrait devenir défectueuse.$\r$\nVeuillez décocher l'option 'pour tous les utilisateurs'."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_FRENCH} "Inkscape n'est pas exécutable sur Windows 95/98/ME!$\r$\nVeuillez, svp, consulter les sites web officiels pour plus d'information."

; Full install type
LangString lng_Full $(LANG_FRENCH) "Complète"

; Optimal install type
LangString lng_Optimal $(LANG_FRENCH) "Optimale"

; Minimal install type
LangString lng_Minimal $(LANG_FRENCH) "Minimale"

; Core install section
LangString lng_Core $(LANG_FRENCH) "Editeur SVG ${PRODUCT_NAME} (nécessaire)"

; Core install section description
LangString lng_CoreDesc $(LANG_FRENCH) "Fichiers indispensables d'${PRODUCT_NAME} et dlls"

; GTK+ install section
LangString lng_GTKFiles $(LANG_FRENCH) "Environnement GTK+ (nécessaire)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_FRENCH) "Une boîte à outils multi-plateformes pour interfaces graphiques, utilisée par ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_FRENCH) "Raccourcis"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_FRENCH) "Raccourcis pour démarrer ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_FRENCH) "Pour tous les utilisateurs"

; All user install section description
LangString lng_AlluserDesc $(LANG_FRENCH) "Installer cette application pour tous les utilisateurs de cet ordinateurs"

; Desktop section
LangString lng_Desktop $(LANG_FRENCH) "Bureau"

; Desktop section description
LangString lng_DesktopDesc $(LANG_FRENCH) "Créer un raccourci vers ${PRODUCT_NAME} sur le bureau"

; Start Menu  section
LangString lng_Startmenu $(LANG_FRENCH) "Menu démarrer"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_FRENCH) "Créer une entrée ${PRODUCT_NAME} dans le menu démarrer"

; Quick launch section
LangString lng_Quicklaunch $(LANG_FRENCH) "Lancement rapide"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_FRENCH) "Créer un raccourci vers ${PRODUCT_NAME} dans la barre de lancement rapide"

; File type association for editing
LangString lng_SVGWriter ${LANG_FRENCH} "Ouvrir les fichiers SVG avec ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_FRENCH} "Choisir ${PRODUCT_NAME} comme éditeur par défaut pour les fichiers SVG"

; Context Menu
LangString lng_ContextMenu ${LANG_FRENCH} "Menu contextuel"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_FRENCH} "Ajouter ${PRODUCT_NAME} dans le menu contextuel des fichiers SVG"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_FRENCH} "Effacer les préférences personnelles"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_FRENCH} "Effacer les préférences personnelles laissées par les précédentes installations"


; Additional files section
LangString lng_Addfiles $(LANG_FRENCH) "Fichiers additionnels"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_FRENCH) "Fichiers additionnels"

; Examples section
LangString lng_Examples $(LANG_FRENCH) "Exemples"

; Examples section description
LangString lng_ExamplesDesc $(LANG_FRENCH) "Examples d'utilisation d'${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_FRENCH) "Didacticiels"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_FRENCH) "Didacticiels sur l'utilisation d'${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_FRENCH) "Traductions"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_FRENCH) "Installer des traductions pour ${PRODUCT_NAME}"

LangString lng_am $(LANG_FRENCH) "am  Amharique"
LangString lng_ar $(LANG_FRENCH) "ar  Arabe"
LangString lng_az $(LANG_FRENCH) "az  Azéri"
LangString lng_be $(LANG_FRENCH) "be  Biélorusse"
LangString lng_bg $(LANG_FRENCH) "bg  Bulgare"
LangString lng_bn $(LANG_FRENCH) "bn  Bengali"
LangString lng_br $(LANG_FRENCH) "br  Breton"
LangString lng_ca $(LANG_FRENCH) "ca  Catalan"
LangString lng_ca@valencia $(LANG_FRENCH) "ca@valencia Catalan Valencien"
LangString lng_cs $(LANG_FRENCH) "cs  Tchèque"
LangString lng_da $(LANG_FRENCH) "da  Danois"
LangString lng_de $(LANG_FRENCH) "de  Allemand"
LangString lng_dz $(LANG_FRENCH) "dz  Dzongkha"
LangString lng_el $(LANG_FRENCH) "el  Grec"
LangString lng_en $(LANG_FRENCH) "en  Anglais"
LangString lng_en_AU $(LANG_FRENCH) "en_AU Anglais (Australie)"
LangString lng_en_CA $(LANG_FRENCH) "en_CA Anglais (Canada)"
LangString lng_en_GB $(LANG_FRENCH) "en_GB Anglais (Grande Bretagne)"
LangString lng_en_US@piglatin $(LANG_FRENCH) "en_US@piglatin Pig Latin"
LangString lng_eo $(LANG_FRENCH) "eo  Esperanto"
LangString lng_es $(LANG_FRENCH) "es  Espagnol"
LangString lng_es_MX $(LANG_FRENCH) "es_MX  Espagnol (Mexique)"
LangString lng_et $(LANG_FRENCH) "et  Estonien"
LangString lng_eu $(LANG_FRENCH) "eu  Basque"
LangString lng_fi $(LANG_FRENCH) "fi  Finnois"
LangString lng_fr $(LANG_FRENCH) "fr  Français"
LangString lng_he $(LANG_FRENCH) "he  Hébreu"
LangString lng_ga $(LANG_FRENCH) "ga  Irlandais"
LangString lng_gl $(LANG_FRENCH) "gl  Galicien"
LangString lng_hr $(LANG_FRENCH) "hr  Croatian"
LangString lng_hu $(LANG_FRENCH) "hu  Hongrois"
LangString lng_id $(LANG_FRENCH) "id  Indonésien"
LangString lng_it $(LANG_FRENCH) "it  Italien"
LangString lng_ja $(LANG_FRENCH) "ja  Japonais"
LangString lng_km $(LANG_FRENCH) "km  Khmer"
LangString lng_ko $(LANG_FRENCH) "ko  Coréen"
LangString lng_lt $(LANG_FRENCH) "lt  Lituanien"
LangString lng_mk $(LANG_FRENCH) "mk  Macédonien"
LangString lng_mn $(LANG_FRENCH) "mn  Mongol"
LangString lng_ne $(LANG_FRENCH) "ne  Népalais"
LangString lng_nb $(LANG_FRENCH) "nb  Norvégien Bokmal"
LangString lng_nl $(LANG_FRENCH) "nl  Néerlandais"
LangString lng_nn $(LANG_FRENCH) "nn  Norvégien Nynorsk"
LangString lng_pa $(LANG_FRENCH) "pa  Pendjabi"
LangString lng_pl $(LANG_FRENCH) "po  Polonais"
LangString lng_pt $(LANG_FRENCH) "pt  Portugais"
LangString lng_pt_BR $(LANG_FRENCH) "pt_BR Portugais (Brésil)"
LangString lng_ro $(LANG_FRENCH) "ro  Roumain"
LangString lng_ru $(LANG_FRENCH) "ru  Russe"
LangString lng_rw $(LANG_FRENCH) "rw  Kinyarouandais"
LangString lng_sk $(LANG_FRENCH) "sk  Slovaque"
LangString lng_sl $(LANG_FRENCH) "sl  Slovène"
LangString lng_sq $(LANG_FRENCH) "sq  Albanian"
LangString lng_sr $(LANG_FRENCH) "sr  Serbe"
LangString lng_sr@latin $(LANG_FRENCH) "sr@latin  Serbe (notation latine)"
LangString lng_sv $(LANG_FRENCH) "sv  Suédois"
LangString lng_th $(LANG_FRENCH) "th  Thaï"
LangString lng_tr $(LANG_FRENCH) "tr  Turc"
LangString lng_uk $(LANG_FRENCH) "uk  Ukrainien"
LangString lng_vi $(LANG_FRENCH) "vi  Vietnamien"
LangString lng_zh_CN $(LANG_FRENCH) "zh_CH  Chinois"
LangString lng_zh_TW $(LANG_FRENCH) "zh_TW  Chinois (Taïwan)"




; uninstallation options
LangString lng_UInstOpt   ${LANG_FRENCH} "Options de désinstallation"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_FRENCH} "Choisissez parmi les options additionnelles"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_FRENCH} "Conserver les préférences personnelles"

LangString lng_RETRY_CANCEL_DESC ${LANG_FRENCH} "$\n$\nCliquez sur RETRY pour réessayer ou sur CANCEL pour abandonner."

LangString lng_ClearDirectoryBefore ${LANG_FRENCH} "${PRODUCT_NAME} doit être installé dans un dossier vide. $INSTDIR n'étant pas vide, prière de le nettoyer avant de recommencer.$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_FRENCH} "$INSTDIR\uninstall.log introuvable !$\r$\nVeuillez procéder à la désinstallation en nettoyant le dossier $INSTDIR manuellement."

LangString lng_FileChanged ${LANG_FRENCH} "Le fichier $filename a été modifié après l'installation.$\r$\nSouhaitez-vous vraiment le supprimer ?"

LangString lng_Yes ${LANG_FRENCH} "Oui"

LangString lng_AlwaysYes ${LANG_FRENCH} "toujours répondre Oui"

LangString lng_No ${LANG_FRENCH} "Non"

LangString lng_AlwaysNo ${LANG_FRENCH} "toujours répondre Non"

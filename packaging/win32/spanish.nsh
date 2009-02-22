; #######################################
; spanish.nsh
; spanish language strings for inkscape installer
; windows code page: 1252
; Authors:
; Adib Taraben theAdib@googlemail.com
; Lucas Vieites lucas@codexion.com
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

!insertmacro MUI_LANGUAGE "Spanish"

; Product name
LangString lng_Caption   ${LANG_SPANISH} "${PRODUCT_NAME} -- Editor de gráficos vectoriales escalables (SVG) de código abierto"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_SPANISH} "Siguiente >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_SPANISH} "$(^Name) se publica bajo la Licencia Pública General GNU (GPL). Esta licencia se muestra aquí solamente como información. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_SPANISH} "Inkscape ha sido instalado por el usuario $0.$\r$\nSi continúa, la operación podría finalizar sin éxito.$\r$\nInicie sesión como $0 y vuelva a intentarlo."

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_SPANISH} "$R1 ya ha sido instalado. $\n¿Desea eliminar la versión anterior antes de instalar $(^Name) ?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_SPANISH} "$\n$\nPulse Aceptar para continuar o Cancelar para terminar."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_SPANISH} "No dispone de privilegios de administrador.$\r$\nLa instalación de Inkscape para todos los usuarios podría terminar sin éxito.$\r$\ndesmarque la casilla «para todos los usuarios»."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_SPANISH} "Inkscape no se ejecuta correctamente en Windows 95/98/ME$\r$\nAcceda a la página web oficial si desea obtener más información."

; Full install type
LangString lng_Full $(LANG_SPANISH) "Completa"

; Optimal install type
LangString lng_Optimal $(LANG_SPANISH) "Óptima"

; Minimal install type
LangString lng_Minimal $(LANG_SPANISH) "Mínima"

; Core install section
LangString lng_Core $(LANG_SPANISH) "${PRODUCT_NAME} Editor SVG (requerido)"

; Core install section description
LangString lng_CoreDesc $(LANG_SPANISH) "Archivos ${PRODUCT_NAME} básicos y dlls"

; GTK+ install section
LangString lng_GTKFiles $(LANG_SPANISH) "GTK+ Runtime Environment (requerido)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_SPANISH) "Un conjunto de herramientas GUI, utilizado por ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_SPANISH) "Acceso directo"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_SPANISH) "Accesos directos para iniciar ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_SPANISH) "para todos los usuarios"

; All user install section description
LangString lng_AlluserDesc $(LANG_SPANISH) "Instalar esta aplicación para todos los que utilizan este equipo (todos los usuarios)"

; Desktop section
LangString lng_Desktop $(LANG_SPANISH) "Escritorio"

; Desktop section description
LangString lng_DesktopDesc $(LANG_SPANISH) "Crear un acceso directo a ${PRODUCT_NAME} en el escritorio"

; Start Menu  section
LangString lng_Startmenu $(LANG_SPANISH) "Menú Inicio"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_SPANISH) "Crear una entrada en el menú Inicio para ${PRODUCT_NAME}"

; Quick launch section
LangString lng_Quicklaunch $(LANG_SPANISH) "Acceso rápido"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_SPANISH) "Crear un acceso directo a ${PRODUCT_NAME} en la barra de acceso rápido"

; File type association for editing
LangString lng_SVGWriter ${LANG_SPANISH} "Abrir archivos SVG con ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_SPANISH} "Seleccionar ${PRODUCT_NAME} como editor predeterminado para archivos"

; Context Menu
LangString lng_ContextMenu ${LANG_SPANISH} "Menú contextual"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_SPANISH} "Añadir ${PRODUCT_NAME} al menú contextual para archivos SVG"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_SPANISH} "Eliminar preferencias personales"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_SPANISH} "Eliminar las preferencias personales restantes de instalaciones anteriores."

; Additional files section
LangString lng_Addfiles $(LANG_SPANISH) "Archivos adicionales"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_SPANISH) "Archivos adicionales"

; Examples section
LangString lng_Examples $(LANG_SPANISH) "Ejemplos"

; Examples section description
LangString lng_ExamplesDesc $(LANG_SPANISH) "Ejemplos de uso de ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_SPANISH) "Tutoriales"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_SPANISH) "Tutoriales del uso de ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_SPANISH) "Traducciones"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_SPANISH) "Instalar varias traducciones para ${PRODUCT_NAME}"

LangString lng_am $(LANG_SPANISH) "am  Amárico"
LangString lng_ar $(LANG_SPANISH) "ar  Árabe"
LangString lng_az $(LANG_SPANISH) "az  Azerbayano"
LangString lng_be $(LANG_SPANISH) "be  Bielorruso"
LangString lng_bg $(LANG_SPANISH) "bg  Búlgaro"
LangString lng_bn $(LANG_SPANISH) "bn  Bengalí"
LangString lng_br $(LANG_SPANISH) "br  Bretón"
LangString lng_ca $(LANG_SPANISH) "ca  Catalán"
LangString lng_ca@valencia $(LANG_SPANISH) "ca@valencia  Valenciano"
LangString lng_cs $(LANG_SPANISH) "cs  Checo"
LangString lng_da $(LANG_SPANISH) "da  Dané"
LangString lng_de $(LANG_SPANISH) "de  Alemán"
LangString lng_dz $(LANG_SPANISH) "dz  Dzongkha"
LangString lng_el $(LANG_SPANISH) "el  Griego"
LangString lng_en $(LANG_SPANISH) "en  Inglés"
LangString lng_en_AU $(LANG_SPANISH) "en_AU  Inglés australiano"
LangString lng_en_CA $(LANG_SPANISH) "en_CA  Inglés canadiense"
LangString lng_en_GB $(LANG_SPANISH) "en_GB  Inglés británico"
LangString lng_en_US@piglatin $(LANG_SPANISH) "en_US@piglatin Pig Latin"
LangString lng_eo $(LANG_SPANISH) "eo  Esperanto"
LangString lng_es $(LANG_SPANISH) "es  Español"
LangString lng_es_MX $(LANG_SPANISH) "es_MX  Español mexicano"
LangString lng_et $(LANG_SPANISH) "et  Estonio"
LangString lng_eu $(LANG_SPANISH) "eu  Euskera"
LangString lng_fi $(LANG_SPANISH) "fi  Finés"
LangString lng_fr $(LANG_SPANISH) "fr  Francés"
LangString lng_ga $(LANG_SPANISH) "ga  Irlandés"
LangString lng_gl $(LANG_SPANISH) "gl  Galego"
LangString lng_he $(LANG_SPANISH) "he  Hebreo"
LangString lng_hr $(LANG_SPANISH) "hr  Croata"
LangString lng_hu $(LANG_SPANISH) "hu  Húngaro"
LangString lng_id $(LANG_SPANISH) "id  Indonesio"
LangString lng_it $(LANG_SPANISH) "it  Italiano"
LangString lng_ja $(LANG_SPANISH) "ja  Japonés"
LangString lng_km $(LANG_SPANISH) "km  Jemer"
LangString lng_ko $(LANG_SPANISH) "ko  Coreano"
LangString lng_lt $(LANG_SPANISH) "lt  Lituano"
LangString lng_mk $(LANG_SPANISH) "mk  Macedonio"
LangString lng_mn $(LANG_SPANISH) "mn  Mongol"
LangString lng_ne $(LANG_SPANISH) "ne  Nepalí"
LangString lng_nb $(LANG_SPANISH) "nb  Bokmål noruego"
LangString lng_nl $(LANG_SPANISH) "nl  Holandés"
LangString lng_nn $(LANG_SPANISH) "nn  Nynorsk noruego"
LangString lng_pa $(LANG_SPANISH) "pa  Punyabi"
LangString lng_pl $(LANG_SPANISH) "po  Polaco"
LangString lng_pt $(LANG_SPANISH) "pt  Portugués"
LangString lng_pt_BR $(LANG_SPANISH) "pt_BR Portugués brasileño"
LangString lng_ro $(LANG_SPANISH) "ro  Rumano"
LangString lng_ru $(LANG_SPANISH) "ru  Ruso"
LangString lng_rw $(LANG_SPANISH) "rw  Kinyarwanda"
LangString lng_sk $(LANG_SPANISH) "sk  Eslovaco"
LangString lng_sl $(LANG_SPANISH) "sl  Esloveno"
LangString lng_sq $(LANG_SPANISH) "sq  Albano"
LangString lng_sr $(LANG_SPANISH) "sr  Serbio"
LangString lng_sr@latin $(LANG_SPANISH) "sr@latin  Serbio en escritura latina"
LangString lng_sv $(LANG_SPANISH) "sv  Sueco"
LangString lng_th $(LANG_SPANISH) "th  Tai"
LangString lng_tr $(LANG_SPANISH) "tr  Turco"
LangString lng_uk $(LANG_SPANISH) "uk  Ucranio"
LangString lng_vi $(LANG_SPANISH) "vi  Vietnamita"
LangString lng_zh_CN $(LANG_SPANISH) "zh_CH  Chino simplificado"
LangString lng_zh_TW $(LANG_SPANISH) "zh_TW  Chino taiwanés"




; uninstallation options
LangString lng_UInstOpt   ${LANG_SPANISH} "Opciones de desinstalación"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_SPANISH} "Elija sus opciones adicionales"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_SPANISH} "Guardar las preferencias personales"

LangString lng_RETRY_CANCEL_DESC ${LANG_SPANISH} "$\n$\nPulse Reintentar para continuar o Cancelar para terminar."

LangString lng_ClearDirectoryBefore ${LANG_SPANISH} "${PRODUCT_NAME} debe ser instalado en un directorio vacío. $INSTDIR no está vacío. Limpe este directorio antes de continuar.$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_SPANISH} "No se ha encontrado $INSTDIR\uninstall.log.$\r$\nDesinstale limpiando el directorio $INSTDIR."

LangString lng_FileChanged ${LANG_SPANISH} "El archivo $filename ha sido cambiado después de la instalación.$\r$\n¿Está seguro de que desea eliminar este archivo?"

LangString lng_Yes ${LANG_SPANISH} "Sí"

LangString lng_AlwaysYes ${LANG_SPANISH} "siempre responder Sí"

LangString lng_No ${LANG_SPANISH} "No"

LangString lng_AlwaysNo ${LANG_SPANISH} "siempre responder No"

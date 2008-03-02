; #######################################
; russian.nsh
; russian language strings for inkscape installer
; windows code page: 1251
; Authors:
; Alexandre Prokoudine alexandre.prokoudine@gmail.com
;
; 27 july 2006 new languages en_CA, en_GB, fi, hr, mn, ne, rw, sq
; 11 august 2006 new languages dz bg
; 24 october 2006 new languages en_US@piglatin, th
; 3rd December 2006 new languages eu km
; 14th December 2006 new lng_DeletePrefs, lng_DeletePrefsDesc, lng_WANT_UNINSTALL_BEFORE and lng_OK_CANCEL_DESC; 2nd February 2007 new language ru
; february 15 2007 new language bn, en_AU, eo, id, ro
; april 11 2007 new language he
; october 2007 new language ca@valencian
; January 2008 new uninstaller messages
; February 2008 new languages ar, br

!insertmacro MUI_LANGUAGE "Russian"

; Product name
LangString lng_Caption   ${LANG_RUSSIAN} "${PRODUCT_NAME} -- Редактор векторной графики с открытым исходным кодом"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_RUSSIAN} "Дальше >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_RUSSIAN} "$(^Name) выпущен на условиях GNU General Public License (GPL). Лицензия предлагается для ознакомления. $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_RUSSIAN} "Inkscape установлен пользователем $0.$\r$\nЕсли вы продолжите, установка может не завершиться успешно!$\r$\nВойдите в систему как пользователь $0 и попробуйте снова."

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_RUSSIAN} "$R1 уже установлена. $\nВы хотите удалить предыдущую версию перед установкой $(^Name) ?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_RUSSIAN} "$\n$\nНажмите OK для продолжения или CANCEL для прерывания установки."

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_RUSSIAN} "У вас нет прав администратора.$\r$\nУстановка Inkscape для всех пользователей может не завершиться успешно.$\r$\nНе используйте параметр «Для всех пользователей»."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_RUSSIAN} "Inkscape не работает в Windows 95/98/ME!$\r$\nПодробности изложены на сайте программы."

; Full install type
LangString lng_Full $(LANG_RUSSIAN) "Полная"

; Optimal install type
LangString lng_Optimal $(LANG_RUSSIAN) "Оптимальная"

; Minimal install type
LangString lng_Minimal $(LANG_RUSSIAN) "Минимальная"

; Core install section
LangString lng_Core $(LANG_RUSSIAN) "${PRODUCT_NAME}, редактор SVG (требуется)"

; Core install section description
LangString lng_CoreDesc $(LANG_RUSSIAN) "Основные файлы и библиотеки ${PRODUCT_NAME}"

; GTK+ install section
LangString lng_GTKFiles $(LANG_RUSSIAN) "Среда исполнения GTK+ (требуется)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_RUSSIAN) "Многоплатформенные средства разработки, необходимые для ${PRODUCT_NAME}"

; shortcuts install section
LangString lng_Shortcuts $(LANG_RUSSIAN) "Ярлыки"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_RUSSIAN) "Ярлыки для запуска ${PRODUCT_NAME}"

; All user install section
LangString lng_Alluser $(LANG_RUSSIAN) "Для всех пользователей"

; All user install section description
LangString lng_AlluserDesc $(LANG_RUSSIAN) "Установить программу для всех пользователей этого компьютера"

; Desktop section
LangString lng_Desktop $(LANG_RUSSIAN) "Рабочий стол"

; Desktop section description
LangString lng_DesktopDesc $(LANG_RUSSIAN) "Создать ярлык для ${PRODUCT_NAME} на Рабочем столе"

; Start Menu  section
LangString lng_Startmenu $(LANG_RUSSIAN) "Меню «Пуск»"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_RUSSIAN) "Создать группу ${PRODUCT_NAME} в меню «Пуск»"

; Quick launch section
LangString lng_Quicklaunch $(LANG_RUSSIAN) "Панель быстрого запуска"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_RUSSIAN) "Создать ярлык для ${PRODUCT_NAME} в панели быстрого запуска"

; File type association for editing
LangString lng_SVGWriter ${LANG_RUSSIAN} "Открывать файлы SVG в ${PRODUCT_NAME}"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_RUSSIAN} "Выбрать ${PRODUCT_NAME} редактором файлов SVG по умолчанию"

; Context Menu
LangString lng_ContextMenu ${LANG_RUSSIAN} "Контекстное меню"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_RUSSIAN} "Добавить ${PRODUCT_NAME} в контекстное меню для файлов SVG"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_RUSSIAN} "Удалить личные настройки"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_RUSSIAN} "Удалить личные настройки, оставшиеся от предыдущих версий программы"


; Additional files section
LangString lng_Addfiles $(LANG_RUSSIAN) "Дополнительные файлы"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_RUSSIAN) "Дополнительные файлы"

; Examples section
LangString lng_Examples $(LANG_RUSSIAN) "Примеры"

; Examples section description
LangString lng_ExamplesDesc $(LANG_RUSSIAN) "Примеры файлов, созданных в ${PRODUCT_NAME}"

; Tutorials section
LangString lng_Tutorials $(LANG_RUSSIAN) "Уроки"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_RUSSIAN) "Уроки по использованию ${PRODUCT_NAME}"


; Languages section
LangString lng_Languages $(LANG_RUSSIAN) "Переводы"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_RUSSIAN) "Установка переводов ${PRODUCT_NAME} на разные языки"

LangString lng_am $(LANG_RUSSIAN) "am  Амхарский (Амаринья)"
LangString lng_ar $(LANG_RUSSIAN) "ar  Arabic"
LangString lng_az $(LANG_RUSSIAN) "az  Азербайджанский"
LangString lng_be $(LANG_RUSSIAN) "be  Белорусский"
LangString lng_bg $(LANG_RUSSIAN) "bg  Болгарский"
LangString lng_bn $(LANG_RUSSIAN) "bn  Bengali"
LangString lng_br $(LANG_RUSSIAN) "br  Breton"
LangString lng_ca $(LANG_RUSSIAN) "ca  Каталанский"
LangString lng_ca@valencia $(LANG_RUSSIAN) "ca@valencia  Valencian Catalan"
LangString lng_cs $(LANG_RUSSIAN) "cs  Чешский"
LangString lng_da $(LANG_RUSSIAN) "da  Датский"
LangString lng_de $(LANG_RUSSIAN) "de  Немецкий"
LangString lng_dz $(LANG_RUSSIAN) "dz  Дзонг-кэ"
LangString lng_el $(LANG_RUSSIAN) "el  Греческий"
LangString lng_en $(LANG_RUSSIAN) "en  Английский"
LangString lng_en_AU $(LANG_RUSSIAN) "en_AU Australian English"
LangString lng_en_CA $(LANG_RUSSIAN) "en_CA Английский (Канада)"
LangString lng_en_GB $(LANG_RUSSIAN) "en_GB Английский (Великобритания)"
LangString lng_en_US@piglatin $(LANG_RUSSIAN) "en_US@piglatin Поросячья латынь"
LangString lng_eo $(LANG_RUSSIAN) "eo  Esperanto"
LangString lng_es $(LANG_RUSSIAN) "es  Испанский"
LangString lng_es_MX $(LANG_RUSSIAN) "es_MX  Испанский (Мексика)"
LangString lng_et $(LANG_RUSSIAN) "et  Эстонский"
LangString lng_eu $(LANG_RUSSIAN) "eu  Баскский"
LangString lng_fi $(LANG_RUSSIAN) "fi  Финский"
LangString lng_fr $(LANG_RUSSIAN) "fr  Французский"
LangString lng_ga $(LANG_RUSSIAN) "ga  Ирландский"
LangString lng_gl $(LANG_RUSSIAN) "gl  Галисийский"
LangString lng_he $(LANG_RUSSIAN) "he  Hebrew"
LangString lng_hr $(LANG_RUSSIAN) "hr  Хорватский"
LangString lng_hu $(LANG_RUSSIAN) "hu  Венгерский"
LangString lng_id $(LANG_RUSSIAN) "id  Indonesian"
LangString lng_it $(LANG_RUSSIAN) "it  Итальянский"
LangString lng_ja $(LANG_RUSSIAN) "ja  Японский"
LangString lng_km $(LANG_RUSSIAN) "km  Кхмерский"
LangString lng_ko $(LANG_RUSSIAN) "ko  Корейский"
LangString lng_lt $(LANG_RUSSIAN) "lt  Литовский"
LangString lng_mk $(LANG_RUSSIAN) "mk  Македонский"
LangString lng_mn $(LANG_RUSSIAN) "mn  Монгольский"
LangString lng_ne $(LANG_RUSSIAN) "ne  Непальский"
LangString lng_nb $(LANG_RUSSIAN) "nb  Норвежский (букмол)"
LangString lng_nl $(LANG_RUSSIAN) "nl  Датский"
LangString lng_nn $(LANG_RUSSIAN) "nn  Норвежский (нюнорск)"
LangString lng_pa $(LANG_RUSSIAN) "pa  Пенджабский"
LangString lng_pl $(LANG_RUSSIAN) "po  Польский"
LangString lng_pt $(LANG_RUSSIAN) "pt  Португальский"
LangString lng_pt_BR $(LANG_RUSSIAN) "pt_BR Португальский (Бразилия)"
LangString lng_ro $(LANG_RUSSIAN) "ro  Romanian"
LangString lng_ru $(LANG_RUSSIAN) "ru  Русский"
LangString lng_rw $(LANG_RUSSIAN) "rw  Киньяруанда"
LangString lng_sk $(LANG_RUSSIAN) "sk  Словацкий"
LangString lng_sl $(LANG_RUSSIAN) "sl  Словенский"
LangString lng_sq $(LANG_RUSSIAN) "sq  Албанский"
LangString lng_sr $(LANG_RUSSIAN) "sr  Сербский"
LangString lng_sr@latin $(LANG_RUSSIAN) "sr@latin  Сербский (латиница)"
LangString lng_sv $(LANG_RUSSIAN) "sv  Шведский"
LangString lng_th $(LANG_RUSSIAN) "th  Тайский"
LangString lng_tr $(LANG_RUSSIAN) "tr  Турецкий"
LangString lng_uk $(LANG_RUSSIAN) "uk  Украинский"
LangString lng_vi $(LANG_RUSSIAN) "vi  Вьетнамский"
LangString lng_zh_CN $(LANG_RUSSIAN) "zh_CH  Китайский упрощённый"
LangString lng_zh_TW $(LANG_RUSSIAN) "zh_TW  Китайский традиционный"




; uninstallation options
LangString lng_UInstOpt   ${LANG_RUSSIAN} "Параметры удаления программы из системы"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_RUSSIAN} "Убедитесь в том, что указали дополнительные параметры"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_RUSSIAN} "Сохранить личные настройки"

LangString lng_RETRY_CANCEL_DESC ${LANG_RUSSIAN} "$\n$\nPress RETRY to continue or press CANCEL to abort."

LangString lng_ClearDirectoryBefore ${LANG_RUSSIAN} "${PRODUCT_NAME} must be installed in an empty directory. $INSTDIR is not empty. Please clear this directory first!$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_RUSSIAN} "$INSTDIR\uninstall.log not found!$\r$\nPlease uninstall by clearing directory $INSTDIR yourself!"

LangString lng_FileChanged ${LANG_RUSSIAN} "The file $filename has been changed after installation.$\r$\nDo you still want to delete that file?"

LangString lng_Yes ${LANG_RUSSIAN} "Yes"

LangString lng_AlwaysYes ${LANG_RUSSIAN} "always answer Yes"

LangString lng_No ${LANG_RUSSIAN} "No"

LangString lng_AlwaysNo ${LANG_RUSSIAN} "always answer No"

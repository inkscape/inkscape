
; #######################################
; tradchinese.nsh
; Traditional Chinese language strings for inkscape installer
; windows code page: 950
; Authors:
; Adib Taraben theAdib@googlemail.com
; Dong-Jun Wu <ziyawu@gmail.com>, 2009.
;

!insertmacro MUI_LANGUAGE "TradChinese"

; Product name
LangString lng_Caption   ${LANG_TRADCHINESE} "${PRODUCT_NAME} -- 開放原始碼向量繪圖軟體"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_TRADCHINESE} "下一步 >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_TRADCHINESE} "$(^Name) 是以 GNU 通用公共許可證 (GPL) 發行。 這裡提供的許可證僅為參考訊息。 $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_TRADCHINESE} "使用者 $0 已經安裝 Inkscape。$\r$\n如果繼續你可能無法成功完成！$\r$\n請以 $0 身份登入後再試一次。"

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_TRADCHINESE} "$R1 已經安裝。 $\n你要在安裝 $(^Name) 之前移除上一個版本？"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_TRADCHINESE} "$\n$\n按「確定」繼續或者按「取消」中止。"

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_TRADCHINESE} "你沒有系統管理員權限。$\r$\n安裝 Inkscape 到所有使用者可能無法成功完成。$\r$\n取消勾選「安裝給所有使用者」選項."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_TRADCHINESE} "Inkscape 無法於 Windows 95/98/ME 下運行!$\r$\n請查看官方網站的詳細信息。"

; Full install type
LangString lng_Full $(LANG_TRADCHINESE) "完整"

; Optimal install type
LangString lng_Optimal $(LANG_TRADCHINESE) "理想"

; Minimal install type
LangString lng_Minimal $(LANG_TRADCHINESE) "最小"

; Core install section
LangString lng_Core $(LANG_TRADCHINESE) "${PRODUCT_NAME} SVG 編輯程式(必須)"

; Core install section description
LangString lng_CoreDesc $(LANG_TRADCHINESE) "${PRODUCT_NAME} 核心檔案和 DLL 檔"

; GTK+ install section
LangString lng_GTKFiles $(LANG_TRADCHINESE) "GTK+ 執行環境(必須)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_TRADCHINESE) "${PRODUCT_NAME} 使用的跨平台 GUI 工具組"

; shortcuts install section
LangString lng_Shortcuts $(LANG_TRADCHINESE) "捷徑"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_TRADCHINESE) "啟動 ${PRODUCT_NAME} 的捷徑"

; All user install section
LangString lng_Alluser $(LANG_TRADCHINESE) "安裝給所有使用者"

; All user install section description
LangString lng_AlluserDesc $(LANG_TRADCHINESE) "安裝這個應用程式給任何使用這台電腦的人(所有使用者)"

; Desktop section
LangString lng_Desktop $(LANG_TRADCHINESE) "桌面"

; Desktop section description
LangString lng_DesktopDesc $(LANG_TRADCHINESE) "於桌面上建立 ${PRODUCT_NAME} 捷徑"

; Start Menu  section
LangString lng_Startmenu $(LANG_TRADCHINESE) "開始功能表"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_TRADCHINESE) "於開始功能表建立 ${PRODUCT_NAME} 項目"

; Quick launch section
LangString lng_Quicklaunch $(LANG_TRADCHINESE) "快速啟動"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_TRADCHINESE) "於快速啟動列建立 ${PRODUCT_NAME} 捷徑"

; File type association for editing
LangString lng_SVGWriter ${LANG_TRADCHINESE} "用 ${PRODUCT_NAME} 開啟 SVG 檔"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_TRADCHINESE} "選擇 ${PRODUCT_NAME} 作為 SVG 檔的預設編輯程式"

; Context Menu
LangString lng_ContextMenu ${LANG_TRADCHINESE} "右鍵功能表"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_TRADCHINESE} "新增 ${PRODUCT_NAME} 到 SVG 檔的右鍵功能表"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_TRADCHINESE} "刪除個人偏好設定"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_TRADCHINESE} "刪除上一次安裝遺留的個人偏好設定"


; Additional files section
LangString lng_Addfiles $(LANG_TRADCHINESE) "其他檔案"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_TRADCHINESE) "其他檔案"

; Examples section
LangString lng_Examples $(LANG_TRADCHINESE) "範例"

; Examples section description
LangString lng_ExamplesDesc $(LANG_TRADCHINESE) "${PRODUCT_NAME} 使用範例"

; Tutorials section
LangString lng_Tutorials $(LANG_TRADCHINESE) "指導手冊"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_TRADCHINESE) "${PRODUCT_NAME} 使用教學"


; Languages section
LangString lng_Languages $(LANG_TRADCHINESE) "語言"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_TRADCHINESE) "安裝 ${PRODUCT_NAME} 各種語言翻譯"

LangString lng_am $(LANG_TRADCHINESE) "am  阿比西尼亞語"
LangString lng_ar $(LANG_TRADCHINESE) "ar  阿拉伯語"
LangString lng_az $(LANG_TRADCHINESE) "az  亞塞拜然語"
LangString lng_be $(LANG_TRADCHINESE) "be  白俄羅斯語"
LangString lng_bg $(LANG_TRADCHINESE) "bg  保加利亞語"
LangString lng_bn $(LANG_TRADCHINESE) "bn  孟加拉語"
LangString lng_br $(LANG_TRADCHINESE) "br  不列塔尼語"
LangString lng_ca $(LANG_TRADCHINESE) "ca  加泰隆語"
LangString lng_ca@valencia $(LANG_TRADCHINESE) "ca@valencia 瓦倫西亞語 加泰羅尼亞語"
LangString lng_cs $(LANG_TRADCHINESE) "cs  捷克語"
LangString lng_da $(LANG_TRADCHINESE) "da  丹麥語"
LangString lng_de $(LANG_TRADCHINESE) "de  德語"
LangString lng_dz $(LANG_TRADCHINESE) "dz  宗卡語"
LangString lng_el $(LANG_TRADCHINESE) "el  希臘語"
LangString lng_en $(LANG_TRADCHINESE) "en  英語"
LangString lng_en_AU $(LANG_TRADCHINESE) "en_AU 英語(澳大利亞)"
LangString lng_en_CA $(LANG_TRADCHINESE) "en_CA 英語(加拿大)"
LangString lng_en_GB $(LANG_TRADCHINESE) "en_GB 英語(不列顛)"
LangString lng_en_US@piglatin $(LANG_TRADCHINESE) "en_US@piglatin 豬拉丁語"
LangString lng_eo $(LANG_TRADCHINESE) "eo  世界語"
LangString lng_es $(LANG_TRADCHINESE) "es  西班牙文"
LangString lng_es_MX $(LANG_TRADCHINESE) "es_MX 西班牙語(墨西哥)"
LangString lng_et $(LANG_TRADCHINESE) "et  愛沙尼亞語"
LangString lng_eu $(LANG_TRADCHINESE) "eu  巴斯克語"
LangString lng_fi $(LANG_TRADCHINESE) "fi  芬蘭語"
LangString lng_fr $(LANG_TRADCHINESE) "fr  法文"
LangString lng_ga $(LANG_TRADCHINESE) "ga  愛爾蘭語"
LangString lng_gl $(LANG_TRADCHINESE) "gl  加里西亞語"
LangString lng_he $(LANG_TRADCHINESE) "he  希伯來語"
LangString lng_hr $(LANG_TRADCHINESE) "hr  克羅埃西亞語"
LangString lng_hu $(LANG_TRADCHINESE) "hu  匈牙利語"
LangString lng_id $(LANG_TRADCHINESE) "id  印尼語"
LangString lng_it $(LANG_TRADCHINESE) "it  義大利文"
LangString lng_ja $(LANG_TRADCHINESE) "ja  日文"
LangString lng_km $(LANG_TRADCHINESE) "km  高棉語"
LangString lng_ko $(LANG_TRADCHINESE) "ko  韓文"
LangString lng_lt $(LANG_TRADCHINESE) "lt  立陶宛語"
LangString lng_mk $(LANG_TRADCHINESE) "mk  馬其頓語"
LangString lng_mn $(LANG_TRADCHINESE) "mn  蒙古語"
LangString lng_ne $(LANG_TRADCHINESE) "ne  尼泊爾語"
LangString lng_nb $(LANG_TRADCHINESE) "nb  挪威 Bokmal 語"
LangString lng_nl $(LANG_TRADCHINESE) "nl  荷蘭語"
LangString lng_nn $(LANG_TRADCHINESE) "nn  挪威 Nynorsk 語"
LangString lng_pa $(LANG_TRADCHINESE) "pa  旁遮普語"
LangString lng_pl $(LANG_TRADCHINESE) "po  波蘭語"
LangString lng_pt $(LANG_TRADCHINESE) "pt  葡萄牙文"
LangString lng_pt_BR $(LANG_TRADCHINESE) "pt_BR 葡萄牙文(巴西)"
LangString lng_ro $(LANG_TRADCHINESE) "ro  羅馬尼亞語"
LangString lng_ru $(LANG_TRADCHINESE) "ru  俄文"
LangString lng_rw $(LANG_TRADCHINESE) "rw  金亞盧安達語"
LangString lng_sk $(LANG_TRADCHINESE) "sk  斯洛法克語"
LangString lng_sl $(LANG_TRADCHINESE) "sl  斯洛凡尼亞語"
LangString lng_sq $(LANG_TRADCHINESE) "sq  阿爾巴尼亞語"
LangString lng_sr $(LANG_TRADCHINESE) "sr  賽爾維亞語"
LangString lng_sr@latin $(LANG_TRADCHINESE) "sr@latin 塞爾維亞文-拉丁"
LangString lng_sv $(LANG_TRADCHINESE) "sv  瑞典語"
LangString lng_th $(LANG_TRADCHINESE) "th  泰語"
LangString lng_tr $(LANG_TRADCHINESE) "tr  土耳其語"
LangString lng_uk $(LANG_TRADCHINESE) "uk  烏克蘭語"
LangString lng_vi $(LANG_TRADCHINESE) "vi  越南話"
LangString lng_zh_CN $(LANG_TRADCHINESE) "zh_CH  簡體中文"
LangString lng_zh_TW $(LANG_TRADCHINESE) "zh_TW  繁體中文"




; uninstallation options
LangString lng_UInstOpt   ${LANG_TRADCHINESE} "反安裝選項"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_TRADCHINESE} "請選擇其他選項"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_TRADCHINESE} "保留個人偏好設定"

LangString lng_RETRY_CANCEL_DESC ${LANG_TRADCHINESE} "$\n$\n按「重試」繼續或者按「取消」中止。"

LangString lng_ClearDirectoryBefore ${LANG_TRADCHINESE} "${PRODUCT_NAME} 必安裝於一個空的資料夾。 $INSTDIR 不是空的。 請先清空這個資料夾！$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_TRADCHINESE} "沒有找到 $INSTDIR\uninstall.log ！$\r$\n請自行清除 $INSTDIR 資料夾來反安裝！"

LangString lng_FileChanged ${LANG_TRADCHINESE} "安裝後 $filename 檔案已變更。$\r$\n你是否仍然要刪除那個檔案？"

LangString lng_Yes ${LANG_TRADCHINESE} "是"

LangString lng_AlwaysYes ${LANG_TRADCHINESE} "全部皆是"

LangString lng_No ${LANG_TRADCHINESE} "否"

LangString lng_AlwaysNo ${LANG_TRADCHINESE} "全部皆否"

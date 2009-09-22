; #######################################
; japanese.nsh
; japanese language strings for inkscape installer
; windows code page: 932
; Authors:
; Kenji Inoue kenz@oct.zaq.ne.jp
; Masato Hashimoto cabezon.hashimoto@gmail.com, 2009. 
;
; february 15 2007 new language bn, en_AU, eo, id, ro
; april 11 2007 new language he
; october 2007 new language ca@valencian
; January 2008 new uninstaller messages
; February 2008 new languages ar, br

!insertmacro MUI_LANGUAGE "Japanese"

; Product name
LangString lng_Caption   ${LANG_JAPANESE} "${PRODUCT_NAME} -- Open Source Scalable Vector Graphics Editor"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_JAPANESE} "次へ >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_JAPANESE} "$(^Name) は GNU General Public License (GPL) の下でリリースされます。参考に当該ライセンスをここに表示します。$_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_JAPANESE} "Inkscape はユーザ $0 によってインストールされています。$\r$\nこのまま続けると正常に完了しないかもしれません。$\r$\n$0 でログインしてから再度試みてください。"

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_JAPANESE} "$R1 は既にインストールされています。$\n$(^Name) をインストールする前に以前のバージョンを削除しますか?"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_JAPANESE} "$\n$\nOK を押して継続するか CANCEL を押して中止してください。"

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_JAPANESE} "管理者権限がありません。$\r$\nすべてのユーザに対する Inkscape のインストールは正常に完了しないかもしれません。$\r$\n'すべてのユーザ' オプションのチェックマークを外してください。"

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_JAPANESE} "Inkscape は Windows 95/98/ME 上では動作しません!$\r$\n詳しくはオフィシャルウェブサイトをご覧ください。"

; Full install type
LangString lng_Full $(LANG_JAPANESE) "完全"

; Optimal install type
LangString lng_Optimal $(LANG_JAPANESE) "最適"

; Minimal install type
LangString lng_Minimal $(LANG_JAPANESE) "最小"

; Core install section
LangString lng_Core $(LANG_JAPANESE) "${PRODUCT_NAME} SVG Editor (必須)"

; Core install section description
LangString lng_CoreDesc $(LANG_JAPANESE) "${PRODUCT_NAME} のコアファイルとDLL"

; GTK+ install section
LangString lng_GTKFiles $(LANG_JAPANESE) "GTK+ ランタイム環境 (必須)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_JAPANESE) "マルチプラットフォーム対応 GUI ツールキット (${PRODUCT_NAME} が使用)"

; shortcuts install section
LangString lng_Shortcuts $(LANG_JAPANESE) "ショートカット"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_JAPANESE) "${PRODUCT_NAME} を開始するためのショートカット"

; All user install section
LangString lng_Alluser $(LANG_JAPANESE) "すべてのユーザ"

; All user install section description
LangString lng_AlluserDesc $(LANG_JAPANESE) "このコンピュータを使うすべての人にこのアプリケーションをインストール (すべてのユーザ)"

; Desktop section
LangString lng_Desktop $(LANG_JAPANESE) "デスクトップ"

; Desktop section description
LangString lng_DesktopDesc $(LANG_JAPANESE) "${PRODUCT_NAME} へのショートカットをデスクトップに作成"

; Start Menu  section
LangString lng_Startmenu $(LANG_JAPANESE) "スタートメニュー"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_JAPANESE) "スタートメニューに ${PRODUCT_NAME} の項目を作成"

; Quick launch section
LangString lng_Quicklaunch $(LANG_JAPANESE) "クイック起動"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_JAPANESE) "${PRODUCT_NAME} へのショートカットをクイック起動ツールバーに作成"

; File type association for editing
LangString lng_SVGWriter ${LANG_JAPANESE} "SVG ファイルを ${PRODUCT_NAME} で開く"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_JAPANESE} "SVG ファイルの標準エディタに ${PRODUCT_NAME} を設定"

; Context Menu
LangString lng_ContextMenu ${LANG_JAPANESE} "コンテキストメニュー"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_JAPANESE} "SVG ファイルのコンテキストメニューに ${PRODUCT_NAME} を追加"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_JAPANESE} "個人設定を削除"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_JAPANESE} "以前のインストール時から引き継いだ個人設定を削除"


; Additional files section
LangString lng_Addfiles $(LANG_JAPANESE) "追加ファイル"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_JAPANESE) "追加ファイル"

; Examples section
LangString lng_Examples $(LANG_JAPANESE) "サンプルファイル"

; Examples section description
LangString lng_ExamplesDesc $(LANG_JAPANESE) "${PRODUCT_NAME} のサンプルファイル"

; Tutorials section
LangString lng_Tutorials $(LANG_JAPANESE) "チュートリアル"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_JAPANESE) "${PRODUCT_NAME} のチュートリアル"


; Languages section
LangString lng_Languages $(LANG_JAPANESE) "国際化"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_JAPANESE) "${PRODUCT_NAME} のさまざまな言語のファイルをインストール"

LangString lng_am $(LANG_JAPANESE) "am  アムハラ語"
LangString lng_ar $(LANG_JAPANESE) "ar  アラビア語"
LangString lng_az $(LANG_JAPANESE) "az  アゼルバイジャン語"
LangString lng_be $(LANG_JAPANESE) "be  ベラルーシ語"
LangString lng_bg $(LANG_JAPANESE) "bg  ブルガリア語"
LangString lng_bn $(LANG_JAPANESE) "bn  ベンガル語"
LangString lng_br $(LANG_JAPANESE) "br  ブルトン語"
LangString lng_ca $(LANG_JAPANESE) "ca  カタロニア語"
LangString lng_ca@valencia $(LANG_JAPANESE) "ca@valencia  バレンシア語"
LangString lng_cs $(LANG_JAPANESE) "cs  チェコ語"
LangString lng_da $(LANG_JAPANESE) "da  デンマーク語"
LangString lng_de $(LANG_JAPANESE) "de  ドイツ語"
LangString lng_dz $(LANG_JAPANESE) "dz  ゾンカ語"
LangString lng_el $(LANG_JAPANESE) "el  ギリシャ語"
LangString lng_en $(LANG_JAPANESE) "en  英語"
LangString lng_en_AU $(LANG_JAPANESE) "en_AU 英語 (オーストラリア)"
LangString lng_en_CA $(LANG_JAPANESE) "en_CA 英語 (カナダ)"
LangString lng_en_GB $(LANG_JAPANESE) "en_GB 英語 (英国)"
LangString lng_en_US@piglatin $(LANG_JAPANESE) "en_US@piglatin ピッグ・ラテン語"
LangString lng_eo $(LANG_JAPANESE) "eo  エスペラント語"
LangString lng_es $(LANG_JAPANESE) "es  スペイン語"
LangString lng_es_MX $(LANG_JAPANESE) "es_MX  スペイン語 (メキシコ)"
LangString lng_et $(LANG_JAPANESE) "et  エストニア語"
LangString lng_eu $(LANG_JAPANESE) "eu  バスク語"
LangString lng_fi $(LANG_JAPANESE) "fi  フィンランド語"
LangString lng_fr $(LANG_JAPANESE) "fr  フランス語"
LangString lng_ga $(LANG_JAPANESE) "ga  アイルランド語"
LangString lng_gl $(LANG_JAPANESE) "gl  ガリシア語"
LangString lng_he $(LANG_JAPANESE) "he  ヘブライ語"
LangString lng_hr $(LANG_JAPANESE) "hr  クロアチア語"
LangString lng_hu $(LANG_JAPANESE) "hu  ハンガリー語"
LangString lng_id $(LANG_JAPANESE) "id  インドネシア語"
LangString lng_it $(LANG_JAPANESE) "it  イタリア語"
LangString lng_ja $(LANG_JAPANESE) "ja  日本語"
LangString lng_km $(LANG_JAPANESE) "km  クメール語"
LangString lng_ko $(LANG_JAPANESE) "ko  韓国語"
LangString lng_lt $(LANG_JAPANESE) "lt  リトアニア語"
LangString lng_mk $(LANG_JAPANESE) "mk  マケドニア語"
LangString lng_mn $(LANG_JAPANESE) "mn  モンゴル語"
LangString lng_ne $(LANG_JAPANESE) "ne  ネパール語"
LangString lng_nb $(LANG_JAPANESE) "nb  ノルウェー語ブークモール"
LangString lng_nl $(LANG_JAPANESE) "nl  オランダ語"
LangString lng_nn $(LANG_JAPANESE) "nn  ノルウェー語ニーノシュク"
LangString lng_pa $(LANG_JAPANESE) "pa  パンジャブ語"
LangString lng_pl $(LANG_JAPANESE) "po  ポーランド語"
LangString lng_pt $(LANG_JAPANESE) "pt  ポルトガル語"
LangString lng_pt_BR $(LANG_JAPANESE) "pt_BR ポルトガル語 (ブラジル)"
LangString lng_ro $(LANG_JAPANESE) "ro  ルーマニア語"
LangString lng_ru $(LANG_JAPANESE) "ru  ロシア語"
LangString lng_rw $(LANG_JAPANESE) "rw  キニヤルワンダ語"
LangString lng_sk $(LANG_JAPANESE) "sk  スロバキア語"
LangString lng_sl $(LANG_JAPANESE) "sl  スロベニア語"
LangString lng_sq $(LANG_JAPANESE) "sq  アルバニア語"
LangString lng_sr $(LANG_JAPANESE) "sr  セルビア語"
LangString lng_sr@latin $(LANG_JAPANESE) "sr@latin  セルビア語ラテン文字"
LangString lng_sv $(LANG_JAPANESE) "sv  スウェーデン語"
LangString lng_th $(LANG_JAPANESE) "th  タイ語"
LangString lng_tr $(LANG_JAPANESE) "tr  トルコ語"
LangString lng_uk $(LANG_JAPANESE) "uk  ウクライナ語"
LangString lng_vi $(LANG_JAPANESE) "vi  ベトナム語"
LangString lng_zh_CN $(LANG_JAPANESE) "zh_CH  中国語 (簡体字)"
LangString lng_zh_TW $(LANG_JAPANESE) "zh_TW  中国語 (繁体字)"




; uninstallation options
LangString lng_UInstOpt   ${LANG_JAPANESE} "アンインストールオプション"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_JAPANESE} "必要であれば以下のオプションを選択してください"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_JAPANESE} "個人設定を残す"

LangString lng_RETRY_CANCEL_DESC ${LANG_JAPANESE} "$\n$\nRETRY を押すと続行、CANCEL を押すと中止します。"

LangString lng_ClearDirectoryBefore ${LANG_JAPANESE} "${PRODUCT_NAME} は空のディレクトリにインストールする必要がありますが、$INSTDIR は空ではありません。まずこのディレクトリをきれいにしてください!$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_JAPANESE} "$INSTDIR\uninstall.log が見つかりません!$\r$\nディレクトリ $INSTDIR を手動で削除してアンインストールしてください!"

LangString lng_FileChanged ${LANG_JAPANESE} "ファイル $filename はインストール後に変更されています。$\r$\nこのファイルを削除しますか?"

LangString lng_Yes ${LANG_JAPANESE} "はい"

LangString lng_AlwaysYes ${LANG_JAPANESE} "全てはい"

LangString lng_No ${LANG_JAPANESE} "いいえ"

LangString lng_AlwaysNo ${LANG_JAPANESE} "全ていいえ"

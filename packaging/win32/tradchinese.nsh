
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
LangString lng_Caption   ${LANG_TRADCHINESE} "${PRODUCT_NAME} -- ?}?????l?X?V?qø?ϳn??"

; Button text "Next >" for the license page
LangString lng_LICENSE_BUTTON   ${LANG_TRADCHINESE} "?U?@?B >"

; Bottom text for the license page
LangString lng_LICENSE_BOTTOM_TEXT   ${LANG_TRADCHINESE} "$(^Name) ?O?H GNU ?q?Τ??@?\?i?? (GPL) ?o???C ?o?̴??Ѫ??\?i?ҶȬ??ѦҰT???C $_CLICK"

;has been installed by different user
LangString lng_DIFFERENT_USER ${LANG_TRADCHINESE} "?ϥΪ? $0 ?w?g?w?? Inkscape?C$\r$\n?p?G?~???A?i???L?k???\?????I$\r$\n?ХH $0 ?????n?J???A?դ@???C"

; want to uninstall before install
LangString lng_WANT_UNINSTALL_BEFORE ${LANG_TRADCHINESE} "$R1 ?w?g?w?ˡC $\n?A?n?b?w?? $(^Name) ???e?????W?@?Ӫ????H"

; press OK to continue press Cancel to abort
LangString lng_OK_CANCEL_DESC ${LANG_TRADCHINESE} "$\n$\n???u?T?w?v?~???Ϊ̫??u?????v?????C"

;you have no admin rigths
LangString lng_NO_ADMIN ${LANG_TRADCHINESE} "?A?S???t?κ޲z???v???C$\r$\n?w?? Inkscape ???Ҧ??ϥΪ̥i???L?k???\?????C$\r$\n?????Ŀ??u?w?˵??Ҧ??ϥΪ̡v?ﶵ."

;win9x is not supported
LangString lng_NOT_SUPPORTED ${LANG_TRADCHINESE} "Inkscape ?L?k?? Windows 95/98/ME ?U?B??!$\r$\n?Ьd?ݩx???����??ԲӫH???C"

; Full install type
LangString lng_Full $(LANG_TRADCHINESE) "????"

; Optimal install type
LangString lng_Optimal $(LANG_TRADCHINESE) "?z?Q"

; Minimal install type
LangString lng_Minimal $(LANG_TRADCHINESE) "?̤p"

; Core install section
LangString lng_Core $(LANG_TRADCHINESE) "${PRODUCT_NAME} SVG ?s???{??(????)"

; Core install section description
LangString lng_CoreDesc $(LANG_TRADCHINESE) "${PRODUCT_NAME} ?֤??ɮשM DLL ??"

; GTK+ install section
LangString lng_GTKFiles $(LANG_TRADCHINESE) "GTK+ ????????(????)"

; GTK+ install section description
LangString lng_GTKFilesDesc $(LANG_TRADCHINESE) "${PRODUCT_NAME} ?ϥΪ??󥭥x GUI ?u????"

; shortcuts install section
LangString lng_Shortcuts $(LANG_TRADCHINESE) "???|"

; shortcuts install section description
LangString lng_ShortcutsDesc $(LANG_TRADCHINESE) "?Ұ? ${PRODUCT_NAME} ?????|"

; All user install section
LangString lng_Alluser $(LANG_TRADCHINESE) "?w?˵??Ҧ??ϥΪ?"

; All user install section description
LangString lng_AlluserDesc $(LANG_TRADCHINESE) "?w?˳o??��?ε{?????????ϥγo?x?q?????H(?Ҧ??ϥΪ?)"

; Desktop section
LangString lng_Desktop $(LANG_TRADCHINESE) "?ୱ"

; Desktop section description
LangString lng_DesktopDesc $(LANG_TRADCHINESE) "???ୱ?W?إ? ${PRODUCT_NAME} ???|"

; Start Menu  section
LangString lng_Startmenu $(LANG_TRADCHINESE) "?}?l?\????"

; Start Menu section description
LangString lng_StartmenuDesc $(LANG_TRADCHINESE) "???}?l?\?????إ? ${PRODUCT_NAME} ????"

; Quick launch section
LangString lng_Quicklaunch $(LANG_TRADCHINESE) "?ֳt?Ұ?"

; Quick launch section description
LangString lng_QuicklaunchDesc $(LANG_TRADCHINESE) "???ֳt?ҰʦC?إ? ${PRODUCT_NAME} ???|"

; File type association for editing
LangString lng_SVGWriter ${LANG_TRADCHINESE} "?? ${PRODUCT_NAME} ?}?? SVG ??"

; File type association for editing description
LangString lng_SVGWriterDesc ${LANG_TRADCHINESE} "???? ${PRODUCT_NAME} ?@?? SVG ?ɪ??w?]?s???{??"

; Context Menu
LangString lng_ContextMenu ${LANG_TRADCHINESE} "?k???\????"

; Context Menu description
LangString lng_ContextMenuDesc ${LANG_TRADCHINESE} "?s?W ${PRODUCT_NAME} ?? SVG ?ɪ??k???\????"

; remove personal preferences
LangString lng_DeletePrefs ${LANG_TRADCHINESE} "?R???ӤH???n?]?w"

; remove personal preferences description
LangString lng_DeletePrefsDesc ${LANG_TRADCHINESE} "?R???W?@???w?˿??d???ӤH???n?]?w"


; Additional files section
LangString lng_Addfiles $(LANG_TRADCHINESE) "???L?ɮ?"

; Additional files section description
LangString lng_AddfilesDesc $(LANG_TRADCHINESE) "???L?ɮ?"

; Examples section
LangString lng_Examples $(LANG_TRADCHINESE) "?d??"

; Examples section description
LangString lng_ExamplesDesc $(LANG_TRADCHINESE) "${PRODUCT_NAME} ?ϥνd??"

; Tutorials section
LangString lng_Tutorials $(LANG_TRADCHINESE) "???ɤ??U"

; Tutorials section description
LangString lng_TutorialsDesc $(LANG_TRADCHINESE) "${PRODUCT_NAME} ?ϥαо?"


; Languages section
LangString lng_Languages $(LANG_TRADCHINESE) "?y??"

; Languages section dscription
LangString lng_LanguagesDesc $(LANG_TRADCHINESE) "?w?? ${PRODUCT_NAME} ?U?ػy??½Ķ"

LangString lng_am $(LANG_TRADCHINESE) "am  ?????襧?Ȼy"
LangString lng_ar $(LANG_TRADCHINESE) "ar  ???ԧB?y"
LangString lng_az $(LANG_TRADCHINESE) "az  ?ȶ????M?y"
LangString lng_be $(LANG_TRADCHINESE) "be  ?իXù???y"
LangString lng_bg $(LANG_TRADCHINESE) "bg  ?O?[?Q?Ȼy"
LangString lng_bn $(LANG_TRADCHINESE) "bn  ?s?[?Իy"
LangString lng_br $(LANG_TRADCHINESE) "br  ???C?𥧻y"
LangString lng_ca $(LANG_TRADCHINESE) "ca  ?[?����y"
LangString lng_ca@valencia $(LANG_TRADCHINESE) "ca@valencia ?˭ۦ??Ȼy ?[??ù???Ȼy"
LangString lng_cs $(LANG_TRADCHINESE) "cs  ???J?y"
LangString lng_da $(LANG_TRADCHINESE) "da  ???��y"
LangString lng_de $(LANG_TRADCHINESE) "de  ?w?y"
LangString lng_dz $(LANG_TRADCHINESE) "dz  ?v?d?y"
LangString lng_el $(LANG_TRADCHINESE) "el  ??þ?y"
LangString lng_en $(LANG_TRADCHINESE) "en  ?^?y"
LangString lng_en_AU $(LANG_TRADCHINESE) "en_AU ?^?y(?D?j?Q??)"
LangString lng_en_CA $(LANG_TRADCHINESE) "en_CA ?^?y(?[???j)"
LangString lng_en_GB $(LANG_TRADCHINESE) "en_GB ?^?y(???C?A)"
LangString lng_en_US@piglatin $(LANG_TRADCHINESE) "en_US@piglatin ?ީԤB?y"
LangString lng_eo $(LANG_TRADCHINESE) "eo  ?@?ɻy"
LangString lng_es $(LANG_TRADCHINESE) "es  ???Z????"
LangString lng_es_MX $(LANG_TRADCHINESE) "es_MX ???Z???y(??????)"
LangString lng_et $(LANG_TRADCHINESE) "et  ?R?F???Ȼy"
LangString lng_eu $(LANG_TRADCHINESE) "eu  ?ڴ??J?y"
LangString lng_fi $(LANG_TRADCHINESE) "fi  ?????y"
LangString lng_fr $(LANG_TRADCHINESE) "fr  ?k??"
LangString lng_ga $(LANG_TRADCHINESE) "ga  ?R?????y"
LangString lng_gl $(LANG_TRADCHINESE) "gl  ?[?????Ȼy"
LangString lng_he $(LANG_TRADCHINESE) "he  ?ƧB?ӻy"
LangString lng_hr $(LANG_TRADCHINESE) "hr  ?Jù?J???Ȼy"
LangString lng_hu $(LANG_TRADCHINESE) "hu  ?I???Q?y"
LangString lng_id $(LANG_TRADCHINESE) "id  ?L???y"
LangString lng_it $(LANG_TRADCHINESE) "it  ?q?j?Q??"
LangString lng_ja $(LANG_TRADCHINESE) "ja  ????"
LangString lng_km $(LANG_TRADCHINESE) "km  ???ֻy"
LangString lng_ko $(LANG_TRADCHINESE) "ko  ????"
LangString lng_lt $(LANG_TRADCHINESE) "lt  ?߳??{?y"
LangString lng_mk $(LANG_TRADCHINESE) "mk  ?????y?y"
LangString lng_mn $(LANG_TRADCHINESE) "mn  ?X?j?y"
LangString lng_ne $(LANG_TRADCHINESE) "ne  ???y???y"
LangString lng_nb $(LANG_TRADCHINESE) "nb  ???? Bokmal ?y"
LangString lng_nl $(LANG_TRADCHINESE) "nl  ?????y"
LangString lng_nn $(LANG_TRADCHINESE) "nn  ???? Nynorsk ?y"
LangString lng_pa $(LANG_TRADCHINESE) "pa  ?ǾB???y"
LangString lng_pl $(LANG_TRADCHINESE) "po  ?i???y"
LangString lng_pt $(LANG_TRADCHINESE) "pt  ????????"
LangString lng_pt_BR $(LANG_TRADCHINESE) "pt_BR ????????(?ڦ?)"
LangString lng_ro $(LANG_TRADCHINESE) "ro  ù?????Ȼy"
LangString lng_ru $(LANG_TRADCHINESE) "ru  ?X??"
LangString lng_rw $(LANG_TRADCHINESE) "rw  ???ȿc?w?F?y"
LangString lng_sk $(LANG_TRADCHINESE) "sk  ?????k?J?y"
LangString lng_sl $(LANG_TRADCHINESE) "sl  ?????Z???Ȼy"
LangString lng_sq $(LANG_TRADCHINESE) "sq  ?????ڥ??Ȼy"
LangString lng_sr $(LANG_TRADCHINESE) "sr  ?ɺ????Ȼy"
LangString lng_sr@latin $(LANG_TRADCHINESE) "sr@latin ?뺸???Ȥ?-?ԤB"
LangString lng_sv $(LANG_TRADCHINESE) "sv  ?????y"
LangString lng_th $(LANG_TRADCHINESE) "th  ???y"
LangString lng_tr $(LANG_TRADCHINESE) "tr  ?g?ը??y"
LangString lng_uk $(LANG_TRADCHINESE) "uk  ?Q?J???y"
LangString lng_vi $(LANG_TRADCHINESE) "vi  ?V?n??"
LangString lng_zh_CN $(LANG_TRADCHINESE) "zh_CH  ²?餤??"
LangString lng_zh_TW $(LANG_TRADCHINESE) "zh_TW  ?c?餤??"




; uninstallation options
LangString lng_UInstOpt   ${LANG_TRADCHINESE} "?Ϧw?˿ﶵ"

; uninstallation options subtitle
LangString lng_UInstOpt1  ${LANG_TRADCHINESE} "?п??ܨ??L?ﶵ"

; Ask to purge the personal preferences
LangString lng_PurgePrefs ${LANG_TRADCHINESE} "?O?d?ӤH???n?]?w"

LangString lng_RETRY_CANCEL_DESC ${LANG_TRADCHINESE} "$\n$\n???u???աv?~???Ϊ̫??u?????v?????C"

LangString lng_ClearDirectoryBefore ${LANG_TRADCHINESE} "${PRODUCT_NAME} ???w?˩??@?ӪŪ????Ƨ??C $INSTDIR ???O?Ū??C ?Х??M?ųo?Ӹ??Ƨ??I$(lng_RETRY_CANCEL_DESC)"

LangString lng_UninstallLogNotFound ${LANG_TRADCHINESE} "?S?????? $INSTDIR\uninstall.log ?I$\r$\n?Цۦ??M?? $INSTDIR ???Ƨ??ӤϦw?ˡI"

LangString lng_FileChanged ${LANG_TRADCHINESE} "?w?˫? $filename ?ɮפw?ܧ??C$\r$\n?A?O?_???M?n?R???????ɮסH"

LangString lng_Yes ${LANG_TRADCHINESE} "?O"

LangString lng_AlwaysYes ${LANG_TRADCHINESE} "?????ҬO"

LangString lng_No ${LANG_TRADCHINESE} "?_"

LangString lng_AlwaysNo ${LANG_TRADCHINESE} "?????ҧ_"

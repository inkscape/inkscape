!macro CustomCodePostInstall
	${If} ${FileExists} "$INSTDIR\Data\settings\AppData" ; 0.46 Development Tests
		Rename "$INSTDIR\Data\settings\AppData" "$INSTDIR\Data\_settings"
	${ElseIf} ${FileExists} "$INSTDIR\Data\settings\InkscapePortableAppData" ; 0.46 Pre-Release 1
		Rename "$INSTDIR\Data\settings\InkscapePortableAppData" "$INSTDIR\Data\_settings"
	${EndIf}
	${If} ${FileExists} "$INSTDIR\Data\_settings"
		Rename "$INSTDIR\Data\settings\InkscapePortableSettings.ini" "$INSTDIR\Data\_settings\InkscapePortableSettings.ini"
		RMDir /r "$INSTDIR\Data\settings"
		Rename "$INSTDIR\Data\settings" "$INSTDIR\Data\_settings"
	${EndIf}
!macroend

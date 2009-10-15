;Copyright (C) 2004-2008 John T. Haller of PortableApps.com
;Copyright (C) 2008-2009 Chris Morgan of PortableApps.com and Inkscape.org

;Website: http://PortableApps.com/InkscapePortable

;This software is OSI Certified Open Source Software.
;OSI Certified is a certification mark of the Open Source Initiative.

;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;as published by the Free Software Foundation; either version 2
;of the License, or (at your option) any later version.

;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.

;You should have received a copy of the GNU General Public License
;along with this program; if not, write to the Free Software
;Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

!define PORTABLEAPPNAME "Inkscape Portable"
!define NAME InkscapePortable
!define VER "1.6.6.0"
!define LAUNCHERLANGUAGE English

;=== Program Details
Name "${PORTABLEAPPNAME}"
OutFile "..\..\${NAME}.exe"
VIProductVersion "${VER}"
VIAddVersionKey ProductName "${PORTABLEAPPNAME}"
VIAddVersionKey Comments "Allows Inkscape to be run from a removable drive.  For additional details, visit Inkscape.org"
VIAddVersionKey CompanyName "PortableApps.com"
VIAddVersionKey LegalCopyright "PortableApps.com & Inkscape.org"
VIAddVersionKey FileDescription "${PORTABLEAPPNAME}"
VIAddVersionKey FileVersion "${VER}"
VIAddVersionKey ProductVersion "${VER}"
VIAddVersionKey InternalName "${PORTABLEAPPNAME}"
VIAddVersionKey LegalTrademarks "PortableApps.com is a Trademark of Rare Ideas, LLC."
VIAddVersionKey OriginalFilename "${NAME}.exe"
;VIAddVersionKey PrivateBuild ""
;VIAddVersionKey SpecialBuild ""

;=== Runtime Switches
CRCCheck On
WindowIcon Off
SilentInstall Silent
AutoCloseWindow True
RequestExecutionLevel user

; Best Compression
SetCompress Auto
SetCompressor /SOLID lzma
SetCompressorDictSize 32
SetDatablockOptimize On

;=== Include
;(Standard NSIS)
!include FileFunc.nsh
!insertmacro GetParameters
!insertmacro GetRoot

;(NSIS Plugins)
!include TextReplace.nsh

;(Custom)
!include ReplaceInFileWithTextReplace.nsh

;=== Program Icon
Icon "..\..\App\AppInfo\appicon.ico"

;=== Icon & Stye ===
;!define MUI_ICON "..\..\App\AppInfo\appicon.ico"

;=== Languages
LoadLanguageFile "${NSISDIR}\Contrib\Language files\${LAUNCHERLANGUAGE}.nlf"
!include PortableApps.comLauncherLANG_${LAUNCHERLANGUAGE}.nsh

Var MISSINGFILEORPATH

Section Main
	IfFileExists "$EXEDIR\App\Inkscape\inkscape.exe" DisplaySplash
		StrCpy $MISSINGFILEORPATH "inkscape.exe"
		MessageBox MB_OK|MB_ICONEXCLAMATION `$(LauncherFileNotFound)`
		Abort

	DisplaySplash:
		ReadINIStr $0 "$EXEDIR\${NAME}.ini" "${NAME}" DisableSplashScreen
		StrCmp $0 "true" SettingsDirectory
			;=== Show the splash screen while processing registry entries
			InitPluginsDir
			File /oname=$PLUGINSDIR\splash.jpg "${NAME}.jpg"
			newadvsplash::show /NOUNLOAD 1200 0 0 -1 /L $PLUGINSDIR\splash.jpg

	SettingsDirectory:
		CreateDirectory $EXEDIR\Data\settings
		System::Call 'Kernel32::SetEnvironmentVariableA(t, t) i("INKSCAPE_PORTABLE_PROFILE_DIR", "$EXEDIR\Data\settings").n'
		System::Call 'Kernel32::SetEnvironmentVariableA(t, t) i("HOME", "$EXEDIR\Data\settings").n' ; possibly some GTK stuff
		System::Call 'Kernel32::SetEnvironmentVariableA(t, t) i("APPDATA", "$EXEDIR\Data\settings").n' ; Just to be on the safe side

	;InkscapeLanguage:
		ReadEnvStr $0 "PortableApps.comLocaleglibc"
		StrCmp $0 "" InkscapeLanguageSettingsFile
		StrCmp $0 "en_US" SetInkscapeLanguageVariable
		IfFileExists "$EXEDIR\App\Inkscape\locale\$0\*.*" SetInkscapeLanguageVariable

	InkscapeLanguageSettingsFile:
		ReadINIStr $0 "$EXEDIR\Data\settings\${NAME}Settings.ini" Language LANG
		StrCmp $0 "" AdjustPaths
		StrCmp $0 "en_US" SetInkscapeLanguageVariable
		IfFileExists "$EXEDIR\App\Inkscape\locale\$0\*.*" SetInkscapeLanguageVariable AdjustPaths

	SetInkscapeLanguageVariable:
		System::Call 'Kernel32::SetEnvironmentVariableA(t, t) i("LANG", r0).n'

	AdjustPaths:
		ReadINIStr $0 "$EXEDIR\Data\settings\${NAME}Settings.ini" "${NAME}Settings" LastDrive
		${GetRoot} $EXEDIR $1
		StrCmp $0 $1 Launch
		WriteINIStr "$EXEDIR\Data\settings\${NAME}Settings.ini" "${NAME}Settings" LastDrive $1
		${ReplaceInFile} "$EXEDIR\Data\settings\preferences.xml" "$0\" "$1\"
		${ReplaceInFile} "$EXEDIR\Data\settings\.recently-used.xbel" "file:///$0/" "file:///$1/"

	Launch:
		${GetParameters} $0
		ReadINIStr $1 "$EXEDIR\${NAME}.ini" "${NAME}" AdditionalParameters
		Exec `"$EXEDIR\App\Inkscape\inkscape.exe" $0 $1`
		newadvsplash::stop /WAIT
SectionEnd

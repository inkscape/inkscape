; See http://nsis.sourceforge.net/Check_if_a_file_exists_at_compile_time for documentation
!macro !ifexist _FILE_NAME
	!tempfile _TEMPFILE
	!system `if exist "${_FILE_NAME}" echo !define _FILE_EXISTS > "${_TEMPFILE}"`
	!include `${_TEMPFILE}`
	!delfile `${_TEMPFILE}`
	!undef _TEMPFILE
	!ifdef _FILE_EXISTS
		!undef _FILE_EXISTS
!macroend
!define !ifexist "!insertmacro !ifexist"
!macro !ifnexist _FILE_NAME
	!tempfile _TEMPFILE
	!system `if not exist "${_FILE_NAME}" echo !define _FILE_EXISTS > "${_TEMPFILE}"`
	!include `${_TEMPFILE}`
	!delfile `${_TEMPFILE}`
	!undef _TEMPFILE
	!ifdef _FILE_EXISTS
		!undef _FILE_EXISTS
!macroend
!define !ifnexist "!insertmacro !ifnexist"

!macro !defineifexist _FILE_NAME _VAR_NAME _VAR_VALUE
	!tempfile _TEMPFILE
	!ifdef NSIS_WIN32_MAKENSIS
		; Windows - cmd.exe
		!system 'if exist "${_FILE_NAME}" echo !define ${_VAR_NAME} ${_VAR_VALUE} > "${_TEMPFILE}"'
	!else
		; Posix - sh
		!system 'if [ -e "${_FILE_NAME}" ]; then echo "!define ${_VAR_NAME} ${_VAR_VALUE}" > "${_TEMPFILE}"; fi'
	!endif
	!include '${_TEMPFILE}'
	!delfile '${_TEMPFILE}'
	!undef _TEMPFILE
!macroend
!define !defineifexist "!insertmacro !defineifexist"

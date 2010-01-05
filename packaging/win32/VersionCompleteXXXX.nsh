; See http://nsis.sourceforge.net/VersionCompleteXXXX for documentation
!macro VersionCompleteXXXN _INPUT_VALUE _OUTPUT_SYMBOL _REVISION
	!searchparse /noerrors ${_INPUT_VALUE} "" _VERSION_1 "." _VERSION_2 "." _VERSION_3 "." _VERSION_4
	!if `${_VERSION_1}` == ``
		!undef _VERSION_1
		!define _VERSION_1 0
	!endif
	!if `${_VERSION_2}` == ``
		!undef _VERSION_2
		!define _VERSION_2 0
	!endif
	!if `${_VERSION_3}` == ``
		!undef _VERSION_3
		!define _VERSION_3 0
	!endif
	!define ${_OUTPUT_SYMBOL} ${_VERSION_1}.${_VERSION_2}.${_VERSION_3}.${_REVISION}
	!undef _VERSION_1
	!undef _VERSION_2
	!undef _VERSION_3
	!undef _VERSION_4
!macroend
!define VersionCompleteXXXN `!insertmacro VersionCompleteXXXN`
!macro VersionCompleteXXXX _INPUT_VALUE _OUTPUT_SYMBOL
	!searchparse /noerrors ${_INPUT_VALUE} "" _VERSION_1 "." _VERSION_2 "." _VERSION_3 "." _VERSION_4
	!if `${_VERSION_1}` == ``
		!undef _VERSION_1
		!define _VERSION_1 0
	!endif
	!if `${_VERSION_2}` == ``
		!undef _VERSION_2
		!define _VERSION_2 0
	!endif
	!if `${_VERSION_3}` == ``
		!undef _VERSION_3
		!define _VERSION_3 0
	!endif
	!if `${_VERSION_4}` == ``
		!undef _VERSION_4
		!define _VERSION_4 0
	!endif
	!define ${_OUTPUT_SYMBOL} ${_VERSION_1}.${_VERSION_2}.${_VERSION_3}.${_VERSION_4}
	!undef _VERSION_1
	!undef _VERSION_2
	!undef _VERSION_3
	!undef _VERSION_4
!macroend
!define VersionCompleteXXXX `!insertmacro VersionCompleteXXXX`

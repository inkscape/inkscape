!if ${NSIS_VERSION} = v2.45
	!error "There is a bug in !searchparse which makes this script not compile in NSIS 2.45. Please upgrade to NSIS 2.46 or later and try again."
!else
	!echo "(If you get a compile error with !searchparse, please upgrade to NSIS 2.46 or later and try again.)"
!endif
!searchparse ${NSIS_VERSION} "v" V
!if ${V} < 2.46
	!error "You only have NSIS ${V}, but NSIS 2.46 or later is required for proper Windows 7 support. Please upgrade to NSIS 2.46 or later and try again."
!endif

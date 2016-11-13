!searchparse ${NSIS_VERSION} "v" V
!if ${V} < 3.0
	!error "You only have NSIS ${V}, but NSIS 3.0 or later is required to support Unicode and Windows 10. Please upgrade to NSIS 3.0 or later and try again."
!endif

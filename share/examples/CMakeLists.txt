file(GLOB _FILES "README" "*.svg" "*.svgz" "*.pov")
install(FILES ${_FILES} DESTINATION ${INKSCAPE_SHARE_INSTALL}/examples)

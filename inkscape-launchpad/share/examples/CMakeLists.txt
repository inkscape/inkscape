file(GLOB _FILES "README" "*.svg" "*.svgz" "*.pov")
install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/examples)

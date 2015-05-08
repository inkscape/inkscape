file(GLOB _FILES "README" "*.svg" "*.jpg" "*.png")
install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/tutorials)

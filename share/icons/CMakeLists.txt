add_subdirectory(application)

file(GLOB _FILES "*.svg" "*.jpg" "*.png" "README")
install(FILES ${_FILES} DESTINATION ${INKSCAPE_SHARE_INSTALL}/icons)

install(FILES ../branding/inkscape.svg DESTINATION ${INKSCAPE_SHARE_INSTALL}/icons)

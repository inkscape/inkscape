add_subdirectory(application)

file(GLOB _FILES "*.svg" "*.jpg" "*.png" "README")
install(FILES ${_FILES} DESTINATION ${SHARE_INSTALL}/inkscape/icons)

install(FILES ../branding/inkscape.svg DESTINATION ${SHARE_INSTALL}/inkscape/icons)

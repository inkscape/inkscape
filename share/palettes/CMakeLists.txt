set(I18N_FILES "inkscape.gpl" "svg.gpl" "Tango-Palette.gpl")

add_custom_command(
  OUTPUT palettes.h
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/i18n.py ${I18N_FILES} > ${CMAKE_CURRENT_BINARY_DIR}/palettes.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/i18n.py ${I18N_FILES}
)
add_custom_target(palettes_h ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/palettes.h)

file(GLOB _FILES "*.gpl")

install(FILES ${_FILES} "README" ${CMAKE_CURRENT_BINARY_DIR}/palettes.h DESTINATION ${SHARE_INSTALL}/inkscape/palettes)

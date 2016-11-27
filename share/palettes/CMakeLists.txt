set(I18N_FILES "inkscape.gpl" "svg.gpl" "Tango-Palette.gpl")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/palettes.h
  COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/i18n.py ${I18N_FILES} > ${CMAKE_CURRENT_SOURCE_DIR}/palettes.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/i18n.py ${I18N_FILES}
)  
set_source_files_properties(${CMAKE_SOURCE_DIR}/palettes.h PROPERTIES GENERATED TRUE)

add_custom_target(palettes_h ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/palettes.h)

file(GLOB _FILES "*.gpl")

install(FILES ${_FILES} "README" ${CMAKE_CURRENT_SOURCE_DIR}/palettes.h DESTINATION ${INKSCAPE_SHARE_INSTALL}/palettes)

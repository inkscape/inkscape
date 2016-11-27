add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/patterns.svg.h
  COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/i18n.py ${CMAKE_CURRENT_SOURCE_DIR}/patterns.svg > ${CMAKE_CURRENT_SOURCE_DIR}/patterns.svg.h
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/patterns.svg
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/i18n.py
)
set_source_files_properties(${CMAKE_SOURCE_DIR}/patterns.svg.h PROPERTIES GENERATED TRUE)
add_custom_target(patterns_svg_h ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/patterns.svg.h)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/patterns.svg.h DESTINATION ${INKSCAPE_SHARE_INSTALL}/patterns)
install(FILES "patterns.svg" "README" DESTINATION ${INKSCAPE_SHARE_INSTALL}/patterns)

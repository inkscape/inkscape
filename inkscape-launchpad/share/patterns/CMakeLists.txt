add_custom_command(
  OUTPUT patterns.svg.h
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/i18n.py ${CMAKE_CURRENT_SOURCE_DIR}/patterns.svg > ${CMAKE_CURRENT_BINARY_DIR}/patterns.svg.h
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/patterns.svg
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/i18n.py
)
add_custom_target(patterns_svg_h ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/patterns.svg.h)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/patterns.svg.h DESTINATION ${SHARE_INSTALL}/inkscape/patterns)
install(FILES "patterns.svg" "README" DESTINATION ${SHARE_INSTALL}/inkscape/patterns)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/filters.svg.h
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/i18n.py ${CMAKE_CURRENT_SOURCE_DIR}/filters.svg > ${CMAKE_CURRENT_BINARY_DIR}/filters.svg.h
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/filters.svg
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/i18n.py
)
add_custom_target(filters_svg_h ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/filters.svg.h)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/filters.svg.h DESTINATION ${SHARE_INSTALL}/inkscape/filters)
install(FILES "filters.svg" "README" DESTINATION ${SHARE_INSTALL}/inkscape/filters)

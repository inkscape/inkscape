file(GLOB LANGUAGES *.po)
foreach(language ${LANGUAGES})
  string(REGEX REPLACE "(.+(\\\\|/))+" "" language ${language})
  string(REGEX REPLACE "\\.po$" "" language ${language})
  set(pofile ${CMAKE_CURRENT_SOURCE_DIR}/${language}.po)
  GETTEXT_PROCESS_PO_FILES(${language} ALL INSTALL_DESTINATION "share/locale/" PO_FILES ${pofile})
endforeach(language)

if(NOT WIN32)

#translates inkscape.desktop
add_custom_target(inkscape_desktop DEPENDS ${CMAKE_BINARY_DIR}/inkscape.desktop)

if(${GETTEXT_VERSION_STRING} VERSION_GREATER "0.19")
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/inkscape.desktop 
        DEPENDS ${LANGUAGES}
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --desktop --template ${CMAKE_SOURCE_DIR}/inkscape.desktop.template -d ${CMAKE_CURRENT_SOURCE_DIR} -o ${CMAKE_BINARY_DIR}/inkscape.desktop.template.in --keyword=Name --keyword=GenericName --keyword=X-GNOME-FullName --keyword=Comment --keyword=Keywords
        COMMAND ${CMAKE_COMMAND} -DINKSCAPE_SOURCE_DIR=${CMAKE_SOURCE_DIR} -DINKSCAPE_BINARY_DIR=${CMAKE_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/CMakeScripts/inkscape-desktop.cmake
    )
else()
    message("Old gettext version, not translating inkscape.desktop")
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/inkscape.desktop
        COMMAND cp ${CMAKE_SOURCE_DIR}/inkscape.desktop.template ${CMAKE_BINARY_DIR}/inkscape.desktop.template.in
        COMMAND ${CMAKE_COMMAND} -DINKSCAPE_SOURCE_DIR=${CMAKE_SOURCE_DIR} -DINKSCAPE_BINARY_DIR=${CMAKE_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/CMakeScripts/inkscape-desktop.cmake
    )
endif()

add_dependencies(inkscape inkscape_desktop)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/inkscape.pot 
    COMMAND intltool-update --pot && mv untitled.pot ${CMAKE_CURRENT_SOURCE_DIR}/inkscape.pot
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/inkscape.pot PROPERTIES GENERATED TRUE)
add_custom_target(inkscape_pot ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/inkscape.pot)
add_dependencies(inkscape_pot filters_svg_h)
add_dependencies(inkscape_pot palettes_h)
add_dependencies(inkscape_pot patterns_svg_h)
add_dependencies(inkscape_pot symbols_h)
add_dependencies(inkscape_pot templates_h)




endif()

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

endif()

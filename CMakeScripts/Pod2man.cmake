# Use pod2man to generate manual pages from .pod files

# Usage:  pod2man(<podfile> <release-string> <man-section> <heading-center-text>)
#
# E.g.: pod2man("/path/to/file/mypod.pod" "1.2.3" 1 "My Manual Pages")

find_program(POD2MAN pod2man)
if(NOT POD2MAN)
    message(STATUS "Could not find pod2man - man pages disabled")
endif()

macro(pod2man PODFILE_FULL RELEASE SECTION CENTER)
    get_filename_component(PODFILE ${PODFILE_FULL} NAME)
    string(REPLACE "." ";" PODFILE_LIST ${PODFILE})
    list(GET PODFILE_LIST 0 NAME)
    list(GET PODFILE_LIST 1 LANG)
    string(TOUPPER ${NAME} NAME_UPCASE)
    if(${LANG} STREQUAL "pod")
        set(LANG "")
    endif()

    if(NOT EXISTS ${PODFILE_FULL})
        message(FATAL ERROR "Could not find pod file ${PODFILE_FULL} to generate man page")
    endif(NOT EXISTS ${PODFILE_FULL})

    if(POD2MAN)
        if(LANG)
            set(MANPAGE_TARGET "man-${NAME}-${LANG}")
            set(MANFILE_TEMP "${CMAKE_CURRENT_BINARY_DIR}/man/${NAME}.${LANG}.tmp")
            set(MANFILE_FULL "${CMAKE_CURRENT_BINARY_DIR}/man/${NAME}.${LANG}.${SECTION}")
        else()
            set(MANPAGE_TARGET "man-${NAME}")
            set(MANFILE_TEMP "${CMAKE_CURRENT_BINARY_DIR}/man/${NAME}.tmp")
            set(MANFILE_FULL "${CMAKE_CURRENT_BINARY_DIR}/man/${NAME}.${SECTION}")
        endif()
        add_custom_command(
            OUTPUT ${MANFILE_TEMP}
            COMMAND ${POD2MAN} --utf8 --section="${SECTION}" --center="${CENTER}"
                --release="${RELEASE}" --name="${NAME_UPCASE}" "${PODFILE_FULL}" "${MANFILE_TEMP}"
        )
        add_custom_command(
            OUTPUT ${MANFILE_FULL}
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/fix-roff-punct "${MANFILE_TEMP}" > ${MANFILE_FULL}
            DEPENDS ${MANFILE_TEMP}
        )
        add_custom_target(${MANPAGE_TARGET} ALL
            DEPENDS ${MANFILE_FULL}
        )
        install(
            FILES ${MANFILE_FULL}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/${SHARE_INSTALL}/man/man${SECTION}
        )
    endif()
endmacro(pod2man PODFILE NAME SECTION CENTER)

message(STATUS "Creating build files in: ${CMAKE_CURRENT_BINARY_DIR}")

if(WIN32)
    set(PACKAGE_LOCALE_DIR "locale")
    set(SHARE_INSTALL "share" CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
else(WIN32)
    # TODO: check and change this to correct value:
    if(NOT PACKAGE_LOCALE_DIR)
	set(PACKAGE_LOCALE_DIR "${CMAKE_INSTALL_PREFIX}/share/locale") # packagers might overwrite this
    endif(NOT PACKAGE_LOCALE_DIR)

    if(NOT SHARE_INSTALL)
	set(SHARE_INSTALL "share" CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
    endif(NOT SHARE_INSTALL)
    mark_as_advanced(SHARE_INSTALL)
endif(WIN32)

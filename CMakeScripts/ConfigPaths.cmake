MESSAGE(STATUS "Creating build files in: ${CMAKE_CURRENT_BINARY_DIR}")

IF(WIN32)
    SET(PACKAGE_LOCALE_DIR "locale")
    set(SHARE_INSTALL "share" CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
ELSE(WIN32)
    # TODO: check and change this to correct value:
    SET(PACKAGE_LOCALE_DIR "locale")

  if(NOT SHARE_INSTALL)
    set(SHARE_INSTALL "share" CACHE STRING "Data file install path. Must be a relative path (from CMAKE_INSTALL_PREFIX), with no trailing slash.")
  endif(NOT SHARE_INSTALL)
  mark_as_advanced(SHARE_INSTALL)
ENDIF(WIN32)

#SET(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
#SET(CMAKE_SKIP_RPATH:BOOL OFF)

# Include base dir, so other files can refer to the generated files.
# CMAKE_INCLUDE_CURRENT_DIR is not enough as it only includes the current dir and not the basedir with config.h in it
#INCLUDE_DIRECTORIES ("${CMAKE_BINARY_DIR}" "${PROJECT_SOURCE_DIR}" src/)  
#LINK_DIRECTORIES ("${LINK_DIRECTORIES}" "${CMAKE_BINARY_DIR}" "${PROJECT_SOURCE_DIR}" src/)

#INSTALL(TARGETS INKSCAPE
#  RUNTIME DESTINATION bin
#  LIBRARY DESTINATION lib
#  ARCHIVE DESTINATION lib
#)

#FILE(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
#INSTALL(FILES ${files} DESTINATION include/INKSCAPE/INKSCAPE)

#CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/INKSCAPE.pc.in
#                ${CMAKE_BINARY_DIR}/INKSCAPE.pc @ONLY IMMEDIATE )
#INSTALL(FILES "${CMAKE_BINARY_DIR}/INKSCAPE.pc" DESTINATION lib/pkgconfig)

#SET(EXECUTABLE_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL  "Where to put the executables")set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL  "Where to put the libraries")

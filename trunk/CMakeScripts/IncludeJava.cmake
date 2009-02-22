# - include the src/javainc dir in the include path and the correct config path too


INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src/bind/javainc )

IF (WIN32)
    INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src/bind/javainc/win32 )
ENDIF (WIN32)
IF (UNIX)
    INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src/bind/javainc/linux )
ENDIF (UNIX)
IF (SOLARIS)
    INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src/bind/javainc/solaris )
ENDIF (SOLARIS)

# - include the src/javainc dir in the include path and the correct config path too

include_directories(${CMAKE_SOURCE_DIR}/src/bind/javainc)

if(WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/src/bind/javainc/win32)
endif(WIN32)
if(UNIX)
    include_directories(${CMAKE_SOURCE_DIR}/src/bind/javainc/linux)
endif(UNIX)
if(SOLARIS)
    include_directories(${CMAKE_SOURCE_DIR}/src/bind/javainc/solaris)
endif(SOLARIS)

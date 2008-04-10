SET(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")
ADD_DEFINITIONS(-Wall -Wformat-security -W -Wpointer-arith -Wcast-align -Wsign-compare -Woverloaded-virtual -Wswitch)
ADD_DEFINITIONS(-O2)

#define the flags for profiling if desired:
IF (ENABLE_PROFILING)
    SET(COMPILE_PROFILING_FLAGS "-pg")
    SET(LINK_PROFILING_FLAGS "-pg")
ENDIF (ENABLE_PROFILING)


ADD_DEFINITIONS(-DVERSION=\\\"${INKSCAPE_VERSION}\\\")
ADD_DEFINITIONS(${DEFINE_FLAGS} -DHAVE_CONFIG_H -D_INTL_REDIRECT_INLINE)

IF (WIN32)
     ADD_DEFINITIONS(-DXP_WIN)
ENDIF (WIN32)

# for Inkboard:
ADD_DEFINITIONS(-DHAVE_SSL "-DRELAYTOOL_SSL=\"static const int libssl_is_present=1; static int __attribute__((unused)) libssl_symbol_is_present(char *s){ return 1; }\"")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILE_PROFILING_FLAGS} ")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILE_PROFILING_FLAGS} ")

SET(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} ")


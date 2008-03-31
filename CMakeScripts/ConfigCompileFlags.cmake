SET(COMPILE_WARNING_FLAGS "-Wall -Wformat-security -W -Wpointer-arith -Wcast-align -Wsign-compare -Woverloaded-virtual -Wswitch")
SET(COMPILE_OPTIMIZATION_FLAGS "-O2")

SET(DEFINE_FLAGS "-DHAVE_CONFIG_H -D_INTL_REDIRECT_INLINE ")

IF (WIN32)
     SET(DEFINE_FLAGS "${DEFINE_FLAGS} -DXP_WIN")
ELSE(WIN32)
#     SET(DEFINE_FLAGS "${DEFINE_FLAGS} ")
ENDIF (WIN32)

# for Inkboard:
SET(DEFINE_FLAGS "${DEFINE_FLAGS} -DHAVE_SSL -DRELAYTOOL_SSL=\"static const int libssl_is_present=1; static int __attribute__((unused)) libssl_symbol_is_present(char *s){ return 1; }\" ")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILE_WARNING_FLAGS} ${COMPILE_OPTIMIZATION_FLAGS} ${DEFINE_FLAGS} -DVERSION=\\\"${INKSCAPE_VERSION}\\\" ")

SET(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} -j2")


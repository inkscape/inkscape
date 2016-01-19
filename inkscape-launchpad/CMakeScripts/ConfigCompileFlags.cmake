set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS}")
add_definitions(-Wall -Wformat-security -W -Wpointer-arith -Wcast-align -Wsign-compare -Woverloaded-virtual -Wswitch)
add_definitions(-O2)

# Define the flags for profiling if desired:
if(WITH_PROFILING)
    set(COMPILE_PROFILING_FLAGS "-pg")
    set(LINK_PROFILING_FLAGS "-pg")
endif()

add_definitions(-DVERSION=\\\"${INKSCAPE_VERSION}\\\")
add_definitions(${DEFINE_FLAGS} -DHAVE_CONFIG_H -D_INTL_REDIRECT_INLINE)

if(WIN32)
    add_definitions(-DXP_WIN)
endif(WIN32)

# For Inkboard:
add_definitions(-DHAVE_SSL "-DRELAYTOOL_SSL=\"static const int libssl_is_present=1; static int __attribute__((unused)) libssl_symbol_is_present(char *s){ return 1; }\"")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILE_PROFILING_FLAGS} ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILE_PROFILING_FLAGS} ")

set(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} ")

# message(STATUS "${CMAKE_CXX_FLAGS}")

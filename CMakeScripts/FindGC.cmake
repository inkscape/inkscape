# - Find garbage collector library
# Go hunting for garbage collector compoments
# Defines:
#  LIBGC_INCLUDE_DIR


# To find gc on Windows, use DEVLIBS_PATH variable set by mingwenv.bat

FIND_PATH(LIBGC_INCLUDE_DIR gc.h 
                            /usr/include/gc 
                            /usr/local/include/gc
                            $ENV{DEVLIBS_PATH}//include//gc )


IF (LIBGC_INCLUDE_DIR)
  SET(LIBGC_FOUND TRUE)
ENDIF (LIBGC_INCLUDE_DIR)

IF (LIBGC_FOUND)
     MESSAGE(STATUS "gc: FOUND  ( ${LIBGC_INCLUDE_DIR} )")
ELSE(LIBGC_FOUND)
     MESSAGE(FATAL_ERROR "gc: NOT FOUND")
ENDIF (LIBGC_FOUND)

INCLUDE_DIRECTORIES( ${LIBGC_INCLUDE_DIR} )


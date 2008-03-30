# - Find Boost libraries
# Go hunting for boost compoments
# Defines:
#  BOOST_INCLUDE


# To find boost on Windows, use BOOST_PATH variable set by mingwenv.bat:
SET(env_boost_path "$ENV{BOOST_PATH}")

FIND_PATH(BOOST_INCLUDE_DIR boost/weak_ptr.hpp 
                            /usr/include 
                            /usr/local/include
                            env_boost_path )


IF(MINGW)
  SET (BOOST_ROOT env_boost_path)
  FIND_LIBRARY( BOOST_PYTHON_LIBRARY 
                libboost_python-mgw
                PATHS ${BOOST_ROOT}\\lib )
  FIND_LIBRARY( BOOST_PYTHON_LIBRARY_DEBUG 
                libboost_python-mgw-d
                PATHS ${BOOST_ROOT}\\lib )
ELSE(MINGW)
  FIND_LIBRARY( BOOST_PYTHON_LIBRARY NAMES boost_python
                PATHS /usr/lib /usr/local/lib ${BOOST_ROOT}\\lib )
  FIND_LIBRARY( BOOST_PYTHON_LIBRARY_DEBUG NAMES boost_python-d
                PATHS /usr/lib /usr/local/lib ${BOOST_ROOT}\\lib )
ENDIF(MINGW)

IF (BOOST_INCLUDE_DIR)
  SET(BOOST_FOUND TRUE)
ENDIF (BOOST_INCLUDE_DIR)

IF (BOOST_FOUND)
  IF (NOT Boost_FIND_QUIETLY)
     MESSAGE(STATUS "Found Boost: ${BOOST_INCLUDE_DIR}")
  ENDIF (NOT Boost_FIND_QUIETLY)
ELSE(BOOST_FOUND)
  IF (Boost_FIND_REQUIRED)
     MESSAGE(FATAL_ERROR "Could not find Boost")
  ENDIF (Boost_FIND_REQUIRED)
ENDIF (BOOST_FOUND)



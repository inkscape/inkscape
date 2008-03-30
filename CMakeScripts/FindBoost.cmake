# - Find Boost libraries
# Go hunting for boost compoments
# Defines:
#  BOOST_INCLUDE_DIR


# To find boost on Windows, use BOOST_PATH variable set by mingwenv.bat:
SET(env_boost_path "$ENV{BOOST_PATH}")

FIND_PATH(BOOST_INCLUDE_DIR boost/weak_ptr.hpp 
                            /usr/include 
                            /usr/local/include
                            env_boost_path )


IF (BOOST_INCLUDE_DIR)
  SET(BOOST_FOUND TRUE)
ENDIF (BOOST_INCLUDE_DIR)

IF (BOOST_FOUND)
     MESSAGE(STATUS "Found Boost: ${BOOST_INCLUDE_DIR}")
ELSE(BOOST_FOUND)
     MESSAGE(FATAL_ERROR "Could not find Boost")
ENDIF (BOOST_FOUND)



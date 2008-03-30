# - Find Boost libraries
# Go hunting for boost compoments
# Defines:
#  BOOST_INCLUDE_DIR


# To find boost on Windows, use DEVLIBS_PATH variable set by mingwenv.bat

FIND_PATH(BOOST_INCLUDE_DIR boost/weak_ptr.hpp 
                            /usr/include 
                            /usr/local/include
                            $ENV{DEVLIBS_PATH}//include )


IF (BOOST_INCLUDE_DIR)
  SET(BOOST_FOUND TRUE)
ENDIF (BOOST_INCLUDE_DIR)

IF (BOOST_FOUND)
     MESSAGE(STATUS "boost: FOUND  ( ${BOOST_INCLUDE_DIR} )")
ELSE(BOOST_FOUND)
     MESSAGE(FATAL_ERROR "boost: NOT FOUND")
ENDIF (BOOST_FOUND)

INCLUDE_DIRECTORIES( ${BOOST_INCLUDE_DIR} )


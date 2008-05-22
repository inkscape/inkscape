
# Include dependencies:
find_package(GTK2 REQUIRED)
find_package(GtkMM REQUIRED)
find_package(SigC++ REQUIRED)
find_package(XML2 REQUIRED)
find_package(XSLT REQUIRED)
find_package(GSL REQUIRED)
find_package(ImageMagick++ REQUIRED)
find_package(Freetype2 REQUIRED)
find_package(GnomeVFS2)
find_package(Boost REQUIRED)
find_package(BoehmGC REQUIRED)
find_package(LibWPG)
find_package(PNG REQUIRED) 
find_package(Popt REQUIRED)
find_package(OpenSSL)
INCLUDE(IncludeJava)
# end Dependencies

#Includes 

INCLUDE_DIRECTORIES( 
"${GTK2_INCLUDE_DIRS}"
"${GtkMM_INCLUDE_DIRS}"
"${SigC++_INCLUDE_DIRS}"
"${XML2_INCLUDE_DIRS}"
"${XSLT_INCLUDE_DIRS}"
"${ImageMagick++_INCLUDE_DIRS}"
"${Freetype2_INCLUDE_DIRS}"
"${GnomeVFS2_INCLUDE_DIRS}"
"${Boost_INCLUDE_DIRS}"
"${BoehmGC_INCLUDE_DIRS}"
"${PNG_INCLUDE_DIRS}"
"${Popt_INCLUDE_DIRS}"
"${OpenSSL_INCLUDE_DIRS}"
"${CMAKE_BINARY_DIR}"
"${PROJECT_SOURCE_DIR}"
 src/
)

#C/C++ Flags
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${BOOST_INCLUDE_DIR} ")
#Defines
#SET(CMAKE_MAKE_PROGRAM "${CMAKE_MAKE_PROGRAM} -j2")

#INCLUDE(ConfigCompileFlags)
INCLUDE(ConfigChecks)

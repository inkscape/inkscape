
# ----------------------------------------------------------------------------
# Files we include
# ----------------------------------------------------------------------------

# Include dependencies:
find_package(GTK2 COMPONENTS gtk gtkmm REQUIRED)
#find_package(GtkMM REQUIRED)
find_package(SigC++ REQUIRED)
find_package(GSL REQUIRED)
find_package(ImageMagick++ REQUIRED)
find_package(Freetype2 REQUIRED)  # our own
find_package(GnomeVFS2)

find_package(BoehmGC REQUIRED)
add_definitions(${BOEHMGC_DEFINITIONS})

find_package(LibWPG)
find_package(PNG REQUIRED)

find_package(Popt REQUIRED)
add_definitions(${POPT_DEFINITIONS})

# ----------------------------------------------------------------------------
# CMake's builtin
# ----------------------------------------------------------------------------

find_package(Boost REQUIRED)

find_package(ASPELL)
add_definitions(${ASPELL_DEFINITIONS})

find_package(OpenSSL)

find_package(LibXslt REQUIRED)
add_definitions(${LIBXSLT_DEFINITIONS})

find_package(LibXml2 REQUIRED)
add_definitions(${LIBXML2_DEFINITIONS})

find_package(OpenMP REQUIRED)  # cmake's
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
if(APPLE AND ${CMAKE_GENERATOR} MATCHES "Xcode")
	set(CMAKE_XCODE_ATTRIBUTE_ENABLE_OPENMP_SUPPORT "YES")
endif()
mark_as_advanced(OpenMP_C_FLAGS)
mark_as_advanced(OpenMP_CXX_FLAGS)

include(IncludeJava)
# end Dependencies


# Linking 
list(APPEND INKSCAPE_LIBS
	# ${GTK2_LIBRARIES}
	# ${SIGC++_LIBRARIES}
	# ${GSL_LIBRARIES}
	${LIBXML2_LIBRARIES}
	${LIBXSLT_LIBRARIES}
	# ${IMAGEMAGICK++_LIBRARIES}
	${FREETYPE2_LIBRARIES}
	# ${GNOMEVFS2_LIBRARIES}
	${Boost_LIBRARIES}
	${BOEHMGC_LIBRARIES}
	${PNG_LIBRARIES}
	${POPT_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${ASPELL_LIBRARIES}
)


# Includes 
set(INK_INCLUDES
	${CMAKE_BINARY_DIR}
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/src
	${GTK2_INCLUDE_DIRS}
	${SIGC++_INCLUDE_DIRS}
	${GSL_INCLUDE_DIRS}
	${LIBXML2_INCLUDE_DIR}
	${LIBXSLT_INCLUDE_DIR}
	${IMAGEMAGICK++_INCLUDE_DIRS}
	${FREETYPE2_INCLUDE_DIRS}
	${GNOMEVFS2_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${BOEHMGC_INCLUDE_DIRS}
	${PNG_INCLUDE_DIR}
	${POPT_INCLUDE_DIRS}
	${OPENSSL_INCLUDE_DIR}
	${ASPELL_INCLUDE_DIR}
)

# C/C++ Flags
include_directories(${INK_INCLUDES})

include(ConfigChecks)

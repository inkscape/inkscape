
set(INKSCAPE_LIBS "")
set(INKSCAPE_INCS "")
set(INKSCAPE_INCS_SYS "")
set(INKSCAPE_CXX_FLAGS "")

list(APPEND INKSCAPE_INCS ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src

    # generated includes
    ${CMAKE_BINARY_DIR}/include
)

# ----------------------------------------------------------------------------
# Add C++11 standard compliance
# TODO: Add a proper check for compiler compliance here
# ----------------------------------------------------------------------------
list(APPEND INKSCAPE_CXX_FLAGS "-std=c++11")

# ----------------------------------------------------------------------------
# Files we include
# ----------------------------------------------------------------------------
if(WIN32)
	# Set the link and include directories
	get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
	
	# MinGW supplied STL does not define these floating point constants.. :/
	add_definitions(-DFLT_EPSILON=1e-9)
	add_definitions(-DFLT_MAX=1e+37)
	add_definitions(-DFLT_MIN=1e-37)

	list(APPEND INKSCAPE_LIBS "-lmscms")

	list(APPEND INKSCAPE_CXX_FLAGS "-mwindows")
	list(APPEND INKSCAPE_CXX_FLAGS "-mthreads")
	
	if(HAVE_MINGW64)
		list(APPEND INKSCAPE_LIBS "-lgomp")
		list(APPEND INKSCAPE_LIBS "-lwinpthread")
	
		list(APPEND INKSCAPE_CXX_FLAGS "-m64")
	else()
		list(APPEND INKSCAPE_CXX_FLAGS "-m32")
	endif()
endif()

pkg_check_modules(INKSCAPE_DEP REQUIRED
	          harfbuzz
	          pangocairo
		  pangoft2
		  fontconfig
		  gthread-2.0
		  gsl
		  gmodule-2.0)

list(APPEND INKSCAPE_LIBS ${INKSCAPE_DEP_LDFLAGS})
list(APPEND INKSCAPE_INCS_SYS ${INKSCAPE_DEP_INCLUDE_DIRS})
list(APPEND INKSCAPE_LIBS ${INKSCAPE_DEP_LIBRARIES})
add_definitions(${INKSCAPE_DEP_CFLAGS_OTHER})

if(APPLE AND DEFINED ENV{CMAKE_PREFIX_PATH})
    list(APPEND INKSCAPE_LIBS "-L$ENV{CMAKE_PREFIX_PATH}/lib")
endif()
if(APPLE)
    if(${GTK+_2.0_TARGET} MATCHES "x11")
    pkg_check_modules(x11 REQUIRED x11)
    list(APPEND INKSCAPE_LIBS ${x11_LDFLAGS})
    endif()
elseif(WIN32)
# X11 not available on windows
else()
    pkg_check_modules(x11 REQUIRED x11)
    list(APPEND INKSCAPE_LIBS ${x11_LDFLAGS})

endif()

if(WITH_GNOME_VFS)
    find_package(GnomeVFS2)
    if(GNOMEVFS2_FOUND)
	list(APPEND INKSCAPE_INCS_SYS ${GNOMEVFS2_INCLUDE_DIR})
	list(APPEND INKSCAPE_LIBS ${GNOMEVFS-2_LIBRARY})
    else()
	set(WITH_GNOME_VFS OFF)
    endif()
endif()

if(ENABLE_LCMS)
    find_package(LCMS2)
    if(LCMS2_FOUND)
	list(APPEND INKSCAPE_INCS_SYS ${LCMS2_INCLUDE_DIRS})
	list(APPEND INKSCAPE_LIBS ${LCMS2_LIBRARIES})
	add_definitions(${LCMS2_DEFINITIONS})
        set (HAVE_LIBLCMS2 1)
    else()
        find_package(LCMS)
        if(LCMS_FOUND)
            list(APPEND INKSCAPE_INCS_SYS ${LCMS_INCLUDE_DIRS})
            list(APPEND INKSCAPE_LIBS ${LCMS_LIBRARIES})
            add_definitions(${LCMS_DEFINITIONS})
            set (HAVE_LIBLCMS1 1)
        else()
            set(ENABLE_LCMS OFF)
        endif()
    endif()
endif()

find_package(Iconv REQUIRED)
list(APPEND INKSCAPE_INCS_SYS ${ICONV_INCLUDE_DIRS})
list(APPEND INKSCAPE_LIBS ${ICONV_LIBRARIES})
add_definitions(${ICONV_DEFINITIONS})

find_package(Intl REQUIRED)
list(APPEND INKSCAPE_INCS_SYS ${Intl_INCLUDE_DIRS})
list(APPEND INKSCAPE_LIBS ${Intl_LIBRARIES})
add_definitions(${Intl_DEFINITIONS})

find_package(BoehmGC REQUIRED)
list(APPEND INKSCAPE_INCS_SYS ${BOEHMGC_INCLUDE_DIRS})
list(APPEND INKSCAPE_LIBS ${BOEHMGC_LIBRARIES})
add_definitions(${BOEHMGC_DEFINITIONS})

if(ENABLE_POPPLER)
    find_package(PopplerCairo)
    if(POPPLER_FOUND)
	set(HAVE_POPPLER ON)
	if(ENABLE_POPPLER_CAIRO)
	    if(POPPLER_CAIRO_FOUND AND POPPLER_GLIB_FOUND)
		set(HAVE_POPPLER_CAIRO ON)	  
	    endif()
	    if(POPPLER_GLIB_FOUND AND CAIRO_SVG_FOUND)
		set(HAVE_POPPLER_GLIB ON)
	    endif()
	endif()
	if(POPPLER_VERSION VERSION_GREATER "0.26.0" OR
		POPPLER_VERSION VERSION_EQUAL   "0.26.0")
	    set(POPPLER_EVEN_NEWER_COLOR_SPACE_API ON)
	endif()
	if(POPPLER_VERSION VERSION_GREATER "0.29.0" OR
		POPPLER_VERSION VERSION_EQUAL   "0.29.0")
	    set(POPPLER_EVEN_NEWER_NEW_COLOR_SPACE_API ON)
	endif()
    else()
	set(ENABLE_POPPLER_CAIRO OFF)
    endif()
else()
    set(HAVE_POPPLER OFF)
    set(ENABLE_POPPLER_CAIRO OFF)
endif()

list(APPEND INKSCAPE_INCS_SYS ${POPPLER_INCLUDE_DIRS})
list(APPEND INKSCAPE_LIBS     ${POPPLER_LIBRARIES})
add_definitions(${POPPLER_DEFINITIONS})

if(WITH_LIBWPG)
    find_package(LibWPG)
    if(LIBWPG_FOUND)
	set(WITH_LIBWPG01 ${LIBWPG-0.1_FOUND})
	set(WITH_LIBWPG02 ${LIBWPG-0.2_FOUND})
	set(WITH_LIBWPG03 ${LIBWPG-0.3_FOUND})
	list(APPEND INKSCAPE_INCS_SYS ${LIBWPG_INCLUDE_DIRS})
	list(APPEND INKSCAPE_LIBS     ${LIBWPG_LIBRARIES})
	add_definitions(${LIBWPG_DEFINITIONS})
    else()
	set(WITH_LIBWPG OFF)
    endif()
endif()

if(WITH_LIBVISIO)
    find_package(LibVisio)
    if(LIBVISIO_FOUND)
	set(WITH_LIBVISIO00 ${LIBVISIO-0.0_FOUND})
	set(WITH_LIBVISIO01 ${LIBVISIO-0.1_FOUND})
	list(APPEND INKSCAPE_INCS_SYS ${LIBVISIO_INCLUDE_DIRS})
	list(APPEND INKSCAPE_LIBS     ${LIBVISIO_LIBRARIES})
	add_definitions(${LIBVISIO_DEFINITIONS})
    else()
	set(WITH_LIBVISIO OFF)
    endif()
endif()

if(WITH_LIBCDR)
    find_package(LibCDR)
    if(LIBCDR_FOUND)
	set(WITH_LIBCDR00 ${LIBCDR-0.0_FOUND})
	set(WITH_LIBCDR01 ${LIBCDR-0.1_FOUND})
	list(APPEND INKSCAPE_INCS_SYS ${LIBCDR_INCLUDE_DIRS})
	list(APPEND INKSCAPE_LIBS     ${LIBCDR_LIBRARIES})
	add_definitions(${LIBCDR_DEFINITIONS})
    else()
	set(WITH_LIBCDR OFF)
    endif()
endif()

FIND_PACKAGE(JPEG)
IF(JPEG_FOUND)
    list(APPEND INKSCAPE_INCS_SYS ${JPEG_INCLUDE_DIR})
    list(APPEND INKSCAPE_LIBS ${JPEG_LIBRARIES})
    set(HAVE_JPEG ON)
ENDIF()

find_package(PNG REQUIRED)
list(APPEND INKSCAPE_INCS_SYS ${PNG_PNG_INCLUDE_DIR})
list(APPEND INKSCAPE_LIBS ${PNG_LIBRARY})

find_package(Popt REQUIRED)
list(APPEND INKSCAPE_INCS_SYS ${POPT_INCLUDE_DIR})
list(APPEND INKSCAPE_LIBS ${POPT_LIBRARIES})
add_definitions(${POPT_DEFINITIONS})

find_package(Potrace)
if(POTRACE_FOUND)
    list(APPEND INKSCAPE_INCS_SYS ${POTRACE_INCLUDE_DIRS})
    list(APPEND INKSCAPE_LIBS ${POTRACE_LIBRARIES})
    set(HAVE_POTRACE ON)
    add_definitions(-DHAVE_POTRACE)
else(POTRACE_FOUND)
    set(HAVE_POTRACE OFF)
    message(STATUS "Could not locate the Potrace library headers: the Trace Bitmap and Paintbucket tools will be disabled")
endif()

if(WITH_DBUS)
    pkg_check_modules(DBUS dbus-1 dbus-glib-1)
    if(DBUS_FOUND)
    list(APPEND INKSCAPE_LIBS ${DBUS_LDFLAGS})
    list(APPEND INKSCAPE_INCS_SYS ${DBUS_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/src/extension/dbus/)
    list(APPEND INKSCAPE_LIBS ${DBUS_LIBRARIES})
    add_definitions(${DBUS_CFLAGS_OTHER})
    
    else()
	set(WITH_DBUS OFF)
    endif()
endif()

if(WITH_SVG2)
	add_definitions(-DWITH_MESH -DWITH_CSSBLEND -DWITH_CSSCOMPOSITE -DWITH_SVG2)
else()
	add_definitions(-UWITH_MESH -UWITH_CSSBLEND -UWITH_CSSCOMPOSITE -UWITH_SVG2)
endif()

if(WITH_LPETOOL)
	add_definitions(-DWITH_LPETOOL -DLPE_ENABLE_TEST_EFFECTS)
else()
	add_definitions(-UWITH_LPETOOL -ULPE_ENABLE_TEST_EFFECTS)
endif()

# ----------------------------------------------------------------------------
# CMake's builtin
# ----------------------------------------------------------------------------

set(TRY_GTKSPELL 1)
# Include dependencies:
# use patched version until GTK2_CAIROMMCONFIG_INCLUDE_DIR is added
    pkg_check_modules(
        GTK3
        REQUIRED
        gtkmm-3.0>=3.8
        gdkmm-3.0>=3.8
        gtk+-3.0>=3.8
        gdk-3.0>=3.8
        gdl-3.0>=3.4
        )
    list(APPEND INKSCAPE_CXX_FLAGS ${GTK3_CFLAGS_OTHER})

    # Check whether we can use new features in Gtkmm 3.10
    # TODO: Drop this test and bump the version number in the GTK test above
    #       as soon as all supported distributions provide Gtkmm >= 3.10
    pkg_check_modules(GTKMM_3_10
	gtkmm-3.0>=3.10,
	)

    if("${GTKMM_3_10_FOUND}")
        message("Using Gtkmm 3.10 build")
        set (WITH_GTKMM_3_10 1)
    endif()

    pkg_check_modules(GDL_3_6 gdl-3.0>=3.6)

    if("${GDL_3_6_FOUND}")
        message("Using GDL 3.6 or higher")
        set (WITH_GDL_3_6 1)
    endif()

    set(TRY_GTKSPELL )
    pkg_check_modules(GTKSPELL3 gtkspell3-3.0)

    if("${GTKSPELL3_FOUND}")
        message("Using GtkSpell 3")
        set (WITH_GTKSPELL 1)
    else()
        unset(WITH_GTKSPELL)
    endif()

    list(APPEND INKSCAPE_INCS_SYS
        ${GTK3_INCLUDE_DIRS}
        ${GTKSPELL3_INCLUDE_DIRS}
    )

    list(APPEND INKSCAPE_LIBS
        ${GTK3_LIBRARIES}
        ${GTKSPELL3_LIBRARIES}
    )

find_package(Freetype REQUIRED)
list(APPEND INKSCAPE_INCS_SYS ${FREETYPE_INCLUDE_DIRS})
list(APPEND INKSCAPE_LIBS ${FREETYPE_LIBRARIES})

find_package(Boost REQUIRED)
list(APPEND INKSCAPE_INCS_SYS ${Boost_INCLUDE_DIRS})
# list(APPEND INKSCAPE_LIBS ${Boost_LIBRARIES})

find_package(ASPELL)
if(ASPELL_FOUND)
    list(APPEND INKSCAPE_INCS_SYS ${ASPELL_INCLUDE_DIR})
    list(APPEND INKSCAPE_LIBS     ${ASPELL_LIBRARIES})
    add_definitions(${ASPELL_DEFINITIONS})
    set(HAVE_ASPELL TRUE)
endif()

#find_package(OpenSSL)
#list(APPEND INKSCAPE_INCS_SYS ${OPENSSL_INCLUDE_DIR})
#list(APPEND INKSCAPE_LIBS ${OPENSSL_LIBRARIES})

find_package(LibXslt REQUIRED)
list(APPEND INKSCAPE_INCS_SYS ${LIBXSLT_INCLUDE_DIR})
list(APPEND INKSCAPE_LIBS ${LIBXSLT_LIBRARIES})
add_definitions(${LIBXSLT_DEFINITIONS})

find_package(LibXml2 REQUIRED)
list(APPEND INKSCAPE_INCS_SYS ${LIBXML2_INCLUDE_DIR})
list(APPEND INKSCAPE_LIBS ${LIBXML2_LIBRARIES})
add_definitions(${LIBXML2_DEFINITIONS})

if(WITH_OPENMP)
    find_package(OpenMP)
    if(OPENMP_FOUND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	list(APPEND INKSCAPE_CXX_FLAGS  ${OpenMP_CXX_FLAGS})
	if(APPLE AND ${CMAKE_GENERATOR} MATCHES "Xcode")
	    set(CMAKE_XCODE_ATTRIBUTE_ENABLE_OPENMP_SUPPORT "YES")
	endif()
	mark_as_advanced(OpenMP_C_FLAGS)
	mark_as_advanced(OpenMP_CXX_FLAGS)
	# '-fopenmp' is in OpenMP_C_FLAGS, OpenMP_CXX_FLAGS and implies '-lgomp'
	# uncomment explicit linking below if still needed:
	set(HAVE_OPENMP ON)
	#list(APPEND INKSCAPE_LIBS "-lgomp")  # FIXME
    else()
	set(HAVE_OPENMP OFF)
	set(WITH_OPENMP OFF)
    endif()
endif()

find_package(ZLIB REQUIRED)
list(APPEND INKSCAPE_INCS_SYS ${ZLIB_INCLUDE_DIRS})
list(APPEND INKSCAPE_LIBS ${ZLIB_LIBRARIES})

if(WITH_IMAGE_MAGICK)
    pkg_check_modules(ImageMagick ImageMagick MagickCore Magick++ )
    if(ImageMagick_FOUND)

        list(APPEND INKSCAPE_LIBS ${ImageMagick_LDFLAGS})
        add_definitions(${ImageMagick_CFLAGS_OTHER})

        list(APPEND INKSCAPE_INCS_SYS ${ImageMagick_INCLUDE_DIRS})
        list(APPEND INKSCAPE_LIBS ${ImageMagick_LIBRARIES})
        else()
	set(WITH_IMAGE_MAGICK OFF)  # enable 'Extensions > Raster'
    endif()
endif()

set(ENABLE_NLS OFF)
if(WITH_NLS)
    find_package(Gettext)
    if(GETTEXT_FOUND)
	message(STATUS "Found gettext + msgfmt to convert language files. Translation enabled")
	set(ENABLE_NLS ON)
    else(GETTEXT_FOUND)
	message(STATUS "Cannot find gettext + msgfmt to convert language file. Translation won't be enabled")
    endif(GETTEXT_FOUND)
endif(WITH_NLS)

#sets c++11 for newer sigc++ if required when pkg-config does not detect it
find_package(SigC++ REQUIRED)

pkg_check_modules(SIGC++ REQUIRED sigc++-2.0 )
list(APPEND INKSCAPE_LIBS ${SIGC++_LDFLAGS})

list(APPEND INKSCAPE_CXX_FLAGS ${SIGC++_CFLAGS_OTHER})

find_package(yaml)
if(YAML_FOUND)
    set (WITH_YAML ON)
    list(APPEND INKSCAPE_INCS_SYS ${YAML_INCLUDE_DIRS})
    list(APPEND INKSCAPE_LIBS ${YAML_LIBRARIES})
    add_definitions(-DWITH_YAML)
else(YAML_FOUND)
    set(WITH_YAML OFF)
    message(STATUS "Could not locate the yaml library headers: xverb feature will be disabled")
endif()

list(REMOVE_DUPLICATES INKSCAPE_CXX_FLAGS)
foreach(flag ${INKSCAPE_CXX_FLAGS})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" CACHE STRING "" FORCE)
endforeach()
# end Dependencies

list(REMOVE_DUPLICATES INKSCAPE_LIBS)
list(REMOVE_DUPLICATES INKSCAPE_INCS_SYS)

# C/C++ Flags
include_directories(${INKSCAPE_INCS})
include_directories(SYSTEM ${INKSCAPE_INCS_SYS})

include(${CMAKE_CURRENT_LIST_DIR}/ConfigChecks.cmake)

unset(INKSCAPE_INCS)
unset(INKSCAPE_INCS_SYS)

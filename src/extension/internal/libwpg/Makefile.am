EXTRA_DIST = \
	libwpg.h.in \
	libwpg.rc.in \
	libwpg-stream.rc.in

# These may be in the builddir too
BUILD_EXTRA_DIST = \
	libwpg.rc \
	libwpg-stream.rc	
	
if PLATFORM_WIN32
no_undefined = -no-undefined
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) 
else
no_undefined =
version_info = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
endif

if OS_WIN32
install-libtool-import-lib:
	$(INSTALL) .libs/libwpg-1.dll.a $(DESTDIR)$(libdir)
	$(INSTALL) .libs/libwpg-stream-1.dll.a $(DESTDIR)$(libdir)

uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libwpg-1.dll.a
	-rm $(DESTDIR)$(libdir)/libwpg-stream-1.dll.a
	
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

lib_LTLIBRARIES = libwpg-1.la libwpg-stream-1.la
libwpg_1_includedir = $(includedir)/libwpg-1/libwpg
libwpg_1_include_HEADERS = \
	libwpg.h \
	WPGraphics.h \
	WPGColor.h \
	WPGPen.h \
	WPGBrush.h \
	WPGGradient.h \
	WPGPoint.h \
	WPGRect.h \
	WPGPath.h \
	WPGStream.h \
	WPGPaintInterface.h 

AM_CXXFLAGS = $(LIBWPG_CXXFLAGS) $(DEBUG_CXXFLAGS)

libwpg_1_la_LIBADD  = $(LIBWPG_LIBS) @LIBWPG_WIN32_RESOURCE@
libwpg_1_la_DEPENDENCIES = @LIBWPG_WIN32_RESOURCE@
libwpg_1_la_LDFLAGS = $(version_info) -export-dynamic $(no_undefined)
libwpg_1_la_SOURCES = \
	WPGraphics.cpp \
	WPGPen.cpp \
	WPGGradient.cpp \
	WPGPoint.cpp \
	WPGPath.cpp \
	WPGHeader.cpp \
	WPGXParser.cpp \
	WPG1Parser.cpp \
	WPG2Parser.cpp \
	libwpg.h \
	libwpg_utils.h \
	WPGColor.h \
	WPGPen.h \
	WPGBrush.h \
	WPGPath.h \
	WPGGradient.h \
	WPGHeader.h \
	WPGXParser.h \
	WPG1Parser.h \
	WPG2Parser.h \
	WPGPaintInterface.h 

libwpg_stream_1_includedir = $(includedir)/libwpg-1/libwpg
libwpg_stream_1_include_HEADERS = WPGStreamImplementation.h

libwpg_stream_1_la_LIBADD  = $(LIBWPG_LIBS) @LIBWPG_STREAM_WIN32_RESOURCE@
libwpg_stream_1_la_DEPENDENCIES = @LIBWPG_STREAM_WIN32_RESOURCE@
libwpg_stream_1_la_LDFLAGS = $(version_info) -export-dynamic $(no_undefined)
libwpg_stream_1_la_SOURCES = \
	WPGOLEStream.cpp \
	WPGStreamImplementation.cpp \
	WPGOLEStream.h

if OS_WIN32

@LIBWPG_WIN32_RESOURCE@ : libwpg.rc
	$(top_srcdir)/build/win32/lt-compile-resource libwpg.rc @LIBWPG_WIN32_RESOURCE@

@LIBWPG_STREAM_WIN32_RESOURCE@ : libwpg.rc
	$(top_srcdir)/build/win32/lt-compile-resource libwpg-stream.rc @LIBWPG_STREAM_WIN32_RESOURCE@

endif


install-data-local: install-libtool-import-lib

uninstall-local: uninstall-libtool-import-lib

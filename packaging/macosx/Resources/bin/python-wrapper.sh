#!/bin/sh


# ---------------------------------------------------------------------
# a) to use PyGTK (for Sozi or inksmoto) from MacPorts
# ---------------------------------------------------------------------

# export PYTHONPATH="$INKSCAPE_SHAREDIR"/extensions
# 
# # unset other environment variables used in Inkscape.app
# unset DYLD_LIBRARY_PATH
# unset XDG_CONFIG_DIRS XDG_DATA_DIRS
# unset GTK_PATH GTK_DATA_PREFIX GTK_EXE_PREFIX GTK_IM_MODULE_FILE
# unset FONTCONFIG_FILE FONTCONFIG_PATH HB_SHAPER_LIST PANGO_RC_FILE PANGO_SYSCONFDIR
# unset GDK_PIXBUF_MODULE_FILE GSETTINGS_SCHEMA_DIR
# unset DBUS_SESSION_BUS_PID DBUS_LAUNCHD_SESSION_BUS_SOCKET DBUS_SESSION_BUS_ADDRESS
# unset GNOME_VFS_MODULE_CONFIG_PATH GNOME_VFS_MODULE_PATH
# unset GIO_MODULE_DIR GVFS_MOUNTABLE_DIR
# unset ASPELL_CONF
# unset POPPLER_DATADIR
# unset VERSIONER_PYTHON_VERSION VERSIONER_PYTHON_PREFER_32_BIT
# unset MAGICK_HOME MAGICK_CONFIGURE_PATH MAGICK_CODER_FILTER_PATH MAGICK_CODER_MODULE_PATH
# unset GS_LIB GS_ICC_PROFILES GS_RESOURCE_DIR GS_LIB GS_FONTPATH GS
# 
# # set locale (language) explicitly
# # (not needed with 0.48.5 package)
# #export LANG="en_US.UTF-8"
# 
# # set MacPorts prefix
# LIBPREFIX="/opt/local"
# 
# exec "$LIBPREFIX/bin/python" "$@"


# ---------------------------------------------------------------------
# b) to test different 32bit or 64bit system-provided Python versions 
#    (available on Mac OS X 10.6 Snow Leopard and later versions)
# ---------------------------------------------------------------------

# # Python 2.5 (system) - Lion: 2.5.6 
# #exec /usr/bin/python2.5 "$@"
# export VERSIONER_PYTHON_VERSION=2.5
# export VERSIONER_PYTHON_PREFER_32_BIT=yes
# exec /usr/bin/python "$@"

# # Python 2.6 (system) - Lion: 2.6.7
# #exec arch -i386 /usr/bin/python2.6 "$@"
# export VERSIONER_PYTHON_VERSION=2.6
# export VERSIONER_PYTHON_PREFER_32_BIT=yes
# #export VERSIONER_PYTHON_PREFER_32_BIT=no
# exec /usr/bin/python "$@"

# # Python 2.7 (system) - Lion: 2.7.1
# #exec arch -i386 /usr/bin/python2.7 "$@"
# export VERSIONER_PYTHON_VERSION=2.7
# export VERSIONER_PYTHON_PREFER_32_BIT=yes
# #export VERSIONER_PYTHON_PREFER_32_BIT=no
# exec /usr/bin/python "$@"


# ---------------------------------------------------------------------
# c) to test different 32bit or 64bit MacPorts-provided Python versions 
#    (define $LIBPREFIX locally)
# ---------------------------------------------------------------------

# LIBPREFIX="/opt/local-x11"
# exec "$LIBPREFIX/bin/python2.5" "$@"
# #exec "$LIBPREFIX/bin/python2.6" "$@"
# #exec "$LIBPREFIX/bin/python2.7" "$@"


# ---------------------------------------------------------------------
# d) ... otherwise run default python
# ---------------------------------------------------------------------

exec /usr/bin/python "$@"

# eof

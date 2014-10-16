#!/bin/bash
#
# simple gimp wrapper script for Inkscape.app
#

#_DEBUG=true

# --- defaults for GIMP.app

app_id="org.gnome.gimp"
app_exec_default="GIMP"


# --- defaults for gimp in $PATH

# path for local gimp install
PATH_local="/opt/local/bin"
# launch a specific gimp version? (e.g. gimp-2.9)
gimp_name="gimp"


# --- unset environment inherited from Inkscape.app

unset XDG_CONFIG_HOME XDG_DATA_HOME XDG_CACHE_HOME
unset XDG_CONFIG_DIRS XDG_DATA_DIRS
unset GTK_PATH GTK_DATA_PREFIX GTK_EXE_PREFIX GTK_IM_MODULE_FILE GTK2_RC_FILES 
unset FONTCONFIG_FILE FONTCONFIG_PATH HB_SHAPER_LIST PANGO_RC_FILE PANGO_SYSCONFDIR
unset GDK_PIXBUF_MODULE_FILE GSETTINGS_SCHEMA_DIR
unset DBUS_SESSION_BUS_PID DBUS_LAUNCHD_SESSION_BUS_SOCKET DBUS_SESSION_BUS_ADDRESS
unset GNOME_VFS_MODULE_CONFIG_PATH GNOME_VFS_MODULE_PATH
unset GIO_MODULE_DIR GVFS_MOUNTABLE_DIR 
unset ASPELL_CONF 
unset POPPLER_DATADIR
unset VERSIONER_PYTHON_VERSION VERSIONER_PYTHON_PREFER_32_BIT PYTHONPATH 
unset MAGICK_HOME MAGICK_CONFIGURE_PATH MAGICK_CODER_FILTER_PATH MAGICK_CODER_MODULE_PATH
unset GS_LIB GS_ICC_PROFILES GS_RESOURCE_DIR GS_LIB GS_FONTPATH GS


# --- detect installed GIMP.app

unset GIMP_APP

APPLESCRIPT1="$(cat << EOM
try
	tell application "Finder"
		set theApp to application file id "$app_id" as string
		set theApp_path to POSIX path of theApp as string
		return theApp_path
	end tell
end try
EOM)"

GIMP_APP="$(osascript -e "$APPLESCRIPT1")"


# --- pass command line arguments to GIMP.app or gimp or exit

if [ ! -z "$GIMP_APP" ]; then

	app_exec="$(defaults read "${GIMP_APP}/Contents/Info.plist" CFBundleExecutable)"
	[[ $? -ne 0 ]] && app_exec="$app_exec_default"

	GIMP_APP_EXEC="${GIMP_APP}/Contents/MacOS/${app_exec}"

	[ $_DEBUG ] && echo "GIMP.app found as: $GIMP_APP" 1>&2
	[ $_DEBUG ] && echo "Command line arguments: $@"  1>&2
	if [ $# -eq 1 ]; then
		[ $_DEBUG ] && echo "open -a $GIMP_APP $@" 1>&2
		open -a "$GIMP_APP" "$@"
	else
		[ $_DEBUG ] && echo "exec $GIMP_APP_EXEC $@" 1>&2
		exec "$GIMP_APP_EXEC" "$@"
	fi

else    # --- test for gimp installed in PATH
	
	# remove CWD from path (we don't want to recursively call this script)
	[ $_DEBUG ] && echo "orig  PATH: $PATH" 1>&2
	PATH_cleaned="$(echo $PATH | sed 's|'"$(cd "$(dirname "$0")" && pwd)"':||g')" || exit 1
	[ $_DEBUG ] && echo "clean PATH: $PATH_cleaned" 1>&2
	export PATH="$PATH_local:$PATH_cleaned"
	[ $_DEBUG ] && echo "final PATH: $PATH" 1>&2

	type -p "$gimp_name"

	if [ $? -eq 0 ]; then
		[ $_DEBUG ] && echo "gimp found in \$PATH: $PATH" 1>&2
		[ $_DEBUG ] && echo "Command line arguments: $@" 1>&2
		exec "$gimp_name" -n "$@"
	else
		echo "Giving up - couldn't find GIMP.app nor gimp." 1>&2
	fi

fi

# eof

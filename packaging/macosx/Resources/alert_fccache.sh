ALERT_SCRIPT="$(cat << EOM
try
	set parent_path to "$CWD"
	set icon_path to POSIX path of (parent_path & "/Inkscape.icns")
	set front_app to ((path to frontmost application) as text)
	tell application front_app
		display dialog "While Inkscape is open, its windows can be displayed or hidden by displaying or hiding the X11 application.

The first time this version of Inkscape is run it may take several minutes before the main window is displayed while font caches are built." buttons {"OK"} default button 1 with title "Inkscape on OS X" with icon POSIX file icon_path
		activate
	end tell
end try
EOM)"

if [ -z "$INK_CACHE_DIR" ]; then
	export INK_CACHE_DIR="${HOME}/.cache/inkscape"
	mkdir -p "$INK_CACHE_DIR"
	[ $_DEBUG ] && echo "INK_CACHE_DIR: falling back to $INK_CACHE_DIR"
fi

# Warn the user about time-consuming generation of fontconfig caches.
if [ ! -f "${INK_CACHE_DIR}/.fccache-new" ]; then 
	alert_result=$(osascript -e "$ALERT_SCRIPT")
	mkdir -p "$INK_CACHE_DIR"
	touch  "${INK_CACHE_DIR}/.fccache-new"
fi

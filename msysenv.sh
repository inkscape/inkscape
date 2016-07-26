#!/bin/bash
# -----------------------------------------------------------------------------
# The paths in this file are used if you want to build Inkscape from the
# the MSYS command line for Windows.
# -----------------------------------------------------------------------------

# Directory containing the precompiled Inkscape libraries. Usually /c/devlibs or /c/devlibs64
if [ -z $DEVLIBS_PATH ]; then
	export DEVLIBS_PATH="/c/devlibs64";
fi

# Directory containing the MinGW instance used for compilation. Usually /c/mingw or /c/mingw64
# Note: Make sure there are no whitespaces in the path. MinGW doesn't like that.. :)
if [ -z $MINGW_PATH ]; then
	export MINGW_PATH="/c/mingw64";
fi
# Directory containing the (optional) Ghostscript installation.

if [ -z $GS_PATH ]; then
	export GS_PATH="/c/latex/gs/gs8.61";
fi

# -----------------------------------------------------------------------------
echo Setting environment variables for MSYS build of Inkscape:
echo
echo  "DEVLIBS_PATH: "$DEVLIBS_PATH
echo  "MINGW_PATH:   "$MINGW_PATH
echo  "GS_PATH:      "$GS_PATH

# Include the MinGW environment in the path to prevent error messages during CMake configure.
export PATH=$DEVLIBS_PATH/bin:$DEVLIBS_PATH/python:$MINGW_PATH/bin:$PATH:$GS_PATH/bin

# Also set the pkgconfig path to prevent error messages during CMake configure.
export PKG_CONFIG_PATH=$DEVLIBS_PATH/lib/pkgconfig

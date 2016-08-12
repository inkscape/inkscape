#!/bin/sh -e

export INKSCAPE_PORTABLE_PROFILE_DIR="${SNAP_USER_DATA}"
export INKSCAPE_LOCALEDIR="${SNAP}/share/locale/"

exec $@

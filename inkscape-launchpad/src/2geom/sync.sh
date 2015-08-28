#!/bin/bash
set -e

function usage {
	echo "2Geom sync to upstream script"
	echo "Usage: $0 path/to/2geom/checkout/dir"
}

if [ "x$(which rsync)" = "x" ]; then
	echo "rsync not found on your system, please install it"
	exit 1
fi

if [ "x$1" = "x" ]; then
	usage $0
	exit 64
fi
if [ ! -d "$1" ]; then
	usage $0
	exit 64
fi
if [ ! -f "$1/src/2geom/path.h" ]; then
	usage $0
	exit 64
fi

INK_2GEOM="$(dirname $0)/"
UPSTREAM_2GEOM="$1/src/2geom/"
rsync -r --existing \
	--exclude CMakeLists.txt --exclude sync.sh \
	"$UPSTREAM_2GEOM" "$INK_2GEOM"

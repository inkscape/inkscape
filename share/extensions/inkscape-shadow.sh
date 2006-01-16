#!/bin/sh
convert -mattecolor "#000f" -frame $((${2} * 3))x$((${2} * 3)) -fx '0' -channel A -blur $((${2} * 3))x${2} $1 $1

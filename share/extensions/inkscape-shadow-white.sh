#!/bin/sh
convert -mattecolor "#ffff" -frame $((${2} * 3))x$((${2} * 3)) -fx '1' -channel A -blur $((${2} * 3))x${2} $1 $1

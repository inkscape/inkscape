#!/bin/sh

# This script is called by Jenkins in the scheduled gtk2 job

./autogen.sh
./configure
make clean
make

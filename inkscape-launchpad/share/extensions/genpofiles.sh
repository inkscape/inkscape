#!/bin/bash

OLDPATH=`pwd`

cd ../..
find share/extensions -name "*.inx" | sort | xargs -n 1 printf "[type: gettext/xml] %s\n"
cd ${OLDPATH}

#!/bin/bash

echo UPDATING
(cd inkscape-launchpad; bzr update)
REVNO=$(cd inkscape-launchpad; bzr revno)
LOG=$(cd inkscape-launchpad; bzr log -r-1 --line)

echo PULLED REVISION $REVNO
git add .
git commit -am "$LOG"

echo GIT COMMITTED, READY TO git push

#!/bin/bash

echo UPDATING
(cd inkscape-launchpad; bzr update)
REVNO=$(cd inkscape-launchpad; bzr revno)

echo PULLED REVISION $REVNO
git commit -am "Pull revision $REVNO"

echo GIT COMMITTED, READY TO git push

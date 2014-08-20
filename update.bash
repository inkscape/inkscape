#!/bin/bash

cd inkscape-launchpad
echo UPDATING
bzr update

bzr revno > ../inkscape-revision.txt


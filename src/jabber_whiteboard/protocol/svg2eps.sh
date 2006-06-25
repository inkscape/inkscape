#!/bin/bash
mkdir eps
for i in *.svg; do \
	inkscape -E eps/`echo $i | sed -r 's/([a-zA-Z0-9\-]+)\.svg/\1/g;'`.eps $i; \
done

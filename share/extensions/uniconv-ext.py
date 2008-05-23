#!/usr/bin/env python

"""
uniconv-ext.py
Python script for running UniConvertor in Inkscape extensions

Copyright (C) 2008 Stephen Silver

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

"""

import os
import random
import subprocess
import sys
import tempfile

# We need a filename ending in ".svg" for UniConvertor.
# This is a hack.
chars = list("0123456789abcdefghijklmnopqrstuvwxyz")
while True:
    random.shuffle(chars)
    svgfile = os.path.join(tempfile.gettempdir(), ''.join(chars) + '.svg')
    if not os.path.exists(svgfile):
        break

# Run UniConvertor, and determine our return code.
try:
    p = subprocess.Popen('uniconv "%s" "%s"' % (sys.argv[1], svgfile),
                         shell=True, stderr=subprocess.PIPE)
    err = p.stderr.read()
    rc = p.wait()
    if rc:
        sys.stderr.write("UniConvertor failed: %s\n" % err)
        rc = 1
except Exception, inst:
    sys.stderr.write("Spawn error: %s\n" % str(inst))
    rc = 1

# If successful, copy the SVG file to stdout.
if rc == 0:
    try:
        f = open(svgfile, "rU")
        for line in f:
            sys.stdout.write(line)
        f.close()
    except IOError, inst:
        sys.stderr.write("Error reading temporary SVG file: %s\n" % str(inst))
        rc = 1

# Clean up and return.
try:
    os.remove(svgfile)
except Exception:
    pass
sys.exit(rc)


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99

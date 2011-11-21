#!/usr/bin/env python

"""
ps2pdf-ext.py
Python script for running ps2pdf in Inkscape extensions

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

import sys
from run_command import run

cmd = 'ps2pdf'
if (sys.argv[1] == "--dEPSCrop=true"): cmd += ' -dEPSCrop '

run((cmd+' "%s" "%%s"') % sys.argv[-1].replace("%","%%"), "ps2pdf")

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99

#!/usr/bin/env python
"""
simplestyle.py
Two simple functions for working with inline css

Copyright (C) 2005 Aaron Spike, aaron@ekips.org

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
"""
def parseStyle(s):
    """Create a dictionary from the value of an inline style attribute"""
    return dict([i.split(":") for i in s.split(";")])
def formatStyle(a):
    """Format an inline style attribute from a dictionary"""
    return ";".join([":".join(i) for i in a.iteritems()])

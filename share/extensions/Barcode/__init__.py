#
# Copyright (C) 2014 Martin Owens
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
"""
Renderer for barcodes, SVG extention for Inkscape.

For supported barcodes see Barcode module directory.
"""

# This lists all known Barcodes missing from this package
# ===== UPC-Based Extensions ====== #
# Code11
# ========= Code25-Based ========== #
# Codabar
# Postnet
# ITF25
# ========= Alpha-numeric ========= #
# Code39Mod
# USPS128 
# =========== 2D Based ============ #
# PDF417
# PDF417-Macro
# PDF417-Truncated
# PDF417-GLI

import sys

class NoBarcode(object):
    """Simple class for no barcode"""
    def generate(self):
        return None

def getBarcode(code, **kw):
    """Gets a barcode from a list of available barcode formats"""
    if not code:
        return sys.stderr.write("No barcode format given!\n")

    code = str(code).replace('-', '').strip()
    mod = 'Barcode'
    try:
        return getattr(__import__(mod+'.'+code, fromlist=[mod]), code)(kw)
    except ImportError:
        sys.stderr.write("Invalid type of barcode: %s\n" % code)
    except AttributeError:
        sys.stderr.write("Barcode module is missing barcode class: %s\n" % code)
    return NoBarcode()


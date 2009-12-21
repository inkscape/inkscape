
'''
Barcodes SVG Extention

Supported Barcodes: EAN8, EAN13, Code39, Code39 Extended, Code93, Code128, RM4CC(RM4SCC)

Copyright (C) 2007 Martin Owens

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
'''

# This lists all known Barcodes missing from this package
# =========== UPC-Based =========== #
# ISBN (EAN13)
# ===== UPC-Based Extensions ====== #
# Code11
# ========= Code25-Based ========== #
# Code25
# Codabar
# Postnet
# ITF25
# ========= Alpha-numeric ========= #
# Code39Mod
# EAN128 (Code128)
# USPS128 
# =========== 2D Based ============ #
# PDF417
# PDF417-Macro
# PDF417-Truncated
# PDF417-GLI

import sys

def getBarcode(format, param={}):
	if format:
		format = str(format).lower()
		format = format.replace('-', '')
		format = format.replace(' ', '')
		if format=='code39':
			import Code39
			return Code39.Object(param)
		elif format=='code39ext':
			import Code39Ext
			return Code39Ext.Object(param)
		elif format=='code93':
			import Code93
			return Code93.Object(param)
		elif format=='code128':
			import Code128
			return Code128.Object(param)

		elif format in ['rm4cc', 'rm4scc']:
			import RM4CC
			return RM4CC.Object(param)

		elif format == 'upca':
			import UPCA
			return UPCA.Object(param)
		elif format == 'upce':
			import UPCE
			return UPCE.Object(param)
		elif format in ['ean13', 'ucc13','jan']:
			import EAN13
			return EAN13.Object(param)
		elif format == 'ean5':
			import EAN5
			return EAN5.Object(param)
		elif format in ['ean8', 'ucc8']:
			import EAN8
			return EAN8.Object(param)
	sys.stderr.write("Invalid format for barcode: " + str(format) + "\n")


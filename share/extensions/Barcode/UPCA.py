#!/usr/bin/env python
'''
Copyright (C) 2007 Martin Owens

Thanks to Lineaire Chez of Inkbar ( www.inkbar.lineaire.net )

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

import EAN13
from EAN13 import mapLeftFaimly, guardBar, centerBar, mapRight
import sys

class Object(EAN13.Object):
	def encode(self, number):
		result = ''

		if len(number) < 11 or len(number) > 12 or not number.isdigit():
			sys.stderr.write("Can not encode '" + number + "' into UPC-A Barcode, Size must be 11 numbers only, and 1 check digit (optional).\n")
			return

		if len(number) == 11:
			number = number + self.getChecksum(number)
		else:
			if not self.verifyChecksum(number):
				sys.stderr.write("UPC-A Checksum not correct for this barcode, omit last character to generate new checksum.\n")
				return

		result = result + guardBar

		i = 0
		for i in range(0,6):
			result += mapLeftFaimly[0][int(number[i])]

		result += centerBar

		for i in range (6,12):
			result += mapRight[int(number[i])]

		result = result + guardBar;

		self.label    = number[0] + '   ' + number[1:6] + '    ' + number[6:11] + '   ' + number[11]
		self.inclabel = self.label
		return result;

	def fontSize(self):
		return '10'

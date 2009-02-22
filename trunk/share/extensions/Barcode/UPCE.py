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
from EAN13 import mapLeftFaimly, guardBar, centerBar
import sys

mapFamily  = [ '000111','001011','001101','001110','010011','011001','011100','010101','010110','011010' ]

class Object(EAN13.Object):
	def encode(self, number):
		result = ''

		l = len(number)

		if (l != 6 and l != 7  and l != 11 and l != 12) or not number.isdigit():
			sys.stderr.write("Can not encode '" + number + "' into UPC-E Barcode, Size must be 6 numbers only, and 1 check digit (optional)\nOr a convertable 11 digit UPC-A number with 1 check digit (also optional).\n")
			return

		echeck = None
		if l==7 or l==12:
			echeck = number[-1]
			number = number[:-1]
			sys.stderr.write("CHECKSUM FOUND!")
			l -= 1

		if l==6:
			number = self.ConvertEtoA(number)

		if not echeck:
			echeck = self.getChecksum(number)
		else:
			if not self.varifyChecksum(number + echeck):
				sys.stderr.write("UPC-E Checksum not correct for this barcode, omit last charicter to generate new checksum.\n")
				return

		number = self.ConvertAtoE(number)
		if not number:
			sys.stderr.write("UPC-A code could not be converted into a UPC-E barcode, please follow the UPC guide or enter a 6 digit UPC-E number..\n")
			return

		number = number

		result = result + guardBar
		# The check digit isn't stored as bars but as a mirroring system. :-(
		family = mapFamily[int(echeck)]

		i = 0
		for i in range(0,6):
			result += mapLeftFaimly[int(family[i])-1][int(number[i])]

		result = result + centerBar + '2';

		self.label    = '0  ' + number[:6] + '  ' + echeck
		self.inclabel = self.label
		return result;

	def fontSize(self):
		return '10'

	def ConvertAtoE(self, number):
		# Converting UPC-A to UPC-E

		# All UPC-E Numbers use number system 0
		if number[0] != '0' or len(number)!=11:
			# If not then the code is invalid
			return None

		# Most of the conversions deal
		# with the specific code parts
		manufacturer = number[1:6]
		product = number[6:11]

		# There are 4 cases to convert:
		if manufacturer[2:] == '000' or manufacturer[2:] == '100' or manufacturer[2:] == '200':
			# Maxium number product code digits can be encoded
			if product[:2]=='00':
				return manufacturer[:2] + product[2:] + manufacturer[2]
		elif manufacturer[3:5] == '00':
			# Now only 2 product code digits can be used
			if product[:3]=='000':
				return manufacturer[:3] + product[3:] + '3'
		elif manufacturer[4] == '0':
			# With even more manufacturer code we have less room for product code
			if product[:4]=='0000':
				return manufacturer[0:4] + product[4] + '4'
		elif product[:4]=='0000' and int(product[4]) > 4:
			# The last recorse is to try and squeeze it in the last 5 numbers
			# so long as the product is 00005-00009 so as not to conflict with
			# the 0-4 used above.
			return manufacturer + product[4]
		else:
			# Invalid UPC-A Numbe
			return None

	def ConvertEtoA(self, number):
		# Convert UPC-E to UPC-A

		# It's more likly to convert this without fault
		# But we still must be mindful of the 4 conversions
		if len(number)!=6:
			return None

		if number[5]=='0' or number[5]=='1' or number[5]=='2':
			return '0' + number[:2] + number[5] + '0000' + number[2:5]
		elif number[5]=='3':
			return '0' + number[:3] + '00000' + number[3:5]
		elif number[5]=='4':
			return '0' + number[:4] + '00000' + number[4]
		else:
			return '0' + number[:5] + '0000' + number[5]


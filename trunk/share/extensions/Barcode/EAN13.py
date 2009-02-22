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

from Base import Barcode
import sys

mapLeftFaimly = [
	[ "0001101","0011001","0010011","0111101","0100011","0110001","0101111","0111011","0110111","0001011" ],
	[ "0100111","0110011","0011011","0100001","0011101","0111001","0000101","0010001","0001001","0010111" ],
]
mapRight   = [ "1110010","1100110","1101100","1000010","1011100","1001110","1010000","1000100","1001000","1110100" ]
mapFaimly  = [ '000000','001011','001101','001110','010011','011001','011100','010101','010110','011010' ]

guardBar = '202';
centerBar = '02020';

class Object(Barcode):
	def encode(self, number):
		result = ''

		if len(number) < 12 or len(number) > 13 or not number.isdigit():
			sys.stderr.write("Can not encode '" + number + "' into EAN13 Barcode, Size must be 12 numbers only\n")
			return

		if len(number) == 12:
			number = number + self.getChecksum(number)
		else:
			if not self.varifyChecksum(number):
				sys.stderr.write("EAN13 Checksum not correct for this barcode, omit last charicter to generate new checksum.\n")
				return

		result = result + guardBar
		family = mapFaimly[int(number[0])]

		i = 0
		for i in range(0,6):
			mapLeft = mapLeftFaimly[int(family[i])]
			result += mapLeft[int(number[i+1])]

		result += centerBar

		for i in range (7,13):
			result += mapRight[int(number[i])]

		result = result + guardBar;

		self.label    = number[0] + '    ' + number[1:7] + '    ' + number[7:] + '       '
		self.inclabel = self.label
		return result;

	def getChecksum(self, number):
		# UPCA/EAN13
		weight=[3,1]*6
		magic=10
		sum = 0
		# We need to work from left to right so reverse
		number = number[::-1]
		# checksum based on first 12 digits.
		for i in range(len(number)):
		   sum = sum + int(number[i]) * weight[i]

		# Mod it down to a single digit
		z = ( magic - (sum % magic) ) % magic
		if z < 0 or z >= magic:
		   return 0

		return str(z)

	def varifyChecksum(self, number):
		new = self.getChecksum(number[:12])
		existing = number[12]
		return new == existing

	def getStyle(self, index):
		result = { 'width' : '1', 'top' : int(self.y), 'write' : True }
		if index==0: # White Space
			result['write'] = False
		elif index==1: # Black Bar
			result['height'] = int(self.height)
		elif index==2: # Guide Bar
			result['height'] = int(self.height) + 5
		return result


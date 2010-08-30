#!/usr/bin/env python
'''
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

from Base import Barcode
import sys

leftMap = [ '0001101', '0011001', '0010011', '0111101', '0100011', '0110001', '0101111', '0111011', '0110111', '0001011' ]
rightMap = [ '1110010', '1100110', '1101100', '1000010', '1011100', '1001110', '1010000', '1000100', '1001000', '1110100' ]
weightMap = [ 3, 1, 3, 1, 3, 1, 3 ]

guardBar = '202';
centerBar = '02020';

class Object(Barcode):
	def encode(self, number):
		result = ''

		# Rejig the label for use
		self.label = number[:4] + '   ' + number[4:]

		if len(number) < 7 or len(number) > 8 or not number.isdigit():
			sys.stderr.write("Can not encode '" + number + "' into EAN8 Barcode, Size must be 7 or 8 Numbers only\n")

		if len(number) == 7:
			number = number + self.calculateChecksum(number)

		result = result + guardBar
	
		i = 0
		for num in number:
			if i >= 4:
				result = result + rightMap[int(num)]
			else:
				result = result + leftMap[int(num)]
			
			i = i + 1
			if i == 4:
				result = result + centerBar;

		result = result + guardBar;

		self.inclabel = '  ' + number[:4] + '   ' + number[4:]
		return result;


	def calculateChecksum(self, number):
		weight = 0;
		i = 0;
	
		for num in number:
			weight = weight + (int(num) * weightMap[i])
			i = i + 1
	
		weight = 10 - (weight % 10)
		if weight == 10:
			weight = 0
		return str(weight);

	def getStyle(self, index):
		result = { 'width' : '1', 'top' : int(self.y), 'write' : True }
		if index==0: # White Space
			result['write'] = False
		elif index==1: # Black Bar
			result['height'] = int(self.height)
		elif index==2: # Guide Bar
			result['height'] = int(self.height) + 8
		return result


#!/usr/bin/env python 
''' 
Copyright (C) 2007 Martin Owens 
Copyright (C) 2009 Aaron C Spike 

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

mapLeftFamily = [
    [ "0001101","0011001","0010011","0111101","0100011","0110001","0101111","0111011","0110111","0001011" ], #L
    [ "0100111","0110011","0011011","0100001","0011101","0111001","0000101","0010001","0001001","0010111" ], #G
]
mapFamily  = [ '11000','10100','10010','10001','01100','00110','00011','01010','01001','00101' ]

startBar = '01011';
sepBar = '01';

class Object(Barcode):
    def encode(self, number):
        result = []
        self.x += 110.0             # horizontal offset so it does not overlap EAN13
        self.y -= self.height + 5   # move the text to the top
        if len(number) != 5 or not number.isdigit():
            sys.stderr.write("Can not encode '" + number + "' into EAN5 Barcode, Size must be 5 numbers only\n")
            return

        check = self.getChecksum(number)
        family = mapFamily[check]

        for i in range(5):
            mapLeft = mapLeftFamily[int(family[i])]
            result.append(mapLeft[int(number[i])])

        self.label = number[0]
        for i in range(1,5):
            self.label += ' ' + number[i]
        self.inclabel = self.label
        return startBar + '01'.join(result)

    def getChecksum(self, number):
        return sum([int(n)*int(m) for n,m in zip(number, '39393')]) % 10

    def getStyle(self, index):
        result = { 'width' : '1', 'top' : int(self.y) + self.height + 5 + int(self.fontSize()), 'write' : True }
        if index==0: # White Space
            result['write'] = False
        elif index==1: # Black Bar
            result['height'] = int(self.height)
        elif index==2: # Guide Bar
            result['height'] = int(self.height) + 5
        return result

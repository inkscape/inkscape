#
# Copyright (C) 2007 Martin Owens
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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
"""
Python barcode renderer for Code93 barcodes. Designed for use with Inkscape.
"""

from Base import Barcode

chars = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%'
encode = list(chars)
encode.append('($)')
encode.append('(/)')
encode.append('(+)')
encode.append('(%)')
encode.append('MARKER')

map = {}

i = 0
for char in encode:
    map[char] = i
    i = i + 1

# Extended encoding maps for full ASCII Code93
def getMap(array):

    result = {}
    y = 10

    for x in array:
        result[chr(x)] = encode[y]
        y = y + 1

    return result;

# MapA is eclectic, but B, C, D are all ASCII ranges
mapA = getMap([27,28,29,30,31,59,60,61,62,63,91,92,93,94,95,123,124,125,126,127,0,64,96,127,127,127]) # %
mapB = getMap(range(1, 26)) # $
mapC = getMap(range(33, 58)) # /
mapD = getMap(range(97, 122)) # +

encoding = '100010100 101001000 101000100 101000010 100101000 100100100 100100010 101010000 100010010 100001010 110101000 110100100 110100010 110010100 110010010 110001010 101101000 101100100 101100010 100110100 100011010 101011000 101001100 101000110 100101100 100010110 110110100 110110010 110101100 110100110 110010110 110011010 101101100 101100110 100110110 100111010 100101110 111010100 111010010 111001010 101101110 101110110 110101110 100100110 111011010 111010110 100110010 101011110'.split()

class Code93(Barcode):
    def encode(self, text):
        # start marker  
        bits = self.encode93('MARKER')

        # Extend to ASCII charset ( return Array )
        text = self.encodeAscii(text)

        # Calculate the checksums
        text.append(self.checksum(text, 20)) # C
        text.append(self.checksum(text, 15)) # K

        # Now convert text into the encoding bits (black and white stripes)    
        for char in text:
            bits = bits + self.encode93(char)

        # end marker  
        bits = bits + self.encode93('MARKER')

        # termination bar
        bits = bits + '1'
    
        self.inclabel = text
        return bits

    def checksum(self, text, mod):
        weight = len(text) % mod
        check  = 0
        for char in text:
            check = check + (map[char] * weight)
            # Reset the weight is required
            weight = weight - 1
            if weight == 0:
                weight = mod

        return encode[check % 47]

    # Some charicters need re-encoding into the code93 specification
    def encodeAscii(self, text):
        result = []
        for char in text:
            if map.has_key(char):
                result.append(char)
            elif mapA.has_key(char):
                result.append('(%)')
                result.append(mapA[char])
            elif mapB.has_key(char):
                result.append('($)')
                result.append(mapB[char])
            elif mapC.has_key(char):
                result.append('(/)')
                result.append(mapC[char])
            elif mapD.has_key(char):
                result.append('(+)')
                result.append(mapD[char])
                
        return result

    def encode93(self, char):
        if map.has_key(char):
            return encoding[map[char]]
        return ''


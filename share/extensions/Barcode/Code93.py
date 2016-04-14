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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA.
#
"""
Python barcode renderer for Code93 barcodes. Designed for use with Inkscape.
"""

from .Base import Barcode

PALLET = list('0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%')
PALLET.append('($)')
PALLET.append('(/)')
PALLET.append('(+)')
PALLET.append('(%)')
PALLET.append('MARKER')

MAP = dict((PALLET[i], i) for i in range(len(PALLET)))

def get_map(array):
    """Extended ENCODE maps for full ASCII Code93"""
    result = {}
    pos = 10
    for char in array:
        result[chr(char)] = PALLET[pos]
        pos = pos + 1
    return result

# MapA is eclectic, but B, C, D are all ASCII ranges
MAP_A = get_map([27, 28, 29, 30, 31, 59, 60, 61, 62, 63, 91, 92, 93, 94, 95,
                 123, 124, 125, 126, 127, 0, 64, 96, 127, 127, 127]) # %
MAP_B = get_map(range(1, 26)) # $
MAP_C = get_map(range(33, 58)) # /
MAP_D = get_map(range(97, 122)) # +

ENCODE = [
    '100010100', '101001000', '101000100', '101000010', '100101000',
    '100100100', '100100010', '101010000', '100010010', '100001010',
    '110101000', '110100100', '110100010', '110010100', '110010010',
    '110001010', '101101000', '101100100', '101100010', '100110100',
    '100011010', '101011000', '101001100', '101000110', '100101100',
    '100010110', '110110100', '110110010', '110101100', '110100110',
    '110010110', '110011010', '101101100', '101100110', '100110110',
    '100111010', '100101110', '111010100', '111010010', '111001010',
    '101101110', '101110110', '110101110', '100100110', '111011010',
    '111010110', '100110010', '101011110', ''
]

class Code93(Barcode):
    def encode(self, text):
        # start marker
        bits = ENCODE[MAP.get('MARKER', -1)]

        # Extend to ASCII charset ( return Array )
        text = self.encode_ascii(text)

        # Calculate the checksums
        text.append(self.checksum(text, 20)) # C
        text.append(self.checksum(text, 15)) # K

        # Now convert text into the ENCODE bits (black and white stripes)
        for char in text:
            bits = bits + ENCODE[MAP.get(char, -1)]

        # end marker and termination bar
        return bits + ENCODE[MAP.get('MARKER', -1)] + '1'

    def checksum(self, text, mod):
        """Generate a code 93 checksum"""
        weight = len(text) % mod
        check = 0
        for char in text:
            check = check + (MAP[char] * weight)
            # Reset the weight is required
            weight = weight - 1
            if weight == 0:
                weight = mod

        return PALLET[check % 47]

    # Some charicters need re-ENCODE into the code93 specification
    def encode_ascii(self, text):
        result = []
        for char in text:
            if MAP.has_key(char):
                result.append(char)
            elif MAP_A.has_key(char):
                result.append('(%)')
                result.append(MAP_A[char])
            elif MAP_B.has_key(char):
                result.append('($)')
                result.append(MAP_B[char])
            elif MAP_C.has_key(char):
                result.append('(/)')
                result.append(MAP_C[char])
            elif MAP_D.has_key(char):
                result.append('(+)')
                result.append(MAP_D[char])
        return result


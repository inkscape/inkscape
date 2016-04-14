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
Python barcode renderer for Code39 barcodes. Designed for use with Inkscape.
"""

from Base import Barcode

ENCODE = {
    '0': '000110100',
    '1': '100100001',
    '2': '001100001',
    '3': '101100000', 
    '4': '000110001',
    '5': '100110000',
    '6': '001110000',
    '7': '000100101',
    '8': '100100100',
    '9': '001100100',
    'A': '100001001',
    'B': '001001001',
    'C': '101001000',
    'D': '000011001',
    'E': '100011000',
    'F': '001011000',
    'G': '000001101',
    'H': '100001100',
    'I': '001001100',
    'J': '000011100',
    'K': '100000011',
    'L': '001000011',
    'M': '101000010',
    'N': '000010011',
    'O': '100010010',
    'P': '001010010',
    'Q': '000000111',
    'R': '100000110',
    'S': '001000110',
    'T': '000010110',
    'U': '110000001',
    'V': '011000001',
    'W': '111000000',
    'X': '010010001',
    'Y': '110010000',
    'Z': '011010000',
    '-': '010000101',
    '*': '010010100',
    '+': '010001010',
    '$': '010101000',
    '%': '000101010',
    '/': '010100010',
    '.': '110000100',
    ' ': '011000100',
}

class Code39(Barcode):
    """Convert a text into string binary of black and white markers"""
    def encode(self, text):
        self.text = text.upper()
        result = ''
        # It isposible for us to encode code39
        # into full ascii, but this feature is
        # not enabled here
        for char in '*' + self.text + '*':
            if not ENCODE.has_key(char):
                char = '-'
            result = result + ENCODE[char] + '0'

        # Now we need to encode the code39, best read
        # the code to understand what it's up to:
        encoded = ''
        colour = '1' # 1 = Black, 0 = White
        for data in result:
            if data == '1':
                encoded = encoded + colour + colour
            else:
                encoded = encoded + colour
            colour = colour == '1' and '0' or '1'

        return encoded


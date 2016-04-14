#
# Authored by Martin Owens <doctormo@gmail.com>
# Debugged by Ralf Heinecke & Martin Siepmann 2007-09-07
#             Horst Schottky 2010-02-27
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
Renderer for Code128/EAN128 codes. Designed for use with Inkscape.
"""

from .Base import Barcode
import re

CODE_MAP = [
      '11011001100', '11001101100', '11001100110', '10010011000', '10010001100',
      '10001001100', '10011001000', '10011000100', '10001100100', '11001001000',
      '11001000100', '11000100100', '10110011100', '10011011100', '10011001110',
      '10111001100', '10011101100', '10011100110', '11001110010', '11001011100',
      '11001001110', '11011100100', '11001110100', '11101101110', '11101001100',
      '11100101100', '11100100110', '11101100100', '11100110100', '11100110010',
      '11011011000', '11011000110', '11000110110', '10100011000', '10001011000',
      '10001000110', '10110001000', '10001101000', '10001100010', '11010001000',
      '11000101000', '11000100010', '10110111000', '10110001110', '10001101110',
      '10111011000', '10111000110', '10001110110', '11101110110', '11010001110',
      '11000101110', '11011101000', '11011100010', '11011101110', '11101011000',
      '11101000110', '11100010110', '11101101000', '11101100010', '11100011010',
      '11101111010', '11001000010', '11110001010', '10100110000', '10100001100',
      '10010110000', '10010000110', '10000101100', '10000100110', '10110010000',
      '10110000100', '10011010000', '10011000010', '10000110100', '10000110010',
      '11000010010', '11001010000', '11110111010', '11000010100', '10001111010',
      '10100111100', '10010111100', '10010011110', '10111100100', '10011110100',
      '10011110010', '11110100100', '11110010100', '11110010010', '11011011110',
      '11011110110', '11110110110', '10101111000', '10100011110', '10001011110',
      '10111101000', '10111100010', '11110101000', '11110100010', '10111011110',
      '10111101110', '11101011110', '11110101110', '11010000100', '11010010000',
      '11010011100', '11000111010', '11']

def map_extra(data, chars):
    """Maps the data into the chars"""
    result = list(data)
    for char in chars:
        result.append(chr(char))
    result.append('FNC3')
    result.append('FNC2')
    result.append('SHIFT')
    return result

# The map_extra method is used to slim down the amount
# of pre code and instead we generate the lists
CHAR_AB = list(" !\"#$%&\'()*+,-./0123456789:;<=>?@"
               "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_")
CHAR_A = map_extra(CHAR_AB, range(0, 31)) # Offset 64
CHAR_B = map_extra(CHAR_AB, range(96, 125)) # Offset -32

class Code128(Barcode):
    """Main barcode object, generates the encoding bits here"""
    def encode(self, text):
        blocks = []
        block = ''

        # Split up into sections of numbers, or charicters
        # This makes sure that all the charicters are encoded
        # In the best way posible for Code128
        for datum in re.findall(r'(?:(?:\d\d){2,})|(?:^\d\d)|.', text):
            if len(datum) == 1:
                block = block + datum
            else:
                if block:
                    blocks.append(self.best_block(block))
                    block = ''
                blocks.append(['C', datum])

        if block:
            blocks.append(self.best_block(block))
            block = ''

        return self.encode_blocks(blocks)

    def best_block(self, block):
        """If this has lower case then select B over A"""
        if block.upper() == block:
            return ['A', block]
        return ['B', block]

    def encode_blocks(self, blocks):
        """Encode the given blocks into A, B or C codes"""
        encode = ''
        total = 0
        pos = 0

        for block in blocks:
            b_set = block[0]
            datum = block[1]

            # POS :   0,   1
            # A   : 101, 103
            # B   : 100, 104
            # C   :  99, 105
            num = 0
            if b_set == 'A':
                num = 103
            elif b_set == 'B':
                num = 104
            elif b_set == 'C':
                num = 105

            i = pos
            if pos:
                num = 204 - num
            else:
                i = 1

            total = total + num * i
            encode = encode + CODE_MAP[num]
            pos = pos + 1

            if b_set == 'A' or b_set == 'B':
                chars = CHAR_B
                if b_set == 'A':
                    chars = CHAR_A

                for char in datum:
                    total = total + (chars.index(char) * pos)
                    encode = encode + CODE_MAP[chars.index(char)]
                    pos = pos + 1
            else:
                for char in (datum[i:i+2] for i in range(0, len(datum), 2)):
                    total = total + (int(char) * pos)
                    encode = encode + CODE_MAP[int(char)]
                    pos = pos + 1

        checksum = total % 103
        encode = encode + CODE_MAP[checksum]
        encode = encode + CODE_MAP[106]
        encode = encode + CODE_MAP[107]

        return encode


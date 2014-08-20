#
# Copyright (C) 2010 Martin Owens
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
Some basic common code shared between EAN and UCP generators.
"""

from Base import Barcode
import sys

MAPPING = [
    # Left side of barcode Family '0'
    [ "0001101", "0011001", "0010011", "0111101", "0100011",
      "0110001", "0101111", "0111011", "0110111", "0001011" ],
    # Left side of barcode Family '1' and flipped to right side.
    [ "0100111", "0110011", "0011011", "0100001", "0011101",
      "0111001", "0000101", "0010001", "0001001", "0010111" ],
]
# This chooses which of the two encodings above to use.
FAMILIES  = [ '000000', '001011', '001101', '001110', '010011',
              '011001', '011100', '010101', '010110', '011010' ]

GUARD_BAR  = '202'
CENTER_BAR = '02020'

class EanBarcode(Barcode):
    """Simple base class for all EAN type barcodes"""
    length  = None
    lengths = None
    checks  = []

    def intarray(self, number):
        """Convert a string of digits into an array of ints"""
        return [ int(i) for i in number ]


    def encode_interleaved(self, family, number, fams=FAMILIES):
        """Encode any side of the barcode, interleaved"""
        result = []
        encset = self.intarray(fams[family])
        for i in range(len(number)):
            thismap = MAPPING[encset[i]]
            result.append( thismap[number[i]] )
        return result


    def encode_right(self, number):
        """Encode the right side of the barcode, non-interleaved"""
        result = []
        for n in number:
            # The right side is always the reverse of the left's family '1'
            result.append( MAPPING[1][n][::-1] )
        return result


    def encode_left(self, number):
        """Encode the left side of the barcode, non-interleaved"""
        result = []
        for n in number:
            result.append( MAPPING[0][n] )
        return result


    def space(self, *spacing):
        """Space out an array of numbers"""
        result = ''
        for space in spacing:
            if isinstance(space, list):
                for i in space:
                    result += str(i)
            elif isinstance(space, int):
                result += ' ' * space
        return result


    def getLengths(self):
        """Return a list of acceptable lengths"""
        if self.length:
            return [ self.length ]
        return self.lengths[:]


    def encode(self, code):
        """Encode any EAN barcode"""
        if not code.isdigit():
            return self.error(code, 'Not a Number, must be digits 0-9 only')
        lengths = self.getLengths() + self.checks

        if len(code) not in lengths:
            return self.error(code, 'Wrong size, must be %s digits' % 
                (', '.join(self.space(lengths))))

        if self.checks:
            if len(code) not in self.checks:
                code = self.appendChecksum(code)
            elif not self.verifyChecksum(code):
                return self.error(code, 'Checksum failed, omit for new sum')
        return self._encode(self.intarray(code))


    def _encode(self, n):
        raise NotImplementedError("_encode should be provided by parent EAN")

    def enclose(self, left, right=[], guard=GUARD_BAR, center=CENTER_BAR):
        """Standard Enclosure"""
        parts = [ guard ] + left + [ center ] + right + [ guard ]
        return ''.join( parts )

    def getChecksum(self, number, magic=10):
        """Generate a UPCA/EAN13/EAN8 Checksum"""
        weight = [3,1] * len(number)
        result = 0
        # We need to work from left to right so reverse
        number = number[::-1]
        # checksum based on first digits.
        for i in range(len(number)):
           result += int(number[i]) * weight[i]
        # Modulous result to a single digit checksum
        checksum = magic - (result % magic)
        if checksum < 0 or checksum >= magic:
           return '0'
        return str(checksum)

    def appendChecksum(self, number):
        """Apply the checksum to a short number"""
        return number + self.getChecksum(number)

    def verifyChecksum(self, number):
        """Verify any checksum"""
        return self.getChecksum(number[:-1]) == number[-1]


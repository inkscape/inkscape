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
Python barcode renderer for UPCE barcodes. Designed for use with Inkscape.
"""

from BaseEan import EanBarcode
import sys

# This is almost exactly the same as the standard FAMILIES
# But flipped around and with the first 111000 instead of 000000.
FAMS  = [ '111000', '110100', '110010', '110001', '101100',
          '100110', '100011', '101010', '101001', '100101' ]

class Upce(EanBarcode):
    """Generate EAN6/UPC-E barcode generator"""
    name    = 'upce'
    lengths = [ 6, 11 ]
    checks  = [ 7, 12 ]

    def _encode(self, n):
        """Generate a UPC-E Barcode"""
        self.label = self.space(['0'], 2, n[:6], 2, n[-1])
        code = self.encode_interleaved(n[-1], n[:6], FAMS)
        # 202(guard) + code + 020(center) + 202(guard)
        return self.enclose(code, center='020')

    def appendChecksum(self, number):
        """Generate a UPCE Checksum"""
        if len(number) == 6:
            number = self.ConvertEtoA(number)
        result = self.getChecksum(number)
        return self.ConvertAtoE(number) + result

    def fontSize(self):
        """We need a font size of 10"""
        return 10

    def ConvertAtoE(self, number):
        """Converting UPC-A to UPC-E, may cause errors."""
        # All UPC-E Numbers use number system 0
        if number[0] != '0' or len(number)!=11:
            # If not then the code is invalid
            raise ValueError("Invalid UPC Number")

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
            raise ValueError("Invalid UPC Number")

    def ConvertEtoA(self, number):
        """Convert UPC-E to UPC-A by padding with zeros"""
        # It's more likly to convert this without fault
        # But we still must be mindful of the 4 conversions
        if len(number) != 6:
            return None

        if number[5] in ['0', '1', '2']:
            return '0' + number[:2] + number[5] + '0000' + number[2:5]
        elif number[5] == '3':
            return '0' + number[:3] + '00000' + number[3:5]
        elif number[5] == '4':
            return '0' + number[:4] + '00000' + number[4]
        else:
            return '0' + number[:5] + '0000' + number[5]


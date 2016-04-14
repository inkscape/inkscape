#
# Copyright (C) 2009 Aaron C Spike
#               2010 Martin Owens
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
Python barcode renderer for EAN5 barcodes. Designed for use with Inkscape.
"""

from .BaseEan import EanBarcode

FAMS = ['11000', '10100', '10010', '10001', '01100',
        '00110', '00011', '01010', '01001', '00101']
START = '01011'

class Ean5(EanBarcode):
    """Provide an Ean5 barcode generator"""
    name = 'ean5'
    length = 5

    def _encode(self, number):
        self.text = ' '.join(self.space(number))
        family = sum([int(n)*int(m) for n, m in zip(number, '39393')]) % 10
        return START + '01'.join(self.encode_interleaved(family, number, FAMS))


#
# Copyright (C) 2016 Martin Owens
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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110, USA.
#
"""
Python barcode renderer for EAN2 barcodes. Designed for use with Inkscape.
"""

from .BaseEan import EanBarcode

FAMS = ['00', '01', '10', '11']
START = '01011'

class Ean2(EanBarcode):
    """Provide an Ean5 barcode generator"""
    length = 2
    name = 'ean5'

    def _encode(self, number):
        if len(number) != 2:
            number = ([0, 0] + number)[-2:]
        self.text = ' '.join(self.space(number))
        family = ((number[0] * 10) + number[1]) % 4
        return START + '01'.join(self.encode_interleaved(family, number, FAMS))


#
# Copyright (C) 2010 Martin Owens
#
# Thanks to Lineaire Chez of Inkbar ( www.inkbar.lineaire.net )
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
Python barcode renderer for EAN13 barcodes. Designed for use with Inkscape.
"""

from .BaseEan import EanBarcode

class Ean13(EanBarcode):
    """Provide an Ean13 barcode generator"""
    name = 'ean13'
    extras = {2: 'Ean2', 5: 'Ean5'}
    checks = [13]
    lengths = [12]

    def _encode(self, n):
        """Encode an ean13 barcode"""
        self.text = self.space(n[0:1], 4, n[1:7], 5, n[7:], 7)
        return self.enclose(
            self.encode_interleaved(n[0], n[1:7]), self.encode_right(n[7:]))



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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA.
#
"""
Python barcode renderer for EAN8 barcodes. Designed for use with Inkscape.
"""

from .BaseEan import EanBarcode

class Ean8(EanBarcode):
    """Provide an EAN8 barcode generator"""
    name = 'ean8'
    checks = [8]
    lengths = [7]

    def _encode(self, n):
        """Encode an ean8 barcode"""
        self.text = self.space(n[:4], 3, n[4:])
        return self.enclose(self.encode_left(n[:4]), self.encode_right(n[4:]))


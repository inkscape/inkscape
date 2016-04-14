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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
"""
Python barcode renderer for RM4CC barcodes. Designed for use with Inkscape.
"""

from Base import Barcode

map = {
    '(' : '25',
    ')' : '3',
    '0' : '05053535',
    '1' : '05152535',
    '2' : '05153525',
    '3' : '15052535',
    '4' : '15053525',
    '5' : '15152525',
    '6' : '05251535',
    '7' : '05350535',
    '8' : '05351525',
    '9' : '15250535',
    'A' : '15251525',
    'B' : '15350525',
    'C' : '05253515',
    'D' : '05352515',
    'E' : '05353505',
    'F' : '15252515',
    'G' : '15253505',
    'H' : '15352505',
    'I' : '25051535',
    'J' : '25150535',
    'K' : '25151525',
    'L' : '35050535',
    'M' : '35051525',
    'N' : '35150525',
    'O' : '25053525',
    'P' : '25152515',
    'Q' : '25153505',
    'R' : '35052515',
    'S' : '35053505',
    'T' : '35152505',
    'U' : '25251515',
    'V' : '25350515',
    'W' : '25351505',
    'X' : '35250515',
    'Y' : '35251505',
    'Z' : '35350505',
}

check = ['ZUVWXY','501234','B6789A','HCDEFG','NIJKLM','TOPQRS']
(BAR_TRACK, BAR_DOWN, BAR_UP, BAR_FULL, BAR_NONE, WHITE_SPACE) = range(6)

class Rm4scc(Barcode):
    default_height = 18

    def encode(self, text):
        result = ''

        text = text.upper()
        text.replace('(', '')
        text.replace(')', '')

        text = '(' + text + self.checksum(text) + ')'

        i = 0
        for char in text:
            if map.has_key(char):
                result = result + map[char]
                i = i + 1

        return result;

    # given a string of data, return the check character
    def checksum(self, text):
        total_lower = 0
        total_upper = 0
        for char in text:
            if map.has_key(char):
                bars = map[char][0:8:2]
                lower = 0
                upper = 0

                if int(bars[0]) & 1:
                    lower = lower + 4 
                if int(bars[1]) & 1:
                    lower = lower + 2
                if int(bars[2]) & 1:
                    lower = lower + 1
                if int(bars[0]) & 2:
                    upper = upper + 4
                if int(bars[1]) & 2:
                    upper = upper + 2
                if int(bars[2]) & 2:
                    upper = upper + 1
            total_lower = total_lower + (lower % 6)
            total_upper = total_upper + (upper % 6)

        total_lower = total_upper % 6
        total_upper = total_upper % 6
    
        checkchar = check[total_upper][total_lower]
        return checkchar

    def get_style(self, index):
        """Royal Mail Barcodes use a completely different style"""
        result = { 'width' : 2, 'write' : True, 'top' : 0 }
        if index == BAR_TRACK: # Track Bar
            result['top']    = 6
            result['height'] = 5
        elif index == BAR_DOWN: # Decender Bar
            result['top']    = 6
            result['height'] = 11
        elif index == BAR_UP: # Accender Bar
            result['height'] = 11
        elif index == BAR_FULL: # Full Bar
            result['height'] = 17
        elif index == WHITE_SPACE: # White Space
            result['write']  = False
        return result

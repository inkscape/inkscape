#!/usr/bin/env python

import unittest, sys
sys.path.append('..') # this line allows to import the extension code

from simplestyle import parseColor, parseStyle


class ParseColorTest(unittest.TestCase):
    """Test for single transformations"""
    def test_namedcolor(self):
        "Parse 'red'"
        col = parseColor('red')
        self.failUnlessEqual((255,0,0),col)
        
    def test_hexcolor4digit(self):
        "Parse '#ff0102'"
        col = parseColor('#ff0102')
        self.failUnlessEqual((255,1,2),col)
        
    def test_hexcolor3digit(self):
        "Parse '#fff'"
        col = parseColor('#fff')
        self.failUnlessEqual((255,255,255),col)
        
    def test_rgbcolorint(self):
        "Parse 'rgb(255,255,255)'"
        col = parseColor('rgb(255,255,255)')
        self.failUnlessEqual((255,255,255),col)
        
    def test_rgbcolorpercent(self):
        "Parse 'rgb(100%,100%,100%)'"
        col = parseColor('rgb(100%,100%,100%)')
        self.failUnlessEqual((255,255,255),col)
        
    def test_rgbcolorpercent2(self):
        "Parse 'rgb(100%,100%,100%)'"
        col = parseColor('rgb(50%,0%,1%)')
        self.failUnlessEqual((127,0,2),col)
        
    def test_rgbcolorpercentdecimal(self):
        "Parse 'rgb(66.667%,0%,6.667%)'"
        col = parseColor('rgb(66.667%,0%,6.667%)')
        self.failUnlessEqual((170, 0, 17),col)
    
    # TODO: This test appears to be broken.  parseColor can
    #       only return an RGB colour code
    #def test_currentColor(self):
    #    "Parse 'currentColor'"
    #    col = parseColor('currentColor')
    #    self.failUnlessEqual(('currentColor'),col)

    def test_spaceinstyle(self):
        "Parse 'stop-color: rgb(0,0,0)'"
        col = parseStyle('stop-color: rgb(0,0,0)')
        self.failUnlessEqual({'stop-color': 'rgb(0,0,0)'},col)

    #def test_unknowncolor(self):
    #    "Parse 'unknown'"
    #    col = parseColor('unknown')
    #    self.failUnlessEqual((0,0,0),col)
    #
    
if __name__ == '__main__':
    unittest.main()

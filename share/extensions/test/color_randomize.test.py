#!/usr/bin/env python
'''
Unit test file for ../color_randomize.py
--
If you want to help, read the python unittest documentation:
http://docs.python.org/library/unittest.html
'''

import os
import sys
import unittest

sys.path.append('..') # this line allows to import the extension code
from color_randomize import *

def extract_hsl(e,rgb):
    r = int(rgb[:2], 16)
    g = int(rgb[2:4], 16)
    b = int(rgb[4:6], 16)
    return e.rgb_to_hsl(r/255.0, g/255.0, b/255.0)


class ColorRandomizeBasicTest(unittest.TestCase):
    def setUp(self):
        self.e=C()

    def test_default_values(self):
        """ The default ranges are set to 0, and thus the color and opacity should not change. """
        args = ['svg/empty-SVG.svg']
        self.e.affect(args, False)
        col = self.e.colmod(128, 128, 255)
        self.assertEqual("8080ff", col)
        opac = self.e.opacmod(5)
        self.assertEqual(5, opac)


class ColorRandomizeColorModificationTest(unittest.TestCase):
    def setUp(self):
        self.e=C()

    def test_no_change(self):
        """ The user selected 0% values, and thus the color should not change. """
        args = ['-y 0', '-t 0', '-m 0', 'svg/empty-SVG.svg']
        self.e.affect(args, False)
        col = self.e.colmod(128, 128, 255)
        self.assertEqual("8080ff", col)

    def test_random_hue(self):
        """ Random hue only. Saturation and lightness not changed. """
        args = ['-y 50','-t 0','-m 0','svg/empty-SVG.svg']
        self.e.affect(args, False)
        hsl = extract_hsl(self.e, self.e.colmod(150, 100, 200))
        self.assertEqual([0.47, 0.59], [round(hsl[1], 2), round(hsl[2], 2)])

    @unittest.skip("Inaccurate convertion")
    def test_random_lightness(self):
        """ Random lightness only. Hue and saturation not changed. """
        args = ['-y 0', '-t 0', '-m 50', 'svg/empty-SVG.svg']
        self.e.affect(args, False)
        hsl = extract_hsl(self.e, self.e.colmod(150, 100, 200))
        # Lightness change also affects hue and saturation...
        #self.assertEqual(0.75, round(hsl[0], 2))
        #self.assertEqual(0.48, round(hsl[1], 2))

    def test_random_saturation(self):
        """ Random saturation only. Hue and lightness not changed. """
        args = ['-y 0', '-t 50', '-m 0', 'svg/empty-SVG.svg']
        self.e.affect(args, False)
        hsl = extract_hsl(self.e, self.e.colmod(150, 100, 200))
        self.assertEqual([0.75, 0.59], [round(hsl[0], 2), round(hsl[2], 2)])

    def test_range_limits(self):
        """ The maximum hsl values should be between 0 and 100% of their maximum """
        args = ['-y 100', '-t 100', '-m 100', 'svg/empty-SVG.svg']
        self.e.affect(args, False)
        hsl = extract_hsl(self.e, self.e.colmod(156, 156, 156))
        self.assertLessEqual([hsl[0], hsl[1], hsl[2]], [1, 1, 1])
        self.assertGreaterEqual([hsl[0], hsl[1], hsl[2]], [0, 0, 0])


class ColorRandomizeOpacityModificationTest(unittest.TestCase):
    def setUp(self):
        self.e=C()

    def test_no_change(self):
        """ The user selected 0% opacity range, and thus the opacity should not change. """
        args = ['-o 0', 'svg/empty-SVG.svg']
        self.e.affect(args, False)
        opac = self.e.opacmod(0.15)
        self.assertEqual(0.15, opac)

    def test_range_min_limit(self):
        """ The opacity value should be greater than 0 """
        args = ['-o 100', 'svg/empty-SVG.svg']
        self.e.affect(args, False)
        opac = self.e.opacmod(0)
        self.assertGreaterEqual(opac, "0")

    def test_range_max_limit(self):
        """ The opacity value should be lesser than 1 """
        args = ['-o 100', 'svg/empty-SVG.svg']
        self.e.affect(args, False)
        opac = self.e.opacmod(1)
        self.assertLessEqual(opac, "1")

    def test_non_float_opacity(self):
        """ Non-float opacity value not changed """
        args = ['-o 100', 'svg/empty-SVG.svg']
        self.e.affect(args, False)
        opac = self.e.opacmod("toto")
        self.assertLessEqual(opac, "toto")

if __name__ == '__main__':
    #unittest.main()
    suite = unittest.TestLoader().loadTestsFromTestCase(ColorRandomizeBasicTest)
    unittest.TextTestRunner(verbosity=2).run(suite)
    suite = unittest.TestLoader().loadTestsFromTestCase(ColorRandomizeColorModificationTest)
    unittest.TextTestRunner(verbosity=2).run(suite)
    suite = unittest.TestLoader().loadTestsFromTestCase(ColorRandomizeOpacityModificationTest)
    unittest.TextTestRunner(verbosity=2).run(suite)


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99

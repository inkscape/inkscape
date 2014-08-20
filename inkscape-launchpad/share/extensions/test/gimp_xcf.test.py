#!/usr/bin/env python
'''
Unit test file for ../gimp_xcf.py
Revision history:
  * 2012-01-26 (jazzynico): checks defaulf parameters and file handling.

--
If you want to help, read the python unittest documentation:
http://docs.python.org/library/unittest.html
'''

import os
import sys
import unittest

sys.path.append('..') # this line allows to import the extension code
from gimp_xcf import *

class GimpXCFBasicTest(unittest.TestCase):
    def setUp(self):
        sys.stdout = open(os.devnull, 'w')

    def test_expected_file(self):
        # multilayered-test.svg provides 3 layers and a sublayer
        # (all non empty).
        args = [ 'svg/multilayered-test.svg' ]
        e = MyEffect()
        e.affect(args, False)
        #self.assertRaises(GimpXCFExpectedIOError, e.affect, args, False)

    def test_empty_file(self):
        # empty-SVG.svg contains an emply svg element (no layer, no object).
        # The file must have at least one non empty layer and thus the
        # extension rejects it and send an error message.
        args = [ 'svg/empty-SVG.svg' ]
        e = MyEffect()
        e.affect(args, False)
        self.assertEqual(e.valid, 0)

    def test_empty_layer_file(self):
        # default-inkscape-SVG.svg is a copy of the defaut Inkscape
        # template, with one empty layer.
        # The file must have at least one non empty layer and thus the
        # extension rejects it and send an error message.
        args = [ 'svg/default-inkscape-SVG.svg' ]
        e = MyEffect()
        e.affect(args, False)
        self.assertEqual(e.valid, 0)


if __name__ == '__main__':
    #unittest.main()
    suite = unittest.TestLoader().loadTestsFromTestCase(GimpXCFBasicTest)
    unittest.TextTestRunner(verbosity=2).run(suite)

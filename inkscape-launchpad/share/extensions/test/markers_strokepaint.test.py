#!/usr/bin/env python
'''
Unit test file for ../markers_strokepaint.py
Revision history:
  * 2012-01-27 (jazzynico): checks defaulf parameters and file handling.

--
If you want to help, read the python unittest documentation:
http://docs.python.org/library/unittest.html
'''

import sys
import unittest

sys.path.append('..') # this line allows to import the extension code
from markers_strokepaint import *

class StrokeColorBasicTest(unittest.TestCase):

    #def setUp(self):

    def test_no_defs(self):
        args = ['svg/empty-SVG.svg']
        e = MyEffect()
        e.affect(args, False)
        #self.assertEqual( e.something, 'some value', 'A commentary about that.' )

    def test_empty_defs(self):
        args = ['svg/default-plain-SVG.svg']
        e = MyEffect()
        e.affect(args, False)
'''
    def test_markers(self):
        args = ['svg/markers.svg']
'''

if __name__ == '__main__':
    #unittest.main()
    suite = unittest.TestLoader().loadTestsFromTestCase(StrokeColorBasicTest)
    unittest.TextTestRunner(verbosity=2).run(suite)

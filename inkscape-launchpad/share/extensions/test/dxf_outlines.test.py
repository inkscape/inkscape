#!/usr/bin/env python
'''
Unit test file for ../dxf_outlines.py
Revision history:
  * 2012-01-25 (jazzynico): first working version (only checks the extension
    with the default parameters).

--
If you want to help, read the python unittest documentation:
http://docs.python.org/library/unittest.html
'''

import sys
import unittest

sys.path.append('..') # this line allows to import the extension code
from dxf_outlines import *

class DFXOutlineBasicTest(unittest.TestCase):
    #def setUp(self):

    def test_run_without_parameters(self):
        args = [ 'minimal-blank.svg' ]
        e = MyEffect()
        e.affect(args, False)


if __name__ == '__main__':
    unittest.main()
    #suite = unittest.TestLoader().loadTestsFromTestCase(DFXOutlineBasicTest)
    #unittest.TextTestRunner(verbosity=2).run(suite)


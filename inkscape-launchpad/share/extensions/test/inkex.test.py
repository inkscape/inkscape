#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
Unit test file for ../inkex.py
Revision history:
  * 2012-01-27 (jazzynico): check errormsg function.

--
If you want to help, read the python unittest documentation:
http://docs.python.org/library/unittest.html
'''

import sys
sys.path.append('..') # this line allows to import the extension code

import unittest
from inkex import errormsg

class InkexBasicTest(unittest.TestCase):

    #def setUp(self):

    def test_ascii(self):
        #Parse ABCabc
        errormsg('ABCabc')

    def test_nonunicode_latin1(self):
        #Parse Àûïàèé
        errormsg('Àûïàèé')

    def test_unicode_latin1(self):
        #Parse Àûïàèé (unicode)
        errormsg(u'Àûïàèé')

if __name__ == '__main__':
    #unittest.main()
    suite = unittest.TestLoader().loadTestsFromTestCase(InkexBasicTest)
    unittest.TextTestRunner(verbosity=2).run(suite)

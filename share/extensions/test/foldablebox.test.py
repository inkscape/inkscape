#!/usr/bin/env python

import sys
sys.path.append('..') # this line allows to import the extension code

import unittest, calendar
from foldablebox import *

class FoldableBoxArguments(unittest.TestCase):

  #def setUp(self):

  def test_default_names_list(self):
    args = [ 'minimal-blank.svg' ]
    e = FoldableBox()
    e.affect( args, False )
    self.assertEqual( e.options.width, 10.0 )

if __name__ == '__main__':
  unittest.main()


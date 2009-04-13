#!/usr/bin/env python

import sys
sys.path.append('..') # this line allows to import the extension code

import unittest
from foldablebox import *

class FoldableBoxArguments(unittest.TestCase):

  #def setUp(self):

  def test_basic_box_elements(self):
    args = [ 'minimal-blank.svg' ]
    e = FoldableBox()
    e.affect( args, False )
    self.assertEqual( e.box.tag, 'g', 'The box group must be created.' )
    self.assertEqual( len( e.box.getchildren() ), 13, 'The box group must have 13 childs.' )

if __name__ == '__main__':
  unittest.main()

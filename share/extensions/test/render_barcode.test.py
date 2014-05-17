#!/usr/bin/env python
#
# Copyright (C) 2010 Martin Owens
#
# Written to test the coding of generating barcodes.
#

import sys
import random
import unittest

# Allow import of the extension code and modules
sys.path.append('..')

from render_barcode import *


digits = [ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' ]

class InsertBarcodeBasicTest(unittest.TestCase):
  """Render Barcode"""
  def setUp(self):
    self.data = {}
    fhl = open('render_barcode.data', 'r')
    for line in fhl:
      line = line.replace('\n', '').replace('\r', '')
      (btype, text, code) = line.split(':')
      if not self.data.has_key(btype):
        self.data[btype] = []
      self.data[btype].append( [ text, code ] )
    fhl.close()

  #def test_run_without_parameters(self):
  #  args = [ 'minimal-blank.svg', '-t', 'Ean5' ]
  #  e = InsertBarcode()
  #  e.affect( args, False )
    #self.assertEqual( e.something, 'some value', 'A commentary about that.' )

  def test_render_barcode_ian5(self):
    """Barcode IAN5"""
    self.barcode_test( 'Ean5' )

  def test_render_barcode_ian8(self):
    """Barcode IAN5"""
    self.barcode_test( 'Ean8' )

  def test_render_barcode_ian13(self):
    """Barcode IAN5"""
    self.barcode_test( 'Ean13' )

  def test_render_barcode_upca(self):
    """Barcode IAN5"""
    self.barcode_test( 'Upca' )

  def test_render_barcode_upce(self):
    """Barcode UPCE"""
    self.barcode_test( 'Upce' )

  def barcode_test(self, name):
    """Base module for all barcode testing"""
    for datum in self.data[name.lower()]:
      (text, code) = datum
      if not text or not code:
        continue
      coder = getBarcode( name, text=text)
      code2 = coder.encode( text )
      self.assertEqual(code, code2)


if __name__ == '__main__':
  unittest.main()


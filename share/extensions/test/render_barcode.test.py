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

import Barcode.EAN5
import Barcode.EAN8
import Barcode.EAN13
import Barcode.UPCA
import Barcode.UPCE

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

  def test_run_without_parameters(self):
    args = [ 'minimal-blank.svg' ]
    e = InsertBarcode()
    e.affect( args, False )
    #self.assertEqual( e.something, 'some value', 'A commentary about that.' )

  def test_render_barcode_ian5(self):
    """Barcode IAN5"""
    self.barcode_test( 'ean5', Barcode.EAN5 )

  def test_render_barcode_ian8(self):
    """Barcode IAN5"""
    self.barcode_test( 'ean8', Barcode.EAN8 )

  def test_render_barcode_ian13(self):
    """Barcode IAN5"""
    self.barcode_test( 'ean13', Barcode.EAN13 )

  def test_render_barcode_upca(self):
    """Barcode IAN5"""
    self.barcode_test( 'upca', Barcode.UPCA )

  def test_render_barcode_upce(self):
    """Barcode UPCE"""
    self.barcode_test( 'upce', Barcode.UPCE )

  def barcode_test(self, name, module):
    """Base module for all barcode testing"""
    for datum in self.data[name]:
      (text, code) = datum
      if not text or not code:
        continue
      code2 = module.Object( {'text': text} ).encode(text)
      self.assertEqual(code, code2)


if __name__ == '__main__':
  unittest.main()


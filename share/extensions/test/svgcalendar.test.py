#!/usr/bin/env python

import sys
sys.path.append('..') # this line allows to import the extension code

import unittest, calendar
from svgcalendar import *

class CalendarArguments(unittest.TestCase):

  #def setUp(self):

  def test_default_names_list(self):
    args = [ 'minimal-blank.svg' ]
    e = SVGCalendar()
    e.affect( args, False )
    self.assertEqual( e.options.month_names[0], 'January' )
    self.assertEqual( e.options.month_names[11], 'December' )
    self.assertEqual( e.options.day_names[0], 'Sun' )
    self.assertEqual( e.options.day_names[6], 'Sat' )

  def test_modifyed_names_list(self):
    args = [
      '--month-names=JAN FEV MAR ABR MAI JUN JUL AGO SET OUT NOV DEZ',
      '--day-names=DOM SEG TER QUA QUI SEX SAB',
      'minimal-blank.svg'
      ]
    e = SVGCalendar()
    e.affect( args, False )
    self.assertEqual( e.options.month_names[0], 'JAN' )
    self.assertEqual( e.options.month_names[11], 'DEZ' )
    self.assertEqual( e.options.day_names[0], 'DOM' )
    self.assertEqual( e.options.day_names[6], 'SAB' )

  def test_starting_or_ending_spaces_must_not_affect_names_list(self):
    args = [
      '--month-names= JAN FEV MAR ABR MAI JUN JUL AGO SET OUT NOV DEZ ',
      '--day-names=  DOM SEG TER QUA QUI SEX SAB  ',
      'minimal-blank.svg'
      ]
    e = SVGCalendar()
    e.affect( args, False )
    self.assertEqual( e.options.month_names[0], 'JAN' )
    self.assertEqual( e.options.month_names[11], 'DEZ' )
    self.assertEqual( e.options.day_names[0], 'DOM' )
    self.assertEqual( e.options.day_names[6], 'SAB' )

  def test_inner_extra_spaces_must_not_affect_names_list(self):
    args = [
      '--month-names=JAN FEV    MAR ABR MAI JUN JUL AGO SET OUT NOV DEZ',
      '--day-names=DOM SEG    TER QUA QUI SEX SAB',
      'minimal-blank.svg'
      ]
    e = SVGCalendar()
    e.affect( args, False )
    self.assertEqual( e.options.month_names[0], 'JAN' )
    self.assertEqual( e.options.month_names[2], 'MAR' )
    self.assertEqual( e.options.month_names[11], 'DEZ' )
    self.assertEqual( e.options.day_names[0], 'DOM' )
    self.assertEqual( e.options.day_names[2], 'TER' )
    self.assertEqual( e.options.day_names[6], 'SAB' )

  def test_default_year_must_be_the_current_year(self):
    args = [ 'minimal-blank.svg' ]
    e = SVGCalendar()
    e.affect( args, False )
    self.assertEqual( e.options.year, datetime.today().year )

  def test_option_year_equal_0_is_converted_to_current_year(self):
    args = [ '--year=0', 'minimal-blank.svg' ]
    e = SVGCalendar()
    e.affect( args, False )
    self.assertEqual( e.options.year, datetime.today().year )

  def test_option_year_2000_configuration(self):
    args = [ '--year=2000', 'minimal-blank.svg' ]
    e = SVGCalendar()
    e.affect( args, False )
    self.assertEqual( e.options.year, 2000 )

  def test_default_week_start_day(self):
    args = [ 'minimal-blank.svg' ]
    e = SVGCalendar()
    e.affect( args, False )
    self.assertEqual( calendar.firstweekday(), 6 )

  def test_configuring_week_start_day(self):
    args = [ '--start-day=sun', 'minimal-blank.svg' ]
    e = SVGCalendar()
    e.affect( args, False )
    self.assertEqual( calendar.firstweekday(), 6 )
    args = [ '--start-day=mon', 'minimal-blank.svg' ]
    e = SVGCalendar()
    e.affect( args, False )
    self.assertEqual( calendar.firstweekday(), 0 )

class CalendarMethods(unittest.TestCase):

  def test_recognize_a_weekend(self):
    args = [ '--start-day=sun', '--weekend=sat+sun', 'minimal-blank.svg' ]
    e = SVGCalendar()
    e.affect( args, False )
    self.assertTrue(  e.is_weekend(0), 'Sunday is weekend in this configuration' )
    self.assertTrue(  e.is_weekend(6), 'Saturday is weekend in this configuration' )
    self.assertFalse( e.is_weekend(1), 'Monday is NOT weekend' )

if __name__ == '__main__':
  unittest.main()


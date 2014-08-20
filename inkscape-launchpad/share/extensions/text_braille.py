#!/usr/bin/env python
# -*- coding: utf-8 -*-

import chardataeffect, inkex, string

convert_table = {\
'a': unicode("⠁", "utf-8"),\
'b': unicode("⠃", "utf-8"),\
'c': unicode("⠉", "utf-8"),\
'd': unicode("⠙", "utf-8"),\
'e': unicode("⠑", "utf-8"),\
'f': unicode("⠋", "utf-8"),\
'g': unicode("⠛", "utf-8"),\
'h': unicode("⠓", "utf-8"),\
'i': unicode("⠊", "utf-8"),\
'j': unicode("⠚", "utf-8"),\
'k': unicode("⠅", "utf-8"),\
'l': unicode("⠇", "utf-8"),\
'm': unicode("⠍", "utf-8"),\
'n': unicode("⠝", "utf-8"),\
'o': unicode("⠕", "utf-8"),\
'p': unicode("⠏", "utf-8"),\
'q': unicode("⠟", "utf-8"),\
'r': unicode("⠗", "utf-8"),\
's': unicode("⠎", "utf-8"),\
't': unicode("⠞", "utf-8"),\
'u': unicode("⠥", "utf-8"),\
'v': unicode("⠧", "utf-8"),\
'w': unicode("⠺", "utf-8"),\
'x': unicode("⠭", "utf-8"),\
'y': unicode("⠽", "utf-8"),\
'z': unicode("⠵", "utf-8"),\
}

class C(chardataeffect.CharDataEffect):

  def process_chardata(self,text, line, par):
    r = ""
    for c in text:
      if convert_table.has_key(c.lower()):
        r = r + convert_table[c.lower()]
      else:
        r = r + c
    return r

c = C()
c.affect()

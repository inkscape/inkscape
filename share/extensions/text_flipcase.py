#!/usr/bin/env python
import chardataeffect, inkex, string

class C(chardataeffect.CharDataEffect):

  def process_chardata(self,text, line, par):
    r = ""
    for i in range(len(text)):
      c = text[i]
      if c.islower():
        r = r + c.upper()
      elif c.isupper():
        r = r + c.lower()
      else:
        r = r + c

    return r

c = C()
c.affect()

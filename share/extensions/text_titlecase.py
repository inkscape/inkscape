#!/usr/bin/env python
import chardataeffect, inkex, string

class C(chardataeffect.CharDataEffect):

  word_ended = True

  def process_chardata(self,text, line, par):
    r = ""
    for i in range(len(text)):
      c = text[i]
      if c.isspace() or line == True or par == True:
        self.word_ended = True
      if not c.isspace():
        line = False
        par = False

      if self.word_ended and c.isalpha():
        r = r + c.upper()
        self.word_ended = False
      elif c.isalpha():
        r = r + c.lower()
      else:
        r = r + c

    return r

c = C()
c.affect()

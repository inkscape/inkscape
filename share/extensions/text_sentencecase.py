#!/usr/bin/env python
import chardataeffect, inkex, string

class C(chardataeffect.CharDataEffect):

  sentence_start = True
  was_punctuation = False

  def process_chardata(self,text, line, par):
    r = ""
    #inkex.debug(text+str(line)+str(par))
    for c in text:
      if c == '.' or c == '!' or c == '?':
        self.was_punctuation = True
      elif ((c.isspace() or line == True) and self.was_punctuation) or par == True:
        self.sentence_start = True
        self.was_punctuation = False
      elif c == '"' or c == ')':
        pass
      else:
        self.was_punctuation = False

      if not c.isspace():
        line = False
        par = False

      if self.sentence_start and c.isalpha():
        r = r + c.upper()
        self.sentence_start = False
      elif not self.sentence_start and c.isalpha():
        r = r + c.lower()
      else:
        r = r + c

    return r

c = C()
c.affect()

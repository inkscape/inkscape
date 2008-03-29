import chardataeffect, inkex, string

class C(chardataeffect.CharDataEffect):
  def __init__(self):
    chardataeffect.CharDataEffect.__init__(self)
    self.OptionParser.add_option("-f", "--from_text", action="store", type="string", dest="from_text", default="", help="Replace")
    self.OptionParser.add_option("-t", "--to_text", action="store", type="string", dest="to_text", default="", help="by")
	      
  def process_chardata(self,text, line, par):
    fr = self.options.from_text.strip('"').replace('\$','$')
    to = self.options.to_text.strip('"').replace('\$','$')

    return (text.replace(unicode(fr,"utf-8"), unicode(to,"utf-8")))

c = C()
c.affect()

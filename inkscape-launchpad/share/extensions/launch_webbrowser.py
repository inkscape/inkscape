#!/usr/bin/env python
# standard library
import webbrowser
import threading
from optparse import OptionParser
# local library
import inkex

class VisitWebSiteWithoutLockingInkscape(threading.Thread):
    def __init__(self):
        threading.Thread.__init__ (self)
        parser = OptionParser()
        parser.add_option("-u", "--url", action="store", type="string",
                          default="http://www.inkscape.org/",
                          dest="url", help="The URL to open in web browser")
        (self.options, args) = parser.parse_args()

    def run(self):
        inkex.localize()
        webbrowser.open(_(self.options.url))

vwswli = VisitWebSiteWithoutLockingInkscape()
vwswli.start()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99

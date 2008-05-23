#!/usr/bin/env python
import webbrowser, threading
from optparse import OptionParser

class VisitWebSiteWithoutLockingInkscape(threading.Thread):
    def __init__(self):
        threading.Thread.__init__ (self)
        parser = OptionParser()
        parser.add_option("-u", "--url", action="store", type="string",
                          default="http://www.inkscape.org/",
                          dest="url", help="The URL to open in web browser")
        (self.options, args) = parser.parse_args()

    def run(self):
        webbrowser.open(self.options.url)

vwswli = VisitWebSiteWithoutLockingInkscape()
vwswli.start()


# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 encoding=utf-8 textwidth=99

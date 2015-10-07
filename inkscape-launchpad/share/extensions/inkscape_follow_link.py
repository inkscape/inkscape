#!/usr/bin/env python

import threading
import webbrowser

import inkex

class VisitWebSiteWithoutLockingInkscape(threading.Thread):
    def __init__(self, url):
        threading.Thread.__init__ (self)
        self.url = url

    def run(self):
        webbrowser.open(self.url)

class FollowLink(inkex.Effect):
    def __init__(self):
        inkex.Effect.__init__(self)

    def effect(self):
        if (self.options.ids):
            for id, node in self.selected.iteritems():
                if node.tag == inkex.addNS('a','svg'):
                    self.url = node.get(inkex.addNS('href','xlink'))
                    vwswli = VisitWebSiteWithoutLockingInkscape(self.url)
                    vwswli.start()
                    #inkex.errormsg("Link: %s" % self.url)
                    break


if __name__ == '__main__':
    e = FollowLink()
    e.affect(output=False)

# vim: expandtab shiftwidth=4 tabstop=8 softtabstop=4 fileencoding=utf-8 textwidth=99

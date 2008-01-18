#!/usr/bin/env python 
import webbrowser, threading
url = "http://answers.launchpad.net/inkscape/+addquestion"

class VisitWebSiteWithoutLockingInkscape(threading.Thread):
    def __init__(self, url):
        self.url = url
        threading.Thread.__init__ (self)

    def run(self):       
        webbrowser.open(self.url)
        
vwswli = VisitWebSiteWithoutLockingInkscape(url)
vwswli.start()

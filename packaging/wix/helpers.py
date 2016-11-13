#!/usr/bin/python

from __future__ import print_function

import os
import re
import sys


# check where to look for the Inkscape files to bundle
#    - btool builds used [root]/inkscape
#    - cmake builds use [root]/build/inkscape unless "DESTDIR" is specified
def get_inkscape_dist_dir():
    # fisrt check the environment variable
    sourcedir = os.getenv('INKSCAPE_DIST_PATH')
    if sourcedir is None:
        sourcedir = ''
    if not os.path.isdir(sourcedir):
        sourcedir = '..\\..\\inkscape'
    if not os.path.isdir(sourcedir):
        sourcedir = '..\\..\\build\\inkscape'
    if not os.path.isdir(sourcedir):
        print("could not find inkscape distribution files, please set environment variable 'INKSCAPE_DIST_PATH'")
        sys.exit(1)
    return sourcedir


# get the full list of available Inkscape UI translations (by looking at available .po files in the /po directory)
def get_inkscape_locales():
    files = os.listdir('..\\..\\po')
    po_files = [file for file in files if file.endswith('.po')]
    locales = [po_file[0:-3] for po_file in po_files if po_file.endswith('.po')]

    return locales


# get the list of available Inkscape UI translations (by parsing inkscape-preferences.cpp)
def get_inkscape_locales_and_names():
    re_languages = re.compile(r'Glib::ustring languages\[\] = \{(.+?)\};', re.DOTALL)
    re_langValues = re.compile(r'Glib::ustring langValues\[\] = \{(.+?)\};', re.DOTALL)
    re_quotes = re.compile(r'"(.*?)"')

    # get the raw array contents from inkscape-preferences.cpp
    with open('..\\..\\src\\ui\\dialog\\inkscape-preferences.cpp', 'r') as f:
        languages = re.search(re_languages, f.read())
        f.seek(0)
        langValues = re.search(re_langValues, f.read())

    # split array elements and extract strings
    if languages and langValues:
        languages = languages.group(1).split(',')
        langValues = langValues.group(1).split(',')
        languages = [re.search(re_quotes, language).group(1) for language in languages]
        langValues = [re.search(re_quotes, langValue).group(1) for langValue in langValues]

    # return the results as a dict (remove first element which is "System default")
    if languages and langValues:
        return dict(zip(langValues[1:], languages[1:]))
    else:
        print("Could not get the list of Inkscape translations from inkscape-preferences.cpp")
        sys.exit(1)

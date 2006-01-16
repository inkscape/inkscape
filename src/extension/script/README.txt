SWIG Scripting Notes
====================
by Ishmal

The code in this directory is an initial start
at providing application-level scripting to Inkscape
via SWIG and interpreter embedding.  Please do not
modify these files until you have become well
acquainted with SWIG and the various methods of
embedding scripting languages in a C/C++ program.


The classes defined in InkscapeBinding.h and
implemented in InkscapeBinding.cpp are destined to
be a thin shell for scripting Inkscape.  Since
Inkscape currently is not organized in a heirarchical
tree, nor is it threadsafe,  this binding tree will merely
mimic such an arrangement.

Note that this -NOT- the same as ECMAScript binding on an
SVG page.  That is another task, coupled with XPath.

Currently, the way to update InkscapeBinding is to:

1.  Modify InkscapeBinding.h and InkscapeBinding.cpp
2.  Run 'make -f Makefile.tmp wraps'
3.  cd to the src or toplevel directory, and build
4.  when this works well, commit the files in this directory

#### SWIG is available here:

http://www.swig.org

#### Information on embedding Python is here

http://docs.python.org/ext/ext.html


#### Information on embedding PERL is available here:

http://perldoc.com/perl5.8.4/pod/perlembed.html

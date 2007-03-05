README for Inkscape's JS Embedding

Note that we edited jstypes.h to
#if defined(_WIN32) && !defined(__MWERKS__) && !defined(__GNUC__)

So that we can statically link JS to Inkscape on MinGW.


bob
6 Mar 07

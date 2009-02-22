##############################################
#
# Test makefile for InkscapeStreams
#
##############################################


CC  = gcc
CXX = g++


INC = -I. -I..

XSLT_CFLAGS = `pkg-config --cflags libxslt`
XSLT_LIBS   = `pkg-config --libs libxslt`

GLIBMM_CFLAGS = `pkg-config --cflags glibmm-2.4`
GLIBMM_LIBS   = `pkg-config --libs glibmm-2.4`

CFLAGS = -g $(GLIBMM_CFLAGS) $(XSLT_CFLAGS)
LIBS   = $(GLIBMM_LIBS) $(XSLT_LIBS) ../uri.o -lz

OBJ = \
inkscapestream.o \
base64stream.o \
gzipstream.o \
stringstream.o \
uristream.o \
xsltstream.o \
ftos.o

all: streamtest

streamtest: inkscapestream.h libstream.a streamtest.o 
	$(CXX) -o streamtest streamtest.o libstream.a $(LIBS)

libstream.a: $(OBJ)
	ar crv libstream.a $(OBJ)


.cpp.o:
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	-$(RM) *.o *.a
	-$(RM) streamtest


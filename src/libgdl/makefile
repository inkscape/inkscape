# Convenience stub makefile to call the real Makefile.



# Explicit so that it's the default rule.
all:
	cd .. && $(MAKE) libgdl/all

clean %.a %.o:
	cd .. && $(MAKE) libgdl/$@

.PHONY: all clean

OBJEXT = o

.SUFFIXES:
.SUFFIXES: .a .$(OBJEXT)

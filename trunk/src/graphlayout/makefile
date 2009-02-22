# Convenience stub makefile to call the real Makefile.



# Explicit so that it's the default rule.
all:
	cd .. && $(MAKE) graphlayout/all

clean %.a %.o:
	cd .. && $(MAKE) graphlayout/$@

.PHONY: all clean

OBJEXT = o

.SUFFIXES:
.SUFFIXES: .a .$(OBJEXT)

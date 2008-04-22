#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_ZEBRA_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_ZEBRA_H__
/* Change the 'ZEBRA' above to be your file name */

/*
 * Copyright (C) 2008 Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
/* ^^^ Change the copyright to be you and your e-mail address ^^^ */

#include "filter.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Filter {

class Zebra : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("zebra",     /* ID -- should be unique */
		            N_("Zebra Stripes"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Paint your object with zebra stripes"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feColorMatrix type=\"matrix\" values=\"0.15 0.3 0.05 0 0.5 0.15 0.3 0.05 0 0.5 0.15 0.3 0.05 0 0.5 0 0 0 1 0\" result=\"result0\"/>\n"
                        "<feTurbulence numOctaves=\"1\" baseFrequency=\"0.078388278388278387 0.012454212454212455\"/>\n"
                        "<feColorMatrix values=\"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 5 -0.8\"/>\n"
                        "<feComposite in2=\"result0\" operator=\"atop\"/>\n"
					"</filter>\n");
						 /* The XML of the filter that should be added.  There
						  * should be a <svg:filter> surrounding what you'd like
						  * to be added with this effect. */
	};
};

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'ZEBRA' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_ZEBRA_H__ */

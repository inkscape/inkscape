#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_CLOUDS_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_CLOUDS_H__
/* Change the 'CLOUDS' above to be your file name */

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

class Clouds : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("clouds",     /* ID -- should be unique */
		            N_("Clouds"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Yes, more descriptions"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feTurbulence type=\"fractalNoise\" baseFrequency=\"0.040293040293040296 0.10695970695970697\" numOctaves=\"3\"/>\n"
                        "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 5 -2.7\" result=\"result0\"/>\n"
                        "<feFlood flood-color=\"rgb(255,255,255)\" flood-opacity=\"1\"/>\n"
                        "<feComposite in2=\"result0\" operator=\"in\"/>\n"
                        "<feComposite in2=\"SourceGraphic\" operator=\"atop\"/>\n"
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

/* Change the 'CLOUDS' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_CLOUDS_H__ */

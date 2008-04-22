#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_SPECKLE_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_SPECKLE_H__
/* Change the 'SPECKLE' above to be your file name */

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

class Speckle : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("speckle",     /* ID -- should be unique */
		            N_("Speckle"), /* Name in the menus, should have a N_() around it for translation */
		            N_("You look cute with speckles"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                       "<feTurbulence type=\"fractalNoise\" baseFrequency=\"0.20952380952380956\" numOctaves=\"1\" result=\"result1\"/>\n"
                       "<feColorMatrix type=\"luminanceToAlpha\" in=\"SourceGraphic\" result=\"result0\"/>\n"
                       "<feColorMatrix result=\"result2\" values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 0.8 0\"/>\n"
                       "<feComposite in2=\"result2\" operator=\"over\" in=\"result1\"/>\n"
                       "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 20 -14\"/>\n"
                       "<feComposite in=\"SourceGraphic\" operator=\"out\"/>\n"
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

/* Change the 'SPECKLE' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_SPECKLE_H__ */

#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_OIL_SLICK_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_OIL_SLICK_H__
/* Change the 'OIL_SLICK' above to be your file name */

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

class OilSlick : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("oil-slick",     /* ID -- should be unique */
		            N_("OilSlick"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Ooops!  Slippery!"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feTurbulence type=\"fractalNoise\" numOctaves=\"5\" baseFrequency=\"0.14299516908212559\"/>\n"
                        "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 5 -3\"/>\n"
                        "<feMorphology operator=\"dilate\" radius=\"2.6570048309178742\" result=\"result3\"/>\n"
                        "<feTurbulence numOctaves=\"2\" baseFrequency=\"0.12077294685990339\"/>\n"
                        "<feDisplacementMap in=\"result3\" xChannelSelector=\"R\" yChannelSelector=\"A\" scale=\"18.526785714285715\" result=\"result4\"/>\n"
                        "<feComposite in=\"result4\" operator=\"atop\" in2=\"SourceGraphic\"/>\n"
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

/* Change the 'OIL_SLICK' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_OIL_SLICK_H__ */

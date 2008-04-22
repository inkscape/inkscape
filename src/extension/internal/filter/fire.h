#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_FIRE_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_FIRE_H__
/* Change the 'FIRE' above to be your file name */

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

class Fire : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("fire",     /* ID -- should be unique */
		            N_("Fire"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Artist on fire"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter x=\"-0.080000000000000029\" height=\"1.3900000000000001\" width=\"1.21\" y=\"-0.22000000000000008\">\n"
                        "<feMorphology operator=\"dilate\" radius=\"2.4128686327077746\" result=\"result1\"/>\n"
                        "<feTurbulence numOctaves=\"1\" baseFrequency=\"0.0900804289544236 0.027882037533512066\"/>\n"
                        "<feColorMatrix values=\"2 0 0 0 0 0 2 0 0 0 0 0 0 0 0 0 0 0 1 0\" result=\"result2\" type=\"matrix\"/>\n"
                        "<feDisplacementMap in=\"result1\" xChannelSelector=\"R\" yChannelSelector=\"G\" scale=\"10.319410319410318\" result=\"result4\"/>\n"
                        "<feFlood flood-color=\"rgb(255,159,54)\" flood-opacity=\"1\" result=\"result3\"/>\n"
                        "<feMorphology in=\"result4\" result=\"result7\" radius=\"3.755868544600939\"/>\n"
                        "<feGaussianBlur stdDeviation=\"2.3571830985915487\" in=\"result7\" result=\"result7\"/>\n"
                        "<feComposite operator=\"in\" in=\"result3\" in2=\"result4\" result=\"result5\"/>\n"
                        "<feComposite in2=\"result7\" operator=\"out\"/>\n"
                        "<feOffset dx=\"-4.5\" dy=\"-7\" result=\"result6\"/>\n"
                        "<feGaussianBlur stdDeviation=\"4.8352546916890073\" result=\"result7\"/>\n"
                        "<feComposite in2=\"result6\" in=\"SourceGraphic\"/>\n"
                        "<feComposite in2=\"result7\"/>\n"
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

/* Change the 'FIRE' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_FIRE_H__ */

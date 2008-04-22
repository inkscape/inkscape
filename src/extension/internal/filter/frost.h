#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_FROST_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_FROST_H__
/* Change the 'FROST' above to be your file name */

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

class Frost : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("frost",     /* ID -- should be unique */
		            N_("Frost"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Artist text"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feTurbulence baseFrequency=\"0.14299516908212559\" numOctaves=\"5\" type=\"fractalNoise\" />\n"
                        "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 5 -3\"/>\n"
                        "<feComposite in2=\"SourceAlpha\" operator=\"in\"/>\n"
                        "<feMorphology result=\"result3\" radius=\"2.6570048309178742\" operator=\"dilate\" />\n"
                        "<feTurbulence baseFrequency=\"0.12077294685990339\" numOctaves=\"2\" />\n"
                        "<feDisplacementMap result=\"result4\" scale=\"10.044642857142858\" yChannelSelector=\"A\" xChannelSelector=\"R\" in=\"result3\" />\n"
                        "<feFlood flood-color=\"rgb(255,255,255)\" flood-opacity=\"1\"/>\n"
                        "<feComposite in2=\"result4\" operator=\"in\" result=\"result2\"/>\n"
                        "<feComposite in2=\"SourceGraphic\" operator=\"over\" in=\"result2\" />\n"
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

/* Change the 'FROST' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_FROST_H__ */

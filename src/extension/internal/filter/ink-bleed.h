#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_INK_BLEED_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_INK_BLEED_H__
/* Change the 'INK_BLEED' above to be your file name */

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

class InkBleed : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("ink-bleed",     /* ID -- should be unique */
		            N_("InkBleed"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Artist Text"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feGaussianBlur stdDeviation=\"1.3291292875989447\" in=\"SourceGraphic\" result=\"result1\"/>\n"
                        "<feTurbulence baseFrequency=\"0.033773087071240104\" numOctaves=\"4\" result=\"result0\"/>\n"
                        "<feDisplacementMap in2=\"result0\" in=\"result1\" xChannelSelector=\"R\" yChannelSelector=\"G\" scale=\"19.612590799031477\" result=\"result2\"/>\n"
                        "<feColorMatrix result=\"result3\" values=\"2 0 0 0 0 0 2 0 0 0 0 0 2 0 0 0 0 0 0.7 0\"/>\n"
                        "<feGaussianBlur stdDeviation=\"1.0653034300791555\" in=\"SourceGraphic\" result=\"result4\"/>\n"
                        "<feComposite in2=\"result3\" in=\"result4\"/>\n"
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

/* Change the 'INK_BLEED' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_INK_BLEED_H__ */

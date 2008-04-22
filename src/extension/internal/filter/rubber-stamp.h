#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_RUBBER_STAMP_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_RUBBER_STAMP_H__
/* Change the 'RUBBER_STAMP' above to be your file name */

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

class RubberStamp : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("rubber-stamp",     /* ID -- should be unique */
		            N_("RubberStamp"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Use this to forge your passport"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feTurbulence type=\"fractalNoise\" numOctaves=\"4\" baseFrequency=\"0.064033264033264037\" in=\"SourceGraphic\" result=\"result1\"/>\n"
                        "<feGaussianBlur stdDeviation=\"4.8612668463611852\" in=\"SourceAlpha\"/>\n"
                        "<feComposite in=\"SourceAlpha\" operator=\"out\"/>\n"
                        "<feComposite in2=\"result1\"/>\n"
                        "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 20 -9\" numOctaves=\"3\" result=\"result1\"/>\n"
                        "<feComposite in=\"SourceGraphic\" operator=\"in\" result=\"result1\"/>\n"
                        "<feTurbulence numOctaves=\"1\" baseFrequency=\"0.03231597845601436\"/>\n"
                        "<feDisplacementMap in=\"result1\" xChannelSelector=\"R\" yChannelSelector=\"G\" scale=\"4.0609137055837561\"/>\n"
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

/* Change the 'RUBBER_STAMP' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_RUBBER_STAMP_H__ */

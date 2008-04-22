#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_SNOW_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_SNOW_H__
/* Change the 'SNOW' above to be your file name */

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

class Snow : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("snow",     /* ID -- should be unique */
		            N_("Snow"), /* Name in the menus, should have a N_() around it for translation */
		            N_("When the weather outside is frightening..."),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feConvolveMatrix order=\"3 3\" kernelMatrix=\"1 1 1 0 0 0 -1 -1 -1\" preserveAlpha=\"false\" divisor=\"3\"/>\n"
                        "<feMorphology operator=\"dilate\" radius=\"1 3.2345013477088949\"/>\n"
                        "<feGaussianBlur stdDeviation=\"1.6270889487870621\" result=\"result0\"/>\n"
                        "<feColorMatrix values=\"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 10 0\" result=\"result1\"/>\n"
                        "<feOffset dx=\"0\" dy=\"1\" result=\"result5\"/>\n"
                        "<feDiffuseLighting in=\"result0\" diffuseConstant=\"2.2613065326633168\" surfaceScale=\"1\">\n"
                          "<feDistantLight azimuth=\"225\" elevation=\"32\"/>\n"
                        "</feDiffuseLighting>\n"
                        "<feComposite in2=\"result1\" operator=\"in\" result=\"result2\"/>\n"
                        "<feColorMatrix values=\"0.4 0 0 0 0.6 0 0.4 0 0 0.6 0 0 0 0 1 0 0 0 1 0\" result=\"result4\"/>\n"
                        "<feComposite in2=\"result5\" in=\"result4\"/>\n"
                        "<feComposite in2=\"SourceGraphic\"/>\n"
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

/* Change the 'SNOW' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_SNOW_H__ */

#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_RIDGED_BORDER_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_RIDGED_BORDER_H__
/* Change the 'RIDGED_BORDER' above to be your file name */

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

class RidgedBorder : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("ridged-border",     /* ID -- should be unique */
		            N_("Ridged Border"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Artist text"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feMorphology in=\"SourceAlpha\" radius=\"4.2857142857142856\"/>\n"
                        "<feComposite in=\"SourceGraphic\" operator=\"out\"/>\n"
                        "<feGaussianBlur stdDeviation=\"1.2003571428571427\" result=\"result0\"/>\n"
                        "<feDiffuseLighting diffuseConstant=\"1\">\n"
                          "<feDistantLight azimuth=\"225\" elevation=\"66\"/>\n"
                        "</feDiffuseLighting>\n"
                        "<feBlend blend=\"normal\" in2=\"SourceGraphic\" mode=\"multiply\"/>\n"
                        "<feComposite in2=\"SourceAlpha\" operator=\"in\"/>\n"
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

/* Change the 'RIDGED_BORDER' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_RIDGED_BORDER_H__ */

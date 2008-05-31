#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_MELT_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_MELT_H__
/*
 * Copyright (C) 2008 Authors:
 *   Ted Gould <ted@gould.cx>
 *   Filter designed by Chrisdesign (http://chrisdesign.wordpress.com/filter-effects/)
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "filter.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Filter {

class Melt : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("melt",     /* ID -- should be unique */
		            N_("Melt"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Melt effect"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feGaussianBlur stdDeviation=\"10\" in=\"SourceGraphic\" result=\"result0\" />\n"
                        "<feTurbulence result=\"result1\" numOctaves=\"8\" seed=\"488\" baseFrequency=\"0.012000000000000004\" />\n"
                        "<feComposite in=\"result0\" in2=\"result1\" operator=\"in\" result=\"result2\" />\n"
                        "<feSpecularLighting in=\"result2\" specularExponent=\"100.40000000000001\" specularConstant=\"3.9400000000000004\" result=\"result4\">\n"
                        "<feDistantLight azimuth=\"225\" elevation=\"62\" />\n"
                        "</feSpecularLighting>\n"
                        "<feComposite operator=\"atop\" in=\"result4\" in2=\"result2\" />\n"
                        "<feConvolveMatrix order=\"5 5\" kernelMatrix=\"0 0 0 0 0 0 0 -1 0 0 0 -1 5 -1 0 0 0 -1 0 0 0 0 0 0 1\" />\n"
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

#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_MELT_H__ */

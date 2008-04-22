#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_METAL_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_METAL_H__
/* Change the 'METAL' above to be your file name */

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

class Metal : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("metal",     /* ID -- should be unique */
		            N_("Metal"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Iron Man vector objects"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feColorMatrix result=\"result1\" in=\"SourceGraphic\" type=\"saturate\" values=\"0.369458\"/>\n"
                        "<feGaussianBlur in=\"SourceAlpha\" stdDeviation=\"7.0222857142857134\"/>\n"
                        "<feSpecularLighting surfaceScale=\"10\" specularConstant=\"3.8834951456310676\" specularExponent=\"7.971360381861575\">\n"
                          "<feDistantLight elevation=\"17\" azimuth=\"225\" />\n"
                        "</feSpecularLighting>\n"
                        "<feComposite operator=\"atop\" in2=\"result1\"/>\n"
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

/* Change the 'METAL' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_METAL_H__ */

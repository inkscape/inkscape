#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_BLOOM_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_BLOOM_H__
/* Change the 'BLOOM' above to be your file name */

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

class Bloom : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("bloom",     /* ID -- should be unique */
		            N_("Bloom"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Not sure, nobody tell me these things"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feGaussianBlur stdDeviation=\"2.1526428571428569\" in=\"SourceAlpha\" result=\"result1\"/>\n"
                        "<feSpecularLighting surfaceScale=\"5\" specularConstant=\"2\" specularExponent=\"18.063876651982376\" result=\"result0\">\n"
                          "<feDistantLight elevation=\"24\" azimuth=\"225\"/>\n"
                        "</feSpecularLighting>\n"
                        "<feComposite in2=\"SourceAlpha\" operator=\"in\" result=\"result6\"/>\n"
                        "<feMorphology operator=\"dilate\" radius=\"5.7142857142857144\"/>\n"
                        "<feGaussianBlur stdDeviation=\"5.7237142857142853\" result=\"result11\"/>\n"
                        "<feDiffuseLighting in=\"result1\" diffuseConstant=\"2.0099999999999998\" result=\"result3\" surfaceScale=\"5\">\n"
                          "<feDistantLight azimuth=\"225\" elevation=\"25\"/>\n"
                        "</feDiffuseLighting>\n"
                        "<feBlend blend=\"normal\" in2=\"SourceGraphic\" in=\"result3\" mode=\"multiply\" result=\"result7\"/>\n"
                        "<feComposite in2=\"SourceAlpha\" operator=\"in\" in=\"result7\"/>\n"
                        "<feBlend blend=\"normal\" in=\"result6\" mode=\"lighten\" result=\"result9\"/>\n"
                        "<feComposite in=\"result11\"/>\n"
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

/* Change the 'BLOOM' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_BLOOM_H__ */

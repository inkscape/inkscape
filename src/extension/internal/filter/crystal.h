#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_CRYSTAL_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_CRYSTAL_H__
/* Change the 'CRYSTAL' above to be your file name */

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

class Crystal : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("crystal",     /* ID -- should be unique */
		            N_("Crystal"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Artist, insert data here"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feBlend mode=\"multiply\" in2=\"BackgroundImage\" blend=\"normal\" in=\"SourceGraphic\" result=\"result0\"/>\n"
                        "<feGaussianBlur stdDeviation=\"3.7133333333333329\" in=\"SourceAlpha\" result=\"result1\"/>\n"
                        "<feSpecularLighting specularExponent=\"128\" specularConstant=\"1.7636684303350969\" surfaceScale=\"3\">\n"
                          "<feDistantLight azimuth=\"225\" elevation=\"45\"/>\n"
                        "</feSpecularLighting>\n"
                        "<feComposite in2=\"SourceAlpha\" operator=\"in\" result=\"result3\"/>\n"
                        "<feSpecularLighting in=\"result1\" specularExponent=\"128\" specularConstant=\"3.5273368606701938\" surfaceScale=\"-5\">\n"
                          "<feDistantLight azimuth=\"225\" elevation=\"32\"/>\n"
                        "</feSpecularLighting>\n"
                        "<feComposite in2=\"SourceAlpha\" operator=\"in\" result=\"result2\"/>\n"
                        "<feMerge>\n"
                          "<feMergeNode inkscape:collect=\"always\" in=\"result0\"/>\n"
                          "<feMergeNode inkscape:collect=\"always\" in=\"result3\"/>\n"
                          "<feMergeNode inkscape:collect=\"always\" in=\"result2\"/>\n"
                        "</feMerge>\n"
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

/* Change the 'CRYSTAL' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_CRYSTAL_H__ */

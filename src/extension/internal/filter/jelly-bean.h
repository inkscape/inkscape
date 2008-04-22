#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_JELLY_BEAN_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_JELLY_BEAN_H__
/* Change the 'JELLY_BEAN' above to be your file name */

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

class JellyBean : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("jelly-bean",     /* ID -- should be unique */
		            N_("Jelly Bean"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Mmmm, yummy."),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feColorMatrix in=\"SourceGraphic\" result=\"result0\" values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 0.85 0\"/>\n"
                        "<feGaussianBlur stdDeviation=\"7.0222857142857134\" in=\"SourceAlpha\"/>\n"
                        "<feSpecularLighting specularExponent=\"7.971360381861575\" specularConstant=\"0.80000000000000004\" surfaceScale=\"10\">\n"
                          "<feDistantLight azimuth=\"225\" elevation=\"17\"/>\n"
                        "</feSpecularLighting>\n"
                        "<feComposite in2=\"result0\" operator=\"atop\"/>\n"
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

/* Change the 'JELLY_BEAN' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_JELLY_BEAN_H__ */

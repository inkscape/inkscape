#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_EMBOSS_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_EMBOSS_H__
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

class Emboss : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("emboss",     /* ID -- should be unique */
		            N_("Emboss"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Emboss effect"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feGaussianBlur result=\"result0\" in=\"SourceAlpha\" stdDeviation=\"1.01\" />\n"
                        "<feOffset stdDeviation=\"0.40000000000000002\" result=\"result3\" in=\"result0\" dy=\"2\" dx=\"2\" />\n"
                        "<feSpecularLighting specularExponent=\"35\" specularConstant=\"1.05\" surfaceScale=\"0.75\" lighting-color=\"rgb(217,217,217)\" result=\"result1\" in=\"result0\">\n"
                        "<fePointLight z=\"20000\" y=\"-10000\" x=\"-5000\" />\n"
                        "</feSpecularLighting>\n"
                        "<feComposite operator=\"in\" result=\"result2\" in=\"result1\" in2=\"SourceAlpha\" />\n"
                        "<feComposite k3=\"0.99999999999999989\" k2=\"0.99999999999999989\" operator=\"arithmetic\" result=\"result4\" in=\"SourceGraphic\" />\n"
                        "<feMerge><feMergeNode in=\"result3\" />\n"
                        "<feMergeNode in=\"result4\" />\n"
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

#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_EMBOSS_H__ */

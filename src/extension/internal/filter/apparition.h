#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_APPARITION_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_APPARITION_H__
/* Change the 'APPARITION' above to be your file name */

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

class Apparition : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("apparition",     /* ID -- should be unique */
		            N_("Apparition"), /* Name in the menus, should have a N_() around it for translation */
		            N_("I'm not sure what this word means"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feMorphology result=\"result0\" in=\"SourceGraphic\" radius=\"4\"/>\n"
                        "<feGaussianBlur stdDeviation=\"9.2439050131926113\" in=\"result0\"/>\n"
                        "<feComposite in=\"SourceGraphic\" operator=\"in\"/>\n"
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

/* Change the 'APPARITION' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_APPARITION_H__ */

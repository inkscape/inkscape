#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_CUTOUT_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_CUTOUT_H__
/* Change the 'CUTOUT' above to be your file name */

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

class Cutout : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("cutout",     /* ID -- should be unique */
		            N_("Coutout"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Artist text"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feGaussianBlur stdDeviation=\"4.2312137203166218\" in=\"SourceAlpha\"/>\n"
                        "<feOffset dx=\"5\" dy=\"5\" />\n"
                        "<feComposite in=\"SourceGraphic\" operator=\"out\"/>\n"
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

/* Change the 'CUTOUT' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_CUTOUT_H__ */

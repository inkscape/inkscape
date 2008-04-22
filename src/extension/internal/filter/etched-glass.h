#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_ETCHED_GLASS_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_ETCHED_GLASS_H__
/* Change the 'ETCHED_GLASS' above to be your file name */

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

class EtchedGlass : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("etched-glass",     /* ID -- should be unique */
		            N_("Etched Glass"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Artist text"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feGaussianBlur stdDeviation=\"6.3056666666666654\" in=\"BackgroundImage\"/>\n"
                        "<feComposite in2=\"SourceAlpha\" operator=\"in\"/>\n"
                        "<feBlend blend=\"normal\" in2=\"SourceGraphic\" mode=\"multiply\"/>\n"
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

/* Change the 'ETCHED_GLASS' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_ETCHED_GLASS_H__ */

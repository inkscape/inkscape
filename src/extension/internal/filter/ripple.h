#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_RIPPLE_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_RIPPLE_H__
/* Change the 'RIPPLE' above to be your file name */

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

class Ripple : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("ripple",     /* ID -- should be unique */
		            N_("Ripple"), /* Name in the menus, should have a N_() around it for translation */
		            N_("You're 80% water"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feTurbulence baseFrequency=\"0.001904761904761905 0.10666666666666667\" numOctaves=\"1\"/>\n"
                        "<feColorMatrix values=\"2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0.5 0.5\"/>\n"
                        "<feDisplacementMap in=\"SourceGraphic\" scale=\"14.317180616740089\" xChannelSelector=\"R\" yChannelSelector=\"A\"/>\n"
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

/* Change the 'RIPPLE' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_RIPPLE_H__ */

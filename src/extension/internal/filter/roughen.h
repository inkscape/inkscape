#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_ROUGHEN_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_ROUGHEN_H__
/* Change the 'ROUGHEN' above to be your file name */

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

class Roughen : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("roughen",     /* ID -- should be unique */
		            N_("Roughen"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Like Brad Pitt's stubble"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                       "<feTurbulence baseFrequency=\"0.040000000000000001\" in=\"SourceAlpha\" type=\"turbulence\" seed=\"0\" numOctaves=\"3\"/>\n"
                       "<feDisplacementMap in=\"SourceGraphic\" xChannelSelector=\"R\" yChannelSelector=\"G\" scale=\"6.6339066339066335\" numOctaves=\"2\"/>\n"
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

/* Change the 'ROUGHEN' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_ROUGHEN_H__ */

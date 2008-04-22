#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_SEPIA_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_SEPIA_H__
/* Change the 'SEPIA' above to be your file name */

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

class Sepia : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("sepia",     /* ID -- should be unique */
		            N_("Sepia"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Turn all the colors to be sepia tones"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
					    "<feColorMatrix values=\"0.14 0.45 0.05 0 0 0.12 0.39 0.04 0 0 0.08 0.28 0.03 0 0 0 0 0 1 0\" />\n"
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

/* Change the 'SEPIA' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_SEPIA_H__ */

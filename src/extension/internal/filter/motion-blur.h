#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_MOTION_BLUR_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_MOTION_BLUR_H__
/* Change the 'MOTION_BLUR' above to be your file name */

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

class MotionBlur : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("motion-blur",     /* ID -- should be unique */
		            N_("Motion Blur"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Hmm, fast vectors"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feGaussianBlur stdDeviation=\"5 0.01\" in=\"SourceGraphic\"/>\n"
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

/* Change the 'MOTION_BLUR' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_MOTION_BLUR_H__ */

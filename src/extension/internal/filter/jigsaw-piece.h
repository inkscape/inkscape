#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_JIGSAW_PIECE_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_JIGSAW_PIECE_H__
/* Change the 'JIGSAW_PIECE' above to be your file name */

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

class JigsawPiece : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("jigsaw-piece",     /* ID -- should be unique */
		            N_("JigsawPiece"), /* Name in the menus, should have a N_() around it for translation */
		            N_("It's a puzzle, no hints"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feSpecularLighting specularExponent=\"18.527845036319611\" specularConstant=\"2\" surfaceScale=\"1\" in=\"SourceAlpha\">\n"
                          "<feDistantLight azimuth=\"225\" elevation=\"30\"/>\n"
                        "</feSpecularLighting>\n"
                        "<feComposite in2=\"SourceGraphic\" operator=\"atop\" result=\"result0\"/>\n"
                        "<feMorphology operator=\"dilate\" in=\"SourceAlpha\" result=\"result1\" radius=\"2\"/>\n"
                        "<feComposite in2=\"result1\" in=\"result0\"/>\n"
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

/* Change the 'JIGSAW_PIECE' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_JIGSAW_PIECE_H__ */

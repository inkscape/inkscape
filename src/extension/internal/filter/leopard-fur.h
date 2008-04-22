#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_LEOPARD_FUR_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_LEOPARD_FUR_H__
/* Change the 'LEOPARD_FUR' above to be your file name */

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

class LeopardFur : public Inkscape::Extension::Internal::Filter::Filter {
public:
	static void init (void) {
		filter_init("leopard-fur",     /* ID -- should be unique */
		            N_("Leopard Fur"), /* Name in the menus, should have a N_() around it for translation */
		            N_("Purrrr, quiet the kitty is sleeping"),
					             /* Menu tooltip to help users understand the name.  Should also have a N_() */
					"<filter>\n"
                        "<feTurbulence baseFrequency=\"0.14299516908212559\" numOctaves=\"5\" type=\"fractalNoise\"/>\n"
                        "<feColorMatrix values=\"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 5 -3.45\"/>\n"
                        "<feComposite result=\"result3\" in2=\"SourceAlpha\" operator=\"in\"/>\n"
                        "<feColorMatrix values=\"1 0 0 0 0 0 1 0 0 0 0 0 1 0 0 0 0 0 6 0\"/>\n"
                        "<feMorphology result=\"result3\" radius=\"1.8181818181818181\" operator=\"dilate\"/>\n"
                        "<feGaussianBlur result=\"result3\" stdDeviation=\"1\"/>\n"
                        "<feGaussianBlur stdDeviation=\"2.7287613293051356\"/>\n"
                        "<feComposite in2=\"result3\" result=\"result1\" operator=\"out\"/>\n"
                        "<feColorMatrix result=\"result3\" values=\"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 6 0\"/>\n"
                        "<feFlood result=\"result2\" flood-opacity=\"1\" flood-color=\"rgb(209,151,45)\" />\n"
                        "<feComposite operator=\"in\" in=\"result2\" in2=\"SourceGraphic\" />\n"
                        "<feComposite in=\"result3\" result=\"result3\" operator=\"atop\" />\n"
                        "<feGaussianBlur stdDeviation=\"7.1521428571428558\" in=\"SourceAlpha\"/>\n"
                        "<feDiffuseLighting diffuseConstant=\"1.9197207678883073\" surfaceScale=\"10.600706713780937\">\n"
                          "<feDistantLight azimuth=\"225\" elevation=\"48\"/>\n"
                        "</feDiffuseLighting>\n"
                        "<feBlend blend=\"normal\" in2=\"result3\" mode=\"multiply\" result=\"result3\"/>\n"
                        "<feComposite in2=\"SourceAlpha\" operator=\"in\" result=\"result3\"/>\n"
                        "<feTurbulence baseFrequency=\"0.10577777777777778\" numOctaves=\"3\" />\n"
                        "<feDisplacementMap scale=\"4.5454545454545459\" yChannelSelector=\"G\" xChannelSelector=\"R\" in=\"result3\"/>\n"
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

/* Change the 'LEOPARD_FUR' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_LEOPARD_FUR_H__ */

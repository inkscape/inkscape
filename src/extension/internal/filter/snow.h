#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_SNOW_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_SNOW_H__
/* Change the 'SNOW' above to be your file name */

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

class Snow : public Inkscape::Extension::Internal::Filter::Filter {
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	Snow ( ) : Filter() { };
	virtual ~Snow ( ) { if (_filter != NULL) g_free((void *)_filter); return; }

public:
	static void init (void) {
		Inkscape::Extension::build_from_mem(
			"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
				"<name>" N_("Snow") "</name>\n"
				"<id>org.inkscape.effect.filter.snow</id>\n"
				"<param name=\"drift\" gui-text=\"" N_("Drift Size") "\" type=\"float\" min=\"0.0\" max=\"20.0\">3.5</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filters") "\">\n"
   						"<submenu name=\"Shadows and Glows\"/>\n"
			      "</submenu>\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("Snow has fallen on object") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new Snow());
	};

};

gchar const *
Snow::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);

	float drift = ext->get_param_float("drift");

	_filter = g_strdup_printf(
				"<filter>\n"
					"<feConvolveMatrix order=\"3 3\" kernelMatrix=\"1 1 1 0 0 0 -1 -1 -1\" preserveAlpha=\"false\" divisor=\"3\"/>\n"
					"<feMorphology operator=\"dilate\" radius=\"1 %f\"/>\n"
					"<feGaussianBlur stdDeviation=\"1.6270889487870621\" result=\"result0\"/>\n"
					"<feColorMatrix values=\"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 10 0\" result=\"result1\"/>\n"
					"<feOffset dx=\"0\" dy=\"1\" result=\"result5\"/>\n"
					"<feDiffuseLighting in=\"result0\" diffuseConstant=\"2.2613065326633168\" surfaceScale=\"1\">\n"
					  "<feDistantLight azimuth=\"225\" elevation=\"32\"/>\n"
					"</feDiffuseLighting>\n"
					"<feComposite in2=\"result1\" operator=\"in\" result=\"result2\"/>\n"
					"<feColorMatrix values=\"0.4 0 0 0 0.6 0 0.4 0 0 0.6 0 0 0 0 1 0 0 0 1 0\" result=\"result4\"/>\n"
					"<feComposite in2=\"result5\" in=\"result4\"/>\n"
					"<feComposite in2=\"SourceGraphic\"/>\n"
				"</filter>\n", drift);

	return _filter;
};

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'SNOW' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_SNOW_H__ */

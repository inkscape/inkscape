#ifndef __INKSCAPE_EXTENSION_INTERNAL_FILTER_DROP_SHADOW_H__
#define __INKSCAPE_EXTENSION_INTERNAL_FILTER_DROP_SHADOW_H__
/* Change the 'DROP_SHADOW' above to be your file name */

/*
 * Copyright (C) 2008 Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
/* ^^^ Change the copyright to be you and your e-mail address ^^^ */

#include "filter.h"

#include "extension/internal/clear-n_.h"
#include "extension/system.h"
#include "extension/extension.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Filter {

class DropShadow : public Inkscape::Extension::Internal::Filter::Filter {
	int myvar;
protected:
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

public:
	DropShadow ( ) : Filter() { };
	//virtual ~DropShadow ( ) { if (_filter != NULL) g_free(_filter); return; }

	static void init (void) {
		Inkscape::Extension::build_from_mem(
			"<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
				"<name>" N_("Drop Shadow") "</name>\n"
				"<id>org.inkscape.effect.filter.drop-shadow</id>\n"
				"<param name=\"blur\" gui-text=\"" N_("Amount of Blur") "\" type=\"float\" min=\"0.0\" max=\"50.0\">2.0</param>\n"
				"<param name=\"xoffset\" gui-text=\"" N_("Horizontal Offset") "\" type=\"float\" min=\"-50.0\" max=\"50.0\">4.0</param>\n"
				"<param name=\"yoffset\" gui-text=\"" N_("Vertical Offset") "\" type=\"float\" min=\"-50.0\" max=\"50.0\">4.0</param>\n"
				"<effect>\n"
					"<object-type>all</object-type>\n"
					"<effects-menu>\n"
						"<submenu name=\"" N_("Filter") "\" />\n"
					"</effects-menu>\n"
					"<menu-tip>" N_("I hate text") "</menu-tip>\n"
				"</effect>\n"
			"</inkscape-extension>\n", new DropShadow());
	};

};

gchar const *
DropShadow::get_filter_text (Inkscape::Extension::Extension * ext)
{
	if (_filter != NULL) g_free((void *)_filter);

	float blur = ext->get_param_float("blur");
	float x = ext->get_param_float("xoffset");
	float y = ext->get_param_float("yoffset");

	_filter = g_strdup_printf(
		"<filter>\n"
			"<feGaussianBlur in=\"SourceAlpha\" stdDeviation=\"%f\" result=\"blur\"/>\n"
			"<feOffset in=\"blur\" dx=\"%f\" dy=\"%f\" result=\"offsetBlur\"/>\n"
			"<feMerge>\n"
				"<feMergeNode in=\"offsetBlur\"/>\n"
				"<feMergeNode in=\"SourceGraphic\"/>\n"
			"</feMerge>\n"
		"</filter>\n", blur, x, y);

	return _filter;
};

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/* Change the 'DROP_SHADOW' below to be your file name */
#endif /* __INKSCAPE_EXTENSION_INTERNAL_FILTER_DROP_SHADOW_H__ */

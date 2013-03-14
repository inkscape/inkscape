#ifndef INKSCAPE_EXTENSION_INTERNAL_FILTER_FILTER_H
#define INKSCAPE_EXTENSION_INTERNAL_FILTER_FILTER_H

/*
 * Copyright (C) 2008 Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glibmm/i18n.h>

#include "extension/implementation/implementation.h"

namespace Inkscape {

namespace XML {
	struct Document;
}

namespace Extension {

class Effect;
class Extension;

namespace Internal {
namespace Filter {

class Filter : public Inkscape::Extension::Implementation::Implementation {
protected:
	gchar const * _filter;
	virtual gchar const * get_filter_text (Inkscape::Extension::Extension * ext);

private:
	Inkscape::XML::Document * get_filter (Inkscape::Extension::Extension * ext);
	void merge_filters (Inkscape::XML::Node * to, Inkscape::XML::Node * from, Inkscape::XML::Document * doc, gchar const * srcGraphic = NULL, gchar const * srcGraphicAlpha = NULL);

public:
	Filter();
	Filter(gchar const * filter);
	virtual ~Filter();

	bool load(Inkscape::Extension::Extension *module);
	Inkscape::Extension::Implementation::ImplementationDocumentCache * newDocCache (Inkscape::Extension::Extension * ext, Inkscape::UI::View::View * doc);
	void effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View *document, Inkscape::Extension::Implementation::ImplementationDocumentCache * docCache);

	static void filter_init(gchar const * id, gchar const * name, gchar const * submenu, gchar const * tip, gchar const * filter);
	static void filters_all(void);

	/* File loader related */
	static void filters_all_files(void);
	static void filters_load_dir(gchar const * filename, gchar * menuname);
	static void filters_load_file(gchar * filename, gchar * menuname);
	static void filters_load_node(Inkscape::XML::Node * node, gchar * menuname);

};

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

#endif // INKSCAPE_EXTENSION_INTERNAL_FILTER_FILTER_H

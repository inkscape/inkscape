/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "desktop.h"
#include "selection.h"
#include "document-private.h"
#include "sp-item.h"
#include "util/glib-list-iterators.h"
#include "extension/extension.h"
#include "extension/effect.h"
#include "extension/system.h"
#include "xml/repr.h"
#include "xml/simple-node.h"
#include "xml/attribute-record.h"

#include "filter.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Filter {

Filter::Filter() :
		Inkscape::Extension::Implementation::Implementation(),
		_filter(NULL) {
	return;
}

Filter::Filter(gchar const * filter) :
		Inkscape::Extension::Implementation::Implementation(),
		_filter(filter) {
	return;
}

Filter::~Filter (void) {
	if (_filter != NULL) {
		_filter = NULL;
	}

	return;
}

bool Filter::load(Inkscape::Extension::Extension * /*module*/)
{
    return true;
}

Inkscape::Extension::Implementation::ImplementationDocumentCache *Filter::newDocCache(Inkscape::Extension::Extension * /*ext*/,
										      Inkscape::UI::View::View * /*doc*/)
{
    return NULL;
}

gchar const *Filter::get_filter_text(Inkscape::Extension::Extension * /*ext*/)
{
    return _filter;
}

Inkscape::XML::Document *
Filter::get_filter (Inkscape::Extension::Extension * ext) {
	gchar const * filter = get_filter_text(ext);
	return sp_repr_read_mem(filter, strlen(filter), NULL);
}

void
Filter::merge_filters( Inkscape::XML::Node * to, Inkscape::XML::Node * from,
		       Inkscape::XML::Document * doc,
		       gchar const * srcGraphic, gchar const * srcGraphicAlpha)
{
	if (from == NULL) return;

	// copy attributes
    for ( Inkscape::Util::List<Inkscape::XML::AttributeRecord const> iter = from->attributeList() ;
          iter ; ++iter ) {
		gchar const * attr = g_quark_to_string(iter->key);
		//printf("Attribute List: %s\n", attr);
		if (!strcmp(attr, "id")) continue; // nope, don't copy that one!
		to->setAttribute(attr, from->attribute(attr));

		if (!strcmp(attr, "in") || !strcmp(attr, "in2") || !strcmp(attr, "in3")) {
			if (srcGraphic != NULL && !strcmp(from->attribute(attr), "SourceGraphic")) {
				to->setAttribute(attr, srcGraphic);
			}

			if (srcGraphicAlpha != NULL && !strcmp(from->attribute(attr), "SourceAlpha")) {
				to->setAttribute(attr, srcGraphicAlpha);
			}
		}
	}

	// for each child call recursively
	for (Inkscape::XML::Node * from_child = from->firstChild();
	          from_child != NULL ; from_child = from_child->next()) {
		Glib::ustring name = "svg:";
		name += from_child->name();

		Inkscape::XML::Node * to_child = doc->createElement(name.c_str());
		to->appendChild(to_child);
		merge_filters(to_child, from_child, doc, srcGraphic, srcGraphicAlpha);

		if (from_child == from->firstChild() && !strcmp("filter", from->name()) && srcGraphic != NULL && to_child->attribute("in") == NULL) {
			to_child->setAttribute("in", srcGraphic);
		}
    Inkscape::GC::release(to_child);
	}
}

#define FILTER_SRC_GRAPHIC       "fbSourceGraphic"
#define FILTER_SRC_GRAPHIC_ALPHA "fbSourceGraphicAlpha"

void Filter::effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View *document,
		    Inkscape::Extension::Implementation::ImplementationDocumentCache * /*docCache*/)
{
	Inkscape::XML::Document *filterdoc = get_filter(module);
	if (filterdoc == NULL) {
		return; // could not parse the XML source of the filter; typically parser will stderr a warning
	}

	//printf("Calling filter effect\n");
    Inkscape::Selection * selection = ((SPDesktop *)document)->selection;

    // TODO need to properly refcount the items, at least
    std::vector<SPItem*> items(selection->itemList());

	Inkscape::XML::Document * xmldoc = document->doc()->getReprDoc();
	Inkscape::XML::Node * defsrepr = document->doc()->getDefs()->getRepr();

    for(std::vector<SPItem*>::iterator item = items.begin();
            item != items.end(); ++item) {
        SPItem * spitem = *item;
	Inkscape::XML::Node * node = spitem->getRepr();

		SPCSSAttr * css = sp_repr_css_attr(node, "style");
		gchar const * filter = sp_repr_css_property(css, "filter", NULL);

		if (filter == NULL) {

			Inkscape::XML::Node * newfilterroot = xmldoc->createElement("svg:filter");
			merge_filters(newfilterroot, filterdoc->root(), xmldoc);
			defsrepr->appendChild(newfilterroot);

			Glib::ustring url = "url(#"; url += newfilterroot->attribute("id"); url += ")";


      Inkscape::GC::release(newfilterroot);

			sp_repr_css_set_property(css, "filter", url.c_str());
			sp_repr_css_set(node, css, "style");
		} else {
			if (strncmp(filter, "url(#", strlen("url(#")) || filter[strlen(filter) - 1] != ')') {
				// This is not url(#id) -- we can't handle it
				continue;
			}

			gchar * lfilter = g_strndup(filter + 5, strlen(filter) - 6);
			Inkscape::XML::Node * filternode = NULL;
			for (Inkscape::XML::Node * child = defsrepr->firstChild(); child != NULL; child = child->next()) {
				if (!strcmp(lfilter, child->attribute("id"))) {
					filternode = child;
					break;
				}
			}
			g_free(lfilter);

			// no filter
			if (filternode == NULL) {
				g_warning("no assigned filter found!");
				continue;
			}

			if (filternode->lastChild() == NULL) {
                // empty filter, we insert
                merge_filters(filternode, filterdoc->root(), xmldoc);
			} else {
                // existing filter, we merge
                filternode->lastChild()->setAttribute("result", FILTER_SRC_GRAPHIC);
                Inkscape::XML::Node * alpha = xmldoc->createElement("svg:feColorMatrix");
                alpha->setAttribute("result", FILTER_SRC_GRAPHIC_ALPHA);
                alpha->setAttribute("in", FILTER_SRC_GRAPHIC); // not required, but we're being explicit
                alpha->setAttribute("values", "0 0 0 -1 0 0 0 0 -1 0 0 0 0 -1 0 0 0 0 1 0");

                filternode->appendChild(alpha);

                merge_filters(filternode, filterdoc->root(), xmldoc, FILTER_SRC_GRAPHIC, FILTER_SRC_GRAPHIC_ALPHA);

                Inkscape::GC::release(alpha);
			}
		}
    }

    return;
}

#include "extension/internal/clear-n_.h"

void
Filter::filter_init (gchar const * id, gchar const * name, gchar const * submenu, gchar const * tip, gchar const * filter)
{
	gchar * xml_str = g_strdup_printf(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>%s</name>\n"
            "<id>org.inkscape.effect.filter.%s</id>\n"
            "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                    "<submenu name=\"" N_("Filters") "\" />\n"
         						"<submenu name=\"%s\"/>\n"
                "</effects-menu>\n"
                "<menu-tip>%s</menu-tip>\n"
            "</effect>\n"
        "</inkscape-extension>\n", name, id, submenu, tip);
    Inkscape::Extension::build_from_mem(xml_str, new Filter(filter));
	g_free(xml_str);
    return;
}

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */


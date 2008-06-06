/*
 * Copyright (C) 2008 Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "filter.h"

#include "io/sys.h"
#include "io/inkscapestream.h"

/* Directory includes */
#include "path-prefix.h"
#include "inkscape.h"

/* Extension */
#include "extension/extension.h"
#include "extension/system.h"

/* System includes */
#include <glibmm/i18n.h>

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Filter {

void
Filter::filters_all_files (void)
{
	filters_load_dir(INKSCAPE_FILTERDIR, _("Bundled"));
	filters_load_dir(profile_path("filters"), _("Personal"));

	return;
}

#define INKSCAPE_FILTER_FILE  ".svg"

void
Filter::filters_load_dir (gchar const * dirname, gchar * menuname)
{
    if (!dirname) {
        g_warning(_("Null external module directory name.  Filters will not be loaded."));
        return;
    }

    if (!Glib::file_test(std::string(dirname), Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_DIR)) {
        return;
    }

    GError *err;
    GDir *directory = g_dir_open(dirname, 0, &err);
    if (!directory) {
        gchar *safeDir = Inkscape::IO::sanitizeString(dirname);
        g_warning(_("Modules directory (%s) is unavailable.  External modules in that directory will not be loaded."), safeDir);
        g_free(safeDir);
        return;
    }

    gchar *filename;
    while ((filename = (gchar *)g_dir_read_name(directory)) != NULL) {
        if (strlen(filename) < strlen(INKSCAPE_FILTER_FILE)) {
            continue;
        }

        if (strcmp(INKSCAPE_FILTER_FILE, filename + (strlen(filename) - strlen(INKSCAPE_FILTER_FILE)))) {
            continue;
        }

        gchar *pathname = g_build_filename(dirname, filename, NULL);
        filters_load_file(pathname, menuname);
        g_free(pathname);
    }

    g_dir_close(directory);

	return;
}

void
Filter::filters_load_file (gchar * filename, gchar * menuname)
{
    Inkscape::XML::Document *doc = sp_repr_read_file(filename, INKSCAPE_EXTENSION_URI);
	if (doc == NULL) {
		g_warning("File (%s) is not parseable as XML.  Ignored.", filename);
		return;
	}

	Inkscape::XML::Node * root = doc->root();
	if (strcmp(root->name(), "svg:svg")) {
		Inkscape::GC::release(doc);
		g_warning("File (%s) is not SVG.  Ignored.", filename);
		return;
	}

	for (Inkscape::XML::Node * child = root->firstChild();
			child != NULL; child = child->next()) {
		if (!strcmp(child->name(), "svg:defs")) {
			for (Inkscape::XML::Node * defs = child->firstChild();
					defs != NULL; defs = defs->next()) {
				if (!strcmp(defs->name(), "svg:filter")) {
					filters_load_node(defs, menuname);
				} // oh!  a filter
			} //defs
		} // is defs
	} // children of root

	Inkscape::GC::release(doc);
	return;
}

#include "extension/internal/clear-n_.h"

class mywriter : public Inkscape::IO::BasicWriter {
	Glib::ustring _str;
public:
	void close(void);
	void flush(void);
	void put (gunichar ch);
	gchar const * c_str (void) { return _str.c_str(); }
};

void mywriter::close (void) { return; }
void mywriter::flush (void) { return; }
void mywriter::put (gunichar ch) { _str += ch; }


void
Filter::filters_load_node (Inkscape::XML::Node * node, gchar * menuname)
{
	gchar const * label = node->attribute("inkscape:label");
	gchar const * id = node->attribute("id");

	if (label == NULL) {
		label = id;
	}

	gchar * xml_str = g_strdup_printf(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>%s</name>\n"
            "<id>org.inkscape.effect.filter.%s</id>\n"
            "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                    "<submenu name=\"" N_("Filter") "\">\n"
						"<submenu name=\"%s\"/>\n"
					"</submenu>\n"
                "</effects-menu>\n"
            "</effect>\n"
        "</inkscape-extension>\n", label, id, menuname);

	mywriter writer;
	sp_repr_write_stream(node, writer, 0, FALSE, g_quark_from_static_string("svg"), 0, 0);

    Inkscape::Extension::build_from_mem(xml_str, new Filter::Filter(g_strdup(writer.c_str())));
	g_free(xml_str);
    return;
}

}; /* namespace Filter */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */


/** \file
 * Code for handling XSLT extensions.
 */
/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2006-2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "xslt.h"
#include "../extension.h"
#include "../output.h"

#include "xml/repr.h"
#include "io/sys.h"
#include "file.h"
#include <unistd.h>

#include <libxml/parser.h>
#include <libxslt/transform.h>

Inkscape::XML::Document * sp_repr_do_read (xmlDocPtr doc, const gchar * default_ns);

/* Namespaces */
namespace Inkscape {
namespace Extension {
namespace Implementation {

/* Real functions */
/**
    \return    A XSLT object
    \brief     This function creates a XSLT object and sets up the
               variables.

*/
XSLT::XSLT(void) :
    Implementation(),
    _filename(""),
    _parsedDoc(NULL),
    _stylesheet(NULL)
{
}

Glib::ustring
XSLT::solve_reldir(Inkscape::XML::Node *reprin) {

    gchar const *s = reprin->attribute("reldir");

    if (!s) {
        Glib::ustring str = sp_repr_children(reprin)->content();
        return str;
    }

    Glib::ustring reldir = s;

    if (reldir == "extensions") {

        for (unsigned int i=0;
            i < Inkscape::Extension::Extension::search_path.size();
            i++) {

            gchar * fname = g_build_filename(
               Inkscape::Extension::Extension::search_path[i],
               sp_repr_children(reprin)->content(),
               NULL);
            Glib::ustring filename = fname;
            g_free(fname);

            if ( Inkscape::IO::file_test(filename.c_str(), G_FILE_TEST_EXISTS) )
                return filename;

        }
    } else {
        Glib::ustring str = sp_repr_children(reprin)->content();
        return str;
    }

    return "";
}

bool
XSLT::check(Inkscape::Extension::Extension *module)
{
    if (load(module)) {
        unload(module);
        return true;
    } else {
        return false;
    }
}

bool
XSLT::load(Inkscape::Extension::Extension *module)
{
    if (module->loaded()) { return true; }

    Inkscape::XML::Node *child_repr = sp_repr_children(module->get_repr());
    while (child_repr != NULL) {
        if (!strcmp(child_repr->name(), INKSCAPE_EXTENSION_NS "xslt")) {
            child_repr = sp_repr_children(child_repr);
            while (child_repr != NULL) {
                if (!strcmp(child_repr->name(), INKSCAPE_EXTENSION_NS "file")) {
                    _filename = solve_reldir(child_repr);
                }
                child_repr = sp_repr_next(child_repr);
            }

            break;
        }
        child_repr = sp_repr_next(child_repr);
    }

    _parsedDoc = xmlParseFile(_filename.c_str());
    if (_parsedDoc == NULL) { return false; }

    _stylesheet = xsltParseStylesheetDoc(_parsedDoc);

    return true;
}

void
XSLT::unload(Inkscape::Extension::Extension *module)
{
    if (!module->loaded()) { return; }
    xsltFreeStylesheet(_stylesheet);
    xmlFreeDoc(_parsedDoc);
    return;
}

SPDocument *
XSLT::open(Inkscape::Extension::Input */*module*/, gchar const *filename)
{
    xmlDocPtr filein = xmlParseFile(filename);
    if (filein == NULL) { return NULL; }

    const char * params[1];
    params[0] = NULL;

    xmlDocPtr result = xsltApplyStylesheet(_stylesheet, filein, params);
    xmlFreeDoc(filein);

    Inkscape::XML::Document * rdoc = sp_repr_do_read( result, SP_SVG_NS_URI);
    xmlFreeDoc(result);

    if (rdoc == NULL) {
        return NULL;
    }

    if (strcmp(rdoc->root()->name(), "svg:svg") != 0) {
        return NULL;
    }

    gchar * base = NULL;
    gchar * name = NULL;
    gchar * s = NULL, * p = NULL;
    s = g_strdup(filename);
    p = strrchr(s, '/');
    if (p) {
        name = g_strdup(p + 1);
        p[1] = '\0';
        base = g_strdup(s);
    } else {
        base = NULL;
        name = g_strdup(filename);
    }
    g_free(s);

    SPDocument * doc = sp_document_create(rdoc, filename, base, name, true);

    g_free(base); g_free(name);

    return doc;
}

void
XSLT::save(Inkscape::Extension::Output */*module*/, SPDocument *doc, gchar const *filename)
{
    g_return_if_fail(doc != NULL);
    g_return_if_fail(filename != NULL);

    Inkscape::XML::Node *repr = NULL;
    repr = sp_document_repr_root (doc);

    gchar *save_path = g_path_get_dirname (filename);
    Inkscape::IO::fixupHrefs( doc, save_path, true );
    g_free(save_path);

    std::string tempfilename_out;
    int tempfd_out = 0;
    try {
        tempfd_out = Inkscape::IO::file_open_tmp(tempfilename_out, "ink_ext_XXXXXX");
    } catch (...) {
        /// \todo Popup dialog here
        return;
    }

    gboolean const s = sp_repr_save_file (repr->document(), tempfilename_out.c_str(), SP_SVG_NS_URI);
    if (s == FALSE) {
        throw Inkscape::Extension::Output::save_failed();
    }

    xmlDocPtr svgdoc = xmlParseFile(tempfilename_out.c_str());
    close(tempfd_out);
    if (svgdoc == NULL) {
        return;
    }

    const char * params[1];
    params[0] = NULL;

    xmlDocPtr newdoc = xsltApplyStylesheet(_stylesheet, svgdoc, params);
    xmlSaveFile(filename, newdoc);

    xmlFreeDoc(newdoc);
    xmlFreeDoc(svgdoc);

    return;
}


}  /* Implementation  */
}  /* module  */
}  /* Inkscape  */


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

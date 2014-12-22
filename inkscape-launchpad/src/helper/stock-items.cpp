/*
 * Stock-items
 *
 * Stock Item management code
 *
 * Authors:
 *  John Cliff <simarilius@yahoo.com>
 *  Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright 2004 John Cliff
 *
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define noSP_SS_VERBOSE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>

#include "path-prefix.h"

#include <xml/repr.h>
#include "sp-gradient.h"
#include "document-private.h"
#include "sp-pattern.h"
#include "sp-marker.h"
#include "desktop.h"
#include "inkscape.h"

#include "io/sys.h"
#include "stock-items.h"



static SPObject *sp_gradient_load_from_svg(gchar const *name, SPDocument *current_doc);
static SPObject *sp_marker_load_from_svg(gchar const *name, SPDocument *current_doc);
static SPObject *sp_gradient_load_from_svg(gchar const *name, SPDocument *current_doc);


// FIXME: these should be merged with the icon loading code so they
// can share a common file/doc cache.  This function should just 
// take the dir to look in, and the file to check for, and cache
// against that, rather than the existing copy/paste code seen here.

static SPObject * sp_marker_load_from_svg(gchar const *name, SPDocument *current_doc)
{
    static SPDocument *doc = NULL;
    static unsigned int edoc = FALSE;
    if (!current_doc) {
        return NULL;
    }
    /* Try to load from document */
    if (!edoc && !doc) {
        gchar *markers = g_build_filename(INKSCAPE_MARKERSDIR, "/markers.svg", NULL);
        if (Inkscape::IO::file_test(markers, G_FILE_TEST_IS_REGULAR)) {
            doc = SPDocument::createNewDoc(markers, FALSE);
        }
        g_free(markers);
        if (doc) {
            doc->ensureUpToDate();
        } else {
            edoc = TRUE;
        }
    }
    if (!edoc && doc) {
        /* Get the marker we want */
        SPObject *object = doc->getObjectById(name);
        if (object && SP_IS_MARKER(object)) {
            SPDefs *defs = current_doc->getDefs();
            Inkscape::XML::Document *xml_doc = current_doc->getReprDoc();
            Inkscape::XML::Node *mark_repr = object->getRepr()->duplicate(xml_doc);
            defs->getRepr()->addChild(mark_repr, NULL);
            SPObject *cloned_item = current_doc->getObjectByRepr(mark_repr);
            Inkscape::GC::release(mark_repr);
            return cloned_item;
        }
    }
    return NULL;
}


static SPObject *
sp_pattern_load_from_svg(gchar const *name, SPDocument *current_doc)
{
    static SPDocument *doc = NULL;
    static unsigned int edoc = FALSE;
    if (!current_doc) {
        return NULL;
    }
    /* Try to load from document */
    if (!edoc && !doc) {
        gchar *patterns = g_build_filename(INKSCAPE_PATTERNSDIR, "/patterns.svg", NULL);
        if (Inkscape::IO::file_test(patterns, G_FILE_TEST_IS_REGULAR)) {
            doc = SPDocument::createNewDoc(patterns, FALSE);
        }
        if (!doc) {
        gchar *patterns = g_build_filename(CREATE_PATTERNSDIR, "/patterns.svg", NULL);
        if (Inkscape::IO::file_test(patterns, G_FILE_TEST_IS_REGULAR)) {
            doc = SPDocument::createNewDoc(patterns, FALSE);
        }
        g_free(patterns);
        if (doc) {
            doc->ensureUpToDate();
        } else {
            edoc = TRUE;
        }
        }
    }
    if (!edoc && doc) {
        /* Get the pattern we want */
        SPObject *object = doc->getObjectById(name);
        if (object && SP_IS_PATTERN(object)) {
            SPDefs *defs = current_doc->getDefs();
            Inkscape::XML::Document *xml_doc = current_doc->getReprDoc();
            Inkscape::XML::Node *pat_repr = object->getRepr()->duplicate(xml_doc);
            defs->getRepr()->addChild(pat_repr, NULL);
            Inkscape::GC::release(pat_repr);
            return object;
        }
    }
    return NULL;
}


static SPObject *
sp_gradient_load_from_svg(gchar const *name, SPDocument *current_doc)
{
    static SPDocument *doc = NULL;
    static unsigned int edoc = FALSE;
    if (!current_doc) {
        return NULL;
    }
    /* Try to load from document */
    if (!edoc && !doc) {
        gchar *gradients = g_build_filename(INKSCAPE_GRADIENTSDIR, "/gradients.svg", NULL);
        if (Inkscape::IO::file_test(gradients, G_FILE_TEST_IS_REGULAR)) {
            doc = SPDocument::createNewDoc(gradients, FALSE);
        }
        if (!doc) {
        gchar *gradients = g_build_filename(CREATE_GRADIENTSDIR, "/gradients.svg", NULL);
        if (Inkscape::IO::file_test(gradients, G_FILE_TEST_IS_REGULAR)) {
            doc = SPDocument::createNewDoc(gradients, FALSE);
        }
        g_free(gradients);
        if (doc) {
            doc->ensureUpToDate();
        } else {
            edoc = TRUE;
        }
        }
    }
    if (!edoc && doc) {
        /* Get the gradient we want */
        SPObject *object = doc->getObjectById(name);
        if (object && SP_IS_GRADIENT(object)) {
            SPDefs *defs = current_doc->getDefs();
            Inkscape::XML::Document *xml_doc = current_doc->getReprDoc();
            Inkscape::XML::Node *pat_repr = object->getRepr()->duplicate(xml_doc);
            defs->getRepr()->addChild(pat_repr, NULL);
            Inkscape::GC::release(pat_repr);
            return object;
        }
    }
    return NULL;
}

// get_stock_item returns a pointer to an instance of the desired stock object in the current doc
// if necessary it will import the object. Copes with name clashes through use of the inkscape:stockid property
// This should be set to be the same as the id in the libary file.

SPObject *get_stock_item(gchar const *urn, gboolean stock)
{
    g_assert(urn != NULL);
    
    /* check its an inkscape URN */
    if (!strncmp (urn, "urn:inkscape:", 13)) {

        gchar const *e = urn + 13;
        int a = 0;
        gchar * name = g_strdup(e);
        gchar *name_p = name;
        while (*name_p != ':' && *name_p != '\0'){
            name_p++;
            a++;
        }
        
        if (*name_p ==':') {
            name_p++;
        }
        
        gchar * base = g_strndup(e, a);

        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        SPDocument *doc = desktop->getDocument();
        SPDefs *defs = doc->getDefs();
        if (!defs) {
            g_free(base);
            return NULL;
        }
        SPObject *object = NULL;
        if (!strcmp(base, "marker") && !stock) {
            for ( SPObject *child = defs->firstChild(); child; child = child->getNext() )
            {
                if (child->getRepr()->attribute("inkscape:stockid") &&
                    !strcmp(name_p, child->getRepr()->attribute("inkscape:stockid")) &&
                    SP_IS_MARKER(child))
                {
                    object = child;
                }
            }
            
        }
        else if (!strcmp(base,"pattern") && !stock)  {
            for ( SPObject *child = defs->firstChild() ; child; child = child->getNext() )
            {
                if (child->getRepr()->attribute("inkscape:stockid") &&
                    !strcmp(name_p, child->getRepr()->attribute("inkscape:stockid")) &&
                    SP_IS_PATTERN(child))
                {
                    object = child;
                }
            }
            
        }
        else if (!strcmp(base,"gradient") && !stock)  {
            for ( SPObject *child = defs->firstChild(); child; child = child->getNext() )
            {
                if (child->getRepr()->attribute("inkscape:stockid") &&
                    !strcmp(name_p, child->getRepr()->attribute("inkscape:stockid")) &&
                    SP_IS_GRADIENT(child))
                {
                    object = child;
                }
            }
            
        }
        
        if (object == NULL) {
            
            if (!strcmp(base, "marker"))  {
                object = sp_marker_load_from_svg(name_p, doc);
            }
            else if (!strcmp(base, "pattern"))  {
                object = sp_pattern_load_from_svg(name_p, doc);
            }
            else if (!strcmp(base, "gradient"))  {
                object = sp_gradient_load_from_svg(name_p, doc);
            }
        }
        
        g_free(base);
        g_free(name);
        
        if (object) {
            object->getRepr()->setAttribute("inkscape:isstock", "true");
        }

        return object;
    }
    
    else {
        
        SPDesktop *desktop = SP_ACTIVE_DESKTOP;
        SPDocument *doc = desktop->getDocument();
        SPObject *object = doc->getObjectById(urn);

        return object;
    }
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

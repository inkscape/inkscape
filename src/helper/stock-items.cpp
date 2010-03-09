#define __INK_STOCK_ITEMS__

/*
 * Stock-items
 *
 * Stock Item management code
 *
 * Authors:
 *  John Cliff <simarilius@yahoo.com>
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
#include "path-prefix.h"


#include <xml/repr.h>
#include "sp-gradient.h"
#include "document-private.h"
#include "sp-pattern.h"
#include "marker.h"
#include "desktop-handles.h"
#include "inkscape.h"

#include "io/sys.h"




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
            doc = sp_document_new(markers, FALSE);
        }
        g_free(markers);
        if (doc) {
            sp_document_ensure_up_to_date(doc);
        } else {
            edoc = TRUE;
        }
    }
    if (!edoc && doc) {
        /* Get the marker we want */
        SPObject *object = doc->getObjectById(name);
        if (object && SP_IS_MARKER(object)) {
            SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS(current_doc);
            Inkscape::XML::Document *xml_doc = sp_document_repr_doc(current_doc);
            Inkscape::XML::Node *mark_repr = SP_OBJECT_REPR(object)->duplicate(xml_doc);
            SP_OBJECT_REPR(defs)->addChild(mark_repr, NULL);
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
            doc = sp_document_new(patterns, FALSE);
        }
        if (!doc) {
        gchar *patterns = g_build_filename(CREATE_PATTERNSDIR, "/patterns.svg", NULL);
        if (Inkscape::IO::file_test(patterns, G_FILE_TEST_IS_REGULAR)) {
            doc = sp_document_new(patterns, FALSE);
        }
        g_free(patterns);
        if (doc) {
            sp_document_ensure_up_to_date(doc);
        } else {
            edoc = TRUE;
        }
        }
    }
    if (!edoc && doc) {
        /* Get the pattern we want */
        SPObject *object = doc->getObjectById(name);
        if (object && SP_IS_PATTERN(object)) {
            SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS(current_doc);
            Inkscape::XML::Document *xml_doc = sp_document_repr_doc(current_doc);
            Inkscape::XML::Node *pat_repr = SP_OBJECT_REPR(object)->duplicate(xml_doc);
            SP_OBJECT_REPR(defs)->addChild(pat_repr, NULL);
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
            doc = sp_document_new(gradients, FALSE);
        }
        if (!doc) {
        gchar *gradients = g_build_filename(CREATE_GRADIENTSDIR, "/gradients.svg", NULL);
        if (Inkscape::IO::file_test(gradients, G_FILE_TEST_IS_REGULAR)) {
            doc = sp_document_new(gradients, FALSE);
        }
        g_free(gradients);
        if (doc) {
            sp_document_ensure_up_to_date(doc);
        } else {
            edoc = TRUE;
        }
        }
    }
    if (!edoc && doc) {
        /* Get the gradient we want */
        SPObject *object = doc->getObjectById(name);
        if (object && SP_IS_GRADIENT(object)) {
            SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS(current_doc);
            Inkscape::XML::Document *xml_doc = sp_document_repr_doc(current_doc);
            Inkscape::XML::Node *pat_repr = SP_OBJECT_REPR(object)->duplicate(xml_doc);
            SP_OBJECT_REPR(defs)->addChild(pat_repr, NULL);
            Inkscape::GC::release(pat_repr);
            return object;
        }
    }
    return NULL;
}

// get_stock_item returns a pointer to an instance of the desired stock object in the current doc
// if necessary it will import the object. Copes with name clashes through use of the inkscape:stockid property
// This should be set to be the same as the id in the libary file.

SPObject *get_stock_item(gchar const *urn)
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

        SPDesktop *desktop = inkscape_active_desktop();
        SPDocument *doc = sp_desktop_document(desktop);
        SPDefs *defs= (SPDefs *) SP_DOCUMENT_DEFS(doc);

        SPObject *object = NULL;
        if (!strcmp(base, "marker")) {
            for (SPObject *child = sp_object_first_child(SP_OBJECT(defs));
                 child != NULL;
                 child = SP_OBJECT_NEXT(child))
            {
                if (SP_OBJECT_REPR(child)->attribute("inkscape:stockid") &&
                    !strcmp(name_p, SP_OBJECT_REPR(child)->attribute("inkscape:stockid")) &&
                    SP_IS_MARKER(child))
                {
                    object = child;
                }
            }
            
        }
        else if (!strcmp(base,"pattern"))  {
            for (SPObject *child = sp_object_first_child(SP_OBJECT(defs)) ;
                 child != NULL;
                 child = SP_OBJECT_NEXT(child) )
            {
                if (SP_OBJECT_REPR(child)->attribute("inkscape:stockid") &&
                    !strcmp(name_p, SP_OBJECT_REPR(child)->attribute("inkscape:stockid")) &&
                    SP_IS_PATTERN(child))
                {
                    object = child;
                }
            }
            
        }
        else if (!strcmp(base,"gradient"))  {
            for (SPObject *child = sp_object_first_child(SP_OBJECT(defs));
                 child != NULL;
                 child = SP_OBJECT_NEXT(child))
            {
                if (SP_OBJECT_REPR(child)->attribute("inkscape:stockid") &&
                    !strcmp(name_p, SP_OBJECT_REPR(child)->attribute("inkscape:stockid")) &&
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
        
        return object;
    }
    
    else {
        
        SPDesktop *desktop = inkscape_active_desktop();
        SPDocument *doc = sp_desktop_document(desktop);
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

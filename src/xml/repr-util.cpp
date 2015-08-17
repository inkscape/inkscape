/**
 * @file
 * Miscellaneous helpers for reprs.
 */

/*
 * Authors:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * g++ port Copyright (C) 2003 Nathan Hurst
 *
 * Licensed under GNU GPL
 */

#include "config.h"

#include <math.h>

#if HAVE_STRING_H
# include <cstring>
#endif

#if HAVE_STDLIB_H
# include <cstdlib>
#endif


#include <glib.h>
#include <2geom/point.h>
#include "svg/stringstream.h"
#include "svg/css-ostringstream.h"
#include "svg/svg-length.h"

#include "xml/repr.h"
#include "xml/repr-sorting.h"


#define OSB_NS_URI "http://www.openswatchbook.org/uri/2009/osb"


struct SPXMLNs {
    SPXMLNs *next;
    unsigned int uri, prefix;
};

/*#####################
# DEFINITIONS
#####################*/

#ifndef FALSE
# define FALSE 0
#endif

#ifndef TRUE
# define TRUE (!FALSE)
#endif

#ifndef MAX
# define MAX(a,b) (((a) < (b)) ? (b) : (a))
#endif

/*#####################
# FORWARD DECLARATIONS
#####################*/

static void sp_xml_ns_register_defaults();
static char *sp_xml_ns_auto_prefix(char const *uri);

/*#####################
# MAIN
#####################*/

/**
 * SPXMLNs
 */

static SPXMLNs *namespaces=NULL;

/*
 * There are the prefixes to use for the XML namespaces defined
 * in repr.h
 */
static void sp_xml_ns_register_defaults()
{
    static SPXMLNs defaults[11];

    defaults[0].uri = g_quark_from_static_string(SP_SODIPODI_NS_URI);
    defaults[0].prefix = g_quark_from_static_string("sodipodi");
    defaults[0].next = &defaults[1];

    defaults[1].uri = g_quark_from_static_string(SP_XLINK_NS_URI);
    defaults[1].prefix = g_quark_from_static_string("xlink");
    defaults[1].next = &defaults[2];

    defaults[2].uri = g_quark_from_static_string(SP_SVG_NS_URI);
    defaults[2].prefix = g_quark_from_static_string("svg");
    defaults[2].next = &defaults[3];

    defaults[3].uri = g_quark_from_static_string(SP_INKSCAPE_NS_URI);
    defaults[3].prefix = g_quark_from_static_string("inkscape");
    defaults[3].next = &defaults[4];

    defaults[4].uri = g_quark_from_static_string(SP_RDF_NS_URI);
    defaults[4].prefix = g_quark_from_static_string("rdf");
    defaults[4].next = &defaults[5];

    defaults[5].uri = g_quark_from_static_string(SP_CC_NS_URI);
    defaults[5].prefix = g_quark_from_static_string("cc");
    defaults[5].next = &defaults[6];

    defaults[6].uri = g_quark_from_static_string(SP_DC_NS_URI);
    defaults[6].prefix = g_quark_from_static_string("dc");
    defaults[6].next = &defaults[7];

    defaults[7].uri = g_quark_from_static_string(OSB_NS_URI);
    defaults[7].prefix = g_quark_from_static_string("osb");
    defaults[7].next = &defaults[8];

    // Inkscape versions prior to 0.44 would write this namespace
    // URI instead of the correct sodipodi namespace; by adding this
    // entry to the table last (where it gets used for URI -> prefix
    // lookups, but not prefix -> URI lookups), we effectively transfer
    // elements in this namespace to the correct sodipodi namespace:

    defaults[8].uri = g_quark_from_static_string(SP_BROKEN_SODIPODI_NS_URI);
    defaults[8].prefix = g_quark_from_static_string("sodipodi");
    defaults[8].next = &defaults[9];

    // "Duck prion"
    // This URL became widespread due to a bug in versions <= 0.43

    defaults[9].uri = g_quark_from_static_string("http://inkscape.sourceforge.net/DTD/s odipodi-0.dtd");
    defaults[9].prefix = g_quark_from_static_string("sodipodi");
    defaults[9].next = &defaults[10];

    // This namespace URI is being phased out by Creative Commons

    defaults[10].uri = g_quark_from_static_string(SP_OLD_CC_NS_URI);
    defaults[10].prefix = g_quark_from_static_string("cc");
    defaults[10].next = NULL;

    namespaces = &defaults[0];
}

char *sp_xml_ns_auto_prefix(char const *uri)
{
    char const *start, *end;
    char *new_prefix;
    start = uri;
    while ((end = strpbrk(start, ":/"))) {
        start = end + 1;
    }
    end = start + strspn(start, "abcdefghijklmnopqrstuvwxyz");
    if (end == start) {
        start = "ns";
        end = start + 2;
    }
    new_prefix = g_strndup(start, end - start);
    if (sp_xml_ns_prefix_uri(new_prefix)) {
        char *temp;
        int counter=0;
        do {
            temp = g_strdup_printf("%s%d", new_prefix, counter++);
        } while (sp_xml_ns_prefix_uri(temp));
        g_free(new_prefix);
        new_prefix = temp;
    }
    return new_prefix;
}

gchar const *sp_xml_ns_uri_prefix(gchar const *uri, gchar const *suggested)
{
    char const *prefix;

    if (!uri) return NULL;

    if (!namespaces) {
        sp_xml_ns_register_defaults();
    }

    GQuark const key = g_quark_from_string(uri);
    prefix = NULL;
    for ( SPXMLNs *iter=namespaces ; iter ; iter = iter->next ) {
        if ( iter->uri == key ) {
            prefix = g_quark_to_string(iter->prefix);
            break;
        }
    }

    if (!prefix) {
        char *new_prefix;
        SPXMLNs *ns;
        if (suggested) {
            GQuark const prefix_key=g_quark_from_string(suggested);

            SPXMLNs *found=namespaces;
            while (found) {
                if (found->prefix != prefix_key) {
                    found = found->next;
                }
                else {
                    break;
                }
            }

            if (found) { // prefix already used?
                new_prefix = sp_xml_ns_auto_prefix(uri);
            } else { // safe to use suggested
                new_prefix = g_strdup(suggested);
            }
        } else {
            new_prefix = sp_xml_ns_auto_prefix(uri);
        }

        ns = g_new(SPXMLNs, 1);
        g_assert( ns != NULL );
        ns->uri = g_quark_from_string(uri);
        ns->prefix = g_quark_from_string(new_prefix);

        g_free(new_prefix);

        ns->next = namespaces;
        namespaces = ns;

        prefix = g_quark_to_string(ns->prefix);
    }

    return prefix;
}

gchar const *sp_xml_ns_prefix_uri(gchar const *prefix)
{
    SPXMLNs *iter;
    char const *uri;

    if (!prefix) return NULL;

    if (!namespaces) {
        sp_xml_ns_register_defaults();
    }

    GQuark const key = g_quark_from_string(prefix);
    uri = NULL;
    for ( iter = namespaces ; iter ; iter = iter->next ) {
        if ( iter->prefix == key ) {
            uri = g_quark_to_string(iter->uri);
            break;
        }
    }
    return uri;
}

/** 
 *  Works for different-parent objects, so long as they have a common ancestor. Return value:
 *    0    positions are equivalent
 *    1    first object's position is greater than the second
 *   -1    first object's position is less than the second
 * @todo Rewrite this function's description to be understandable
 */
int sp_repr_compare_position(Inkscape::XML::Node const *first, Inkscape::XML::Node const *second)
{
    int p1, p2;
    if (first->parent() == second->parent()) {
        /* Basic case - first and second have same parent */
        p1 = first->position();
        p2 = second->position();
    } else {
        /* Special case - the two objects have different parents.  They
           could be in different groups or on different layers for
           instance. */

        // Find the lowest common ancestor(LCA)
        Inkscape::XML::Node const *ancestor = LCA(first, second);
        g_assert(ancestor != NULL);

        if (ancestor == first) {
            return 1;
        } else if (ancestor == second) {
            return -1;
        } else {
            Inkscape::XML::Node const *to_first = AncetreFils(first, ancestor);
            Inkscape::XML::Node const *to_second = AncetreFils(second, ancestor);
            g_assert(to_second->parent() == to_first->parent());
            p1 = to_first->position();
            p2 = to_second->position();
        }
    }

    if (p1 > p2) return 1;
    if (p1 < p2) return -1;
    return 0;

    /* effic: Assuming that the parent--child relationship is consistent
       (i.e. that the parent really does contain first and second among
       its list of children), it should be equivalent to walk along the
       children and see which we encounter first (returning 0 iff first
       == second).
       
       Given that this function is used solely for sorting, we can use a
       similar approach to do the sort: gather the things to be sorted,
       into an STL vector (to allow random access and faster
       traversals).  Do a single pass of the parent's children; for each
       child, do a pass on whatever items in the vector we haven't yet
       encountered.  If the child is found, then swap it to the
       beginning of the yet-unencountered elements of the vector.
       Continue until no more than one remains unencountered.  --
       pjrm */
}

bool sp_repr_compare_position_bool(Inkscape::XML::Node const *first, Inkscape::XML::Node const *second){
    return sp_repr_compare_position(first, second)<0;
}


/**
 * Find an element node using an unique attribute.
 *
 * This function returns the first child of the specified node that has the attribute
 * @c key equal to @c value. Note that this function does not recurse.
 *
 * @param repr The node to start from
 * @param key The name of the attribute to use for comparisons
 * @param value The value of the attribute to look for
 * @relatesalso Inkscape::XML::Node
 */
Inkscape::XML::Node *sp_repr_lookup_child(Inkscape::XML::Node *repr,
                                          gchar const *key,
                                          gchar const *value)
{
    g_return_val_if_fail(repr != NULL, NULL);
    for ( Inkscape::XML::Node *child = repr->firstChild() ; child ; child = child->next() ) {
        gchar const *child_value = child->attribute(key);
        if ( (child_value == value) ||
             (value && child_value && !strcmp(child_value, value)) )
        {
            return child;
        }
    }
    return NULL;
}

Inkscape::XML::Node const *sp_repr_lookup_name( Inkscape::XML::Node const *repr, gchar const *name, gint maxdepth )
{
    Inkscape::XML::Node const *found = 0;
    g_return_val_if_fail(repr != NULL, NULL);
    g_return_val_if_fail(name != NULL, NULL);

    GQuark const quark = g_quark_from_string(name);

    if ( (GQuark)repr->code() == quark ) {
        found = repr;
    } else if ( maxdepth != 0 ) {
        // maxdepth == -1 means unlimited
        if ( maxdepth == -1 ) {
            maxdepth = 0;
        }

        for (Inkscape::XML::Node const *child = repr->firstChild() ; child && !found; child = child->next() ) {
            found = sp_repr_lookup_name( child, name, maxdepth - 1 );
        }
    }
    return found;
}

Inkscape::XML::Node *sp_repr_lookup_name( Inkscape::XML::Node *repr, gchar const *name, gint maxdepth )
{
    Inkscape::XML::Node const *found = sp_repr_lookup_name( const_cast<Inkscape::XML::Node const *>(repr), name, maxdepth );
    return const_cast<Inkscape::XML::Node *>(found);
}

std::vector<Inkscape::XML::Node const *> sp_repr_lookup_name_many( Inkscape::XML::Node const *repr, gchar const *name, gint maxdepth )
{
    std::vector<Inkscape::XML::Node const *> nodes;
    std::vector<Inkscape::XML::Node const *> found;
    g_return_val_if_fail(repr != NULL, nodes);
    g_return_val_if_fail(name != NULL, nodes);

    GQuark const quark = g_quark_from_string(name);

    if ( (GQuark)repr->code() == quark ) {
        nodes.push_back(repr);
    }

    if ( maxdepth != 0 ) {
        // maxdepth == -1 means unlimited
        if ( maxdepth == -1 ) {
            maxdepth = 0;
        }

        for (Inkscape::XML::Node const *child = repr->firstChild() ; child; child = child->next() ) {
            found = sp_repr_lookup_name_many( child, name, maxdepth - 1);
            nodes.insert(nodes.end(), found.begin(), found.end());
        }
    }

    return nodes;
}

/**
 * Determine if the node is a 'title', 'desc' or 'metadata' element.
 */
bool sp_repr_is_meta_element(const Inkscape::XML::Node *node)
{
    if (node == NULL) return false;
    if (node->type() != Inkscape::XML::ELEMENT_NODE) return false;
    gchar const *name = node->name();
    if (name == NULL) return false;
    if (!std::strcmp(name, "svg:title")) return true;
    if (!std::strcmp(name, "svg:desc")) return true;
    if (!std::strcmp(name, "svg:metadata")) return true;
    return false;
}

/**
 * Parses the boolean value of an attribute "key" in repr and sets val accordingly, or to FALSE if
 * the attr is not set.
 *
 * \return TRUE if the attr was set, FALSE otherwise.
 */
unsigned int sp_repr_get_boolean(Inkscape::XML::Node *repr, gchar const *key, unsigned int *val)
{
    gchar const *v;

    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(val != NULL, FALSE);

    v = repr->attribute(key);

    if (v != NULL) {
        if (!g_ascii_strcasecmp(v, "true") ||
            !g_ascii_strcasecmp(v, "yes" ) ||
            !g_ascii_strcasecmp(v, "y"   ) ||
            (atoi(v) != 0)) {
            *val = TRUE;
        } else {
            *val = FALSE;
        }
        return TRUE;
    } else {
        *val = FALSE;
        return FALSE;
    }
}

unsigned int sp_repr_get_int(Inkscape::XML::Node *repr, gchar const *key, int *val)
{
    gchar const *v;

    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(val != NULL, FALSE);

    v = repr->attribute(key);

    if (v != NULL) {
        *val = atoi(v);
        return TRUE;
    }

    return FALSE;
}

unsigned int sp_repr_get_double(Inkscape::XML::Node *repr, gchar const *key, double *val)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(val != NULL, FALSE);

    gchar const *v = repr->attribute(key);

    if (v != NULL) {
        *val = g_ascii_strtod(v, NULL);
        return TRUE;
    }

    return FALSE;
}

unsigned int sp_repr_set_boolean(Inkscape::XML::Node *repr, gchar const *key, unsigned int val)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);

    repr->setAttribute(key, (val) ? "true" : "false");
    return true;
}

unsigned int sp_repr_set_int(Inkscape::XML::Node *repr, gchar const *key, int val)
{
    gchar c[32];

    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);

    g_snprintf(c, 32, "%d", val);

    repr->setAttribute(key, c);
    return true;
}

/**
 * Set a property attribute to \a val [slightly rounded], in the format
 * required for CSS properties: in particular, it never uses exponent
 * notation.
 */
unsigned int sp_repr_set_css_double(Inkscape::XML::Node *repr, gchar const *key, double val)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);

    Inkscape::CSSOStringStream os;
    os << val;

    repr->setAttribute(key, os.str().c_str());
    return true;
}

/**
 * For attributes where an exponent is allowed.
 *
 * Not suitable for property attributes (fill-opacity, font-size etc.).
 */
unsigned int sp_repr_set_svg_double(Inkscape::XML::Node *repr, gchar const *key, double val)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(val==val, FALSE);//tests for nan

    Inkscape::SVGOStringStream os;
    os << val;

    repr->setAttribute(key, os.str().c_str());
    return true;
}

/**
 * For attributes where an exponent is allowed.
 *
 * Not suitable for property attributes.
 */
unsigned int sp_repr_set_svg_length(Inkscape::XML::Node *repr, gchar const *key, SVGLength &val)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);

    repr->setAttribute(key, val.write());
    return true;
}

unsigned sp_repr_set_point(Inkscape::XML::Node *repr, gchar const *key, Geom::Point const & val)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);

    Inkscape::SVGOStringStream os;
    os << val[Geom::X] << "," << val[Geom::Y];

    repr->setAttribute(key, os.str().c_str());
    return true;
}

unsigned int sp_repr_get_point(Inkscape::XML::Node *repr, gchar const *key, Geom::Point *val)
{
    g_return_val_if_fail(repr != NULL, FALSE);
    g_return_val_if_fail(key != NULL, FALSE);
    g_return_val_if_fail(val != NULL, FALSE);

    gchar const *v = repr->attribute(key);

    g_return_val_if_fail(v != NULL, FALSE);

    gchar ** strarray = g_strsplit(v, ",", 2);

    if (strarray && strarray[0] && strarray[1]) {
        double newx, newy;
        newx = g_ascii_strtod(strarray[0], NULL);
        newy = g_ascii_strtod(strarray[1], NULL);
        g_strfreev (strarray);
        *val = Geom::Point(newx, newy);
        return TRUE;
    }

    g_strfreev (strarray);
    return FALSE;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

#ifndef __SP_REPR_H__
#define __SP_REPR_H__

/** \file
 * C facade to Inkscape::XML::Node.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <stdio.h>
#include <glib/gtypes.h>
#include "gc-anchored.h"

#include "xml/node.h"
#include "xml/document.h"
#include "xml/sp-css-attr.h"
#include "io/inkscapestream.h"

#include <2geom/point.h>

#define SP_SODIPODI_NS_URI "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
#define SP_BROKEN_SODIPODI_NS_URI "http://inkscape.sourceforge.net/DTD/sodipodi-0.dtd"
#define SP_INKSCAPE_NS_URI "http://www.inkscape.org/namespaces/inkscape"
#define SP_XLINK_NS_URI "http://www.w3.org/1999/xlink"
#define SP_SVG_NS_URI "http://www.w3.org/2000/svg"
#define SP_RDF_NS_URI "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define SP_CC_NS_URI "http://creativecommons.org/ns#"
#define SP_OLD_CC_NS_URI "http://web.resource.org/cc/"
#define SP_DC_NS_URI "http://purl.org/dc/elements/1.1/"

/**
 * \note NB! Unless explicitly stated all methods are noref/nostrcpy
 */

/** \todo
 * Though Inkscape::XML::Node provides "signals" for notification when 
 * individual nodes change, there is no mechanism to receive notification 
 * for overall document changes.
 * However, with the addition of the transactions code, it would not be
 * very hard to implement if you wanted it.
 * 
 * \class Inkscape::XML::Node
 * \note
 * Inkscape::XML::Node itself doesn't use GObject signals at present -- 
 * Inkscape::XML::Nodes maintain lists of Inkscape::XML::NodeEventVectors 
 * (added via sp_repr_add_listener), which are used to specify callbacks 
 * when something changes.
 *
 * Here are the current callbacks in an event vector (they may be NULL):
 *
 * void (* child_added)(Inkscape::XML::Node *repr, Inkscape::XML::Node *child, 
 * Inkscape::XML::Node *ref, void *data); Called once a child has been added.
 *
 * void (* child_removed)(Inkscape::XML::Node *repr, 
 * Inkscape::XML::Node *child, Inkscape::XML::Node *ref, void *data);
 * Called after a child is removed; ref is the child that used to precede
 * the removed child.
 *
 * void (* attr_changed)(Inkscape::XML::Node *repr, gchar const *key, 
 * gchar const *oldval, gchar const *newval, void *data);
 * Called after an attribute has been changed.
 *
 * void (* content_changed)(Inkscape::XML::Node *repr, gchar const *oldcontent,
 * gchar const *newcontent, void *data);
 * Called after an element's content has been changed.
 *
 * void (* order_changed)(Inkscape::XML::Node *repr, Inkscape::XML::Node *child,
 * Inkscape::XML::Node *oldref, Inkscape::XML::Node *newref, void *data);
 * Called once the child has been moved to its new position in the child order.
 *
 * <b> Inkscape::XML::Node mini-FAQ </b>
 *
 * Since I'm not very familiar with this section of code but I need to use
 * it heavily for the RDF work, I'm going to answer various questions I run
 * into with my best-guess answers so others can follow along too.
 *
 * \arg
 * Q: How do I find the root Inkscape::XML::Node? <br>
 * A: If you have an SPDocument, use doc->rroot.  For example: 
 * 
 * \code    SP_ACTIVE_DOCUMENT->rroot       \endcode <br>
 * 
 * (but it's better to arrange for your caller to pass in the relevent
 * document rather than assuming it's necessarily the active one and
 * using SP_ACTIVE_DOCUMENT)
 *
 * \arg
 * Q: How do I find an Inkscape::XML::Node by unique key/value? <br>
 * A: Use sp_repr_lookup_child
 *
 * \arg
 * Q: How do I find an Inkscape::XML::Node by unique namespace name? <br>
 * A: Use sp_repr_lookup_name
 *
 * \arg
 * Q: How do I make an Inkscape::XML::Node namespace prefix constant in 
 * the application? <br>
 * A: Add the XML namespace URL as a #define to repr.h at the top with the
 * other SP_<NAMESPACE>_NS_URI #define's, and then in repr-util.cpp,
 * in sp_xml_ns_register_defaults, bump "defaults" up in size one, and
 * add another section.  Don't forget to increment the array offset and
 * keep ".next" pointed to the next (if any) array entry.
 *
 * \arg
 * Q: How do I create a new Inkscape::XML::Node? <br>
 * A: Use the appropriate create* method on Inkscape::XML::Document,
 * parent->appendChild(child), and then use Inkscape::GC::release(child) to
 * let go of it (the parent will hold it in memory).
 *
 * \arg
 * Q: How do I destroy an Inkscape::XML::Node?
 * A: Just call "sp_repr_unparent" on it and release any references
 * you may be retaining to it.  Any attached SPObjects will
 * clean themselves up automatically, as will any children.
 *
 * \arg
 * Q: What about listeners? <br>
 * A: I have no idea yet...
 *
 * \arg
 * Q: How do I add a namespace to a newly created document?  <br>
 * A: The current hack is in document.cpp:sp_document_create
 *
 * Kees Cook  2004-07-01, updated MenTaLguY 2005-01-25
 */

namespace Geom {
class Point;
}

/* SPXMLNs */
char const *sp_xml_ns_uri_prefix(gchar const *uri, gchar const *suggested);
char const *sp_xml_ns_prefix_uri(gchar const *prefix);

Inkscape::XML::Document *sp_repr_document_new(gchar const *rootname);

/* Contents */
/// Sets the node's \a key attribute to \a value.
inline unsigned sp_repr_set_attr(Inkscape::XML::Node *repr, gchar const *key, gchar const *value,
                                 bool is_interactive = false) {
    repr->setAttribute(key, value, is_interactive);
    return true;
}

/* Tree */
/// Returns the node's parent.
inline Inkscape::XML::Node *sp_repr_parent(Inkscape::XML::Node const *repr) {
    return const_cast<Inkscape::XML::Node *>(repr->parent());
}

/// Returns first child of node, resets iterator.
inline Inkscape::XML::Node const *sp_repr_children(Inkscape::XML::Node const *repr) {
    return ( repr ? repr->firstChild() : NULL );
}

/// Returns first child of node, resets iterator.
inline Inkscape::XML::Node *sp_repr_children(Inkscape::XML::Node *repr) {
    return ( repr ? repr->firstChild() : NULL );
}

/// Returns next child of node or NULL.
inline Inkscape::XML::Node const *sp_repr_next(Inkscape::XML::Node const *repr) {
    return ( repr ? repr->next() : NULL );
}

/// Returns next child of node or NULL.
inline Inkscape::XML::Node *sp_repr_next(Inkscape::XML::Node *repr) {
    return ( repr ? repr->next() : NULL );
}

/* IO */

Inkscape::XML::Document *sp_repr_read_file(gchar const *filename, gchar const *default_ns);
Inkscape::XML::Document *sp_repr_read_mem(gchar const *buffer, int length, gchar const *default_ns);
void sp_repr_write_stream (Inkscape::XML::Node *repr, Inkscape::IO::Writer &out,
                 gint indent_level,  bool add_whitespace, Glib::QueryQuark elide_prefix,
				 int inlineattrs, int indent);
Inkscape::XML::Document *sp_repr_read_buf (const Glib::ustring &buf, const gchar *default_ns);
Glib::ustring sp_repr_save_buf(Inkscape::XML::Document *doc);
void sp_repr_save_stream(Inkscape::XML::Document *doc, FILE *to_file, gchar const *default_ns=NULL, bool compress = false);
bool sp_repr_save_file(Inkscape::XML::Document *doc, gchar const *filename, gchar const *default_ns=NULL);

void sp_repr_print(Inkscape::XML::Node *repr);

/* CSS stuff */

SPCSSAttr *sp_repr_css_attr_new(void);
void sp_repr_css_attr_unref(SPCSSAttr *css);
SPCSSAttr *sp_repr_css_attr(Inkscape::XML::Node *repr, gchar const *attr);
SPCSSAttr *sp_repr_css_attr_inherited(Inkscape::XML::Node *repr, gchar const *attr);

gchar const *sp_repr_css_property(SPCSSAttr *css, gchar const *name, gchar const *defval);
void sp_repr_css_set_property(SPCSSAttr *css, gchar const *name, gchar const *value);
void sp_repr_css_unset_property(SPCSSAttr *css, gchar const *name);
bool sp_repr_css_property_is_unset(SPCSSAttr *css, gchar const *name);
double sp_repr_css_double_property(SPCSSAttr *css, gchar const *name, double defval);

gchar *sp_repr_css_write_string(SPCSSAttr *css);
void sp_repr_css_set(Inkscape::XML::Node *repr, SPCSSAttr *css, gchar const *key);
void sp_repr_css_merge(SPCSSAttr *dst, SPCSSAttr *src);
void sp_repr_css_attr_add_from_string(SPCSSAttr *css, const gchar *data);
void sp_repr_css_change(Inkscape::XML::Node *repr, SPCSSAttr *css, gchar const *key);
void sp_repr_css_change_recursive(Inkscape::XML::Node *repr, SPCSSAttr *css, gchar const *key);

void sp_repr_css_print(SPCSSAttr *css);

/* Utility finctions */
/// Remove \a repr from children of its parent node.
inline void sp_repr_unparent(Inkscape::XML::Node *repr) {
    Inkscape::XML::Node *parent=repr->parent();
    if (parent) {
        parent->removeChild(repr);
    }
}

/* Convenience */
unsigned sp_repr_get_boolean(Inkscape::XML::Node *repr, gchar const *key, unsigned *val);
unsigned sp_repr_get_int(Inkscape::XML::Node *repr, gchar const *key, int *val);
unsigned sp_repr_get_double(Inkscape::XML::Node *repr, gchar const *key, double *val);
unsigned sp_repr_set_boolean(Inkscape::XML::Node *repr, gchar const *key, unsigned val);
unsigned sp_repr_set_int(Inkscape::XML::Node *repr, gchar const *key, int val);
unsigned sp_repr_set_css_double(Inkscape::XML::Node *repr, gchar const *key, double val);
unsigned sp_repr_set_svg_double(Inkscape::XML::Node *repr, gchar const *key, double val);
unsigned sp_repr_set_point(Inkscape::XML::Node *repr, gchar const *key, Geom::Point val);
unsigned sp_repr_get_point(Inkscape::XML::Node *repr, gchar const *key, Geom::Point *val);

/// \deprecated !
double sp_repr_get_double_attribute(Inkscape::XML::Node *repr, gchar const *key, double def);
/// \deprecated !
long long int sp_repr_get_int_attribute(Inkscape::XML::Node *repr, gchar const *key, long long int def);
/* End Deprecated? */

int sp_repr_compare_position(Inkscape::XML::Node *first, Inkscape::XML::Node *second);

/* Searching */
Inkscape::XML::Node *sp_repr_lookup_name(Inkscape::XML::Node *repr,
                                         gchar const *name,
                                         gint maxdepth = -1);
Inkscape::XML::Node *sp_repr_lookup_child(Inkscape::XML::Node *repr,
                                          gchar const *key,
                                          gchar const *value);


inline Inkscape::XML::Node *sp_repr_document_first_child(Inkscape::XML::Document const *doc) {
    return const_cast<Inkscape::XML::Node *>(doc->firstChild());
}

#endif


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

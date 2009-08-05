/** @file
 * @brief C facade to Inkscape::XML::Node
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2000-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
 
#ifndef __SP_REPR_H__
#define __SP_REPR_H__

#include <stdio.h>
#include <glib/gtypes.h>
#include "gc-anchored.h"

#include "xml/node.h"
#include "xml/document.h"
#include "xml/sp-css-attr.h"
#include "io/inkscapestream.h"

#include <2geom/forward.h>

#define SP_SODIPODI_NS_URI "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
#define SP_BROKEN_SODIPODI_NS_URI "http://inkscape.sourceforge.net/DTD/sodipodi-0.dtd"
#define SP_INKSCAPE_NS_URI "http://www.inkscape.org/namespaces/inkscape"
#define SP_XLINK_NS_URI "http://www.w3.org/1999/xlink"
#define SP_SVG_NS_URI "http://www.w3.org/2000/svg"
#define SP_RDF_NS_URI "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define SP_CC_NS_URI "http://creativecommons.org/ns#"
#define SP_OLD_CC_NS_URI "http://web.resource.org/cc/"
#define SP_DC_NS_URI "http://purl.org/dc/elements/1.1/"

/* SPXMLNs */
char const *sp_xml_ns_uri_prefix(gchar const *uri, gchar const *suggested);
char const *sp_xml_ns_prefix_uri(gchar const *prefix);

Inkscape::XML::Document *sp_repr_document_new(gchar const *rootname);

/* Tree */
/// @deprecated Use the equivalent member function Inkscape::XML::Node::parent()
inline Inkscape::XML::Node *sp_repr_parent(Inkscape::XML::Node const *repr) {
    return const_cast<Inkscape::XML::Node *>(repr->parent());
}

/// @deprecated Use the equivalent member function Inkscape::XML::Node::firstChild()
inline Inkscape::XML::Node const *sp_repr_children(Inkscape::XML::Node const *repr) {
    return ( repr ? repr->firstChild() : NULL );
}

/// @deprecated Use the equivalent member function Inkscape::XML::Node::firstChild()
inline Inkscape::XML::Node *sp_repr_children(Inkscape::XML::Node *repr) {
    return ( repr ? repr->firstChild() : NULL );
}

/// @deprecated Use the equivalent member function Inkscape::XML::Node::next()
inline Inkscape::XML::Node const *sp_repr_next(Inkscape::XML::Node const *repr) {
    return ( repr ? repr->next() : NULL );
}

/// @deprecated Use the equivalent member function Inkscape::XML::Node::next()
inline Inkscape::XML::Node *sp_repr_next(Inkscape::XML::Node *repr) {
    return ( repr ? repr->next() : NULL );
}


/* IO */

Inkscape::XML::Document *sp_repr_read_file(gchar const *filename, gchar const *default_ns);
Inkscape::XML::Document *sp_repr_read_mem(gchar const *buffer, int length, gchar const *default_ns);
void sp_repr_write_stream(Inkscape::XML::Node *repr, Inkscape::IO::Writer &out,
                          gint indent_level,  bool add_whitespace, Glib::QueryQuark elide_prefix,
                          int inlineattrs, int indent,
                          gchar const *old_href_base = NULL,
                          gchar const *new_href_base = NULL);
Inkscape::XML::Document *sp_repr_read_buf (const Glib::ustring &buf, const gchar *default_ns);
Glib::ustring sp_repr_save_buf(Inkscape::XML::Document *doc);
void sp_repr_save_stream(Inkscape::XML::Document *doc, FILE *to_file,
                         gchar const *default_ns = NULL, bool compress = false,
                         gchar const *old_href_base = NULL,
                         gchar const *new_href_base = NULL);
bool sp_repr_save_file(Inkscape::XML::Document *doc, gchar const *filename, gchar const *default_ns=NULL);
bool sp_repr_save_rebased_file(Inkscape::XML::Document *doc, gchar const *filename_utf8,
                               gchar const *default_ns,
                               gchar const *old_base, gchar const *new_base_filename);


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

bool sp_repr_is_meta_element(const Inkscape::XML::Node *node);

/* Convenience */
unsigned sp_repr_get_boolean(Inkscape::XML::Node *repr, gchar const *key, unsigned *val);
unsigned sp_repr_get_int(Inkscape::XML::Node *repr, gchar const *key, int *val);
unsigned sp_repr_get_double(Inkscape::XML::Node *repr, gchar const *key, double *val);
unsigned sp_repr_set_boolean(Inkscape::XML::Node *repr, gchar const *key, unsigned val);
unsigned sp_repr_set_int(Inkscape::XML::Node *repr, gchar const *key, int val);
unsigned sp_repr_set_css_double(Inkscape::XML::Node *repr, gchar const *key, double val);
unsigned sp_repr_set_svg_double(Inkscape::XML::Node *repr, gchar const *key, double val);
unsigned sp_repr_set_point(Inkscape::XML::Node *repr, gchar const *key, Geom::Point const & val);
unsigned sp_repr_get_point(Inkscape::XML::Node *repr, gchar const *key, Geom::Point *val);

/// \deprecated Use sp_repr_get_double to check for success
double sp_repr_get_double_attribute(Inkscape::XML::Node *repr, gchar const *key, double def);
/// \deprecated Use sp_repr_get_int to check for success
long long int sp_repr_get_int_attribute(Inkscape::XML::Node *repr, gchar const *key, long long int def);

int sp_repr_compare_position(Inkscape::XML::Node *first, Inkscape::XML::Node *second);

/* Searching */
Inkscape::XML::Node *sp_repr_lookup_name(Inkscape::XML::Node *repr,
                                         gchar const *name,
                                         gint maxdepth = -1);
Inkscape::XML::Node *sp_repr_lookup_child(Inkscape::XML::Node *repr,
                                          gchar const *key,
                                          gchar const *value);


inline Inkscape::XML::Node *sp_repr_document_first_child(Inkscape::XML::DocumentTree const *doc) {
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

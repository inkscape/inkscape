#ifndef __SP_ATTRIBUTE_REL_UTIL_H__
#define __SP_ATTRIBUTE_REL_UTIL_H__

/*
 * attribute-rel-util.h
 *
 *  Created on: Sep 8, 2011
 *      Author: tavmjong
 */

#include <glibmm/ustring.h>
#include "xml/sp-css-attr.h"

using Inkscape::XML::Node;

/**
 * Utility functions for cleaning XML tree.
 */

/**
 * Enum for preferences
 */
enum SPAttrClean {
  SP_ATTR_CLEAN_ATTR_WARN      =  1,
  SP_ATTR_CLEAN_ATTR_REMOVE    =  2,
  SP_ATTR_CLEAN_STYLE_WARN     =  4,
  SP_ATTR_CLEAN_STYLE_REMOVE   =  8,
  SP_ATTR_CLEAN_DEFAULT_WARN   = 16,
  SP_ATTR_CLEAN_DEFAULT_REMOVE = 32
};

/**
 * Get preferences
 */
unsigned int sp_attribute_clean_get_prefs();

/**
 * Remove or warn about inappropriate attributes and useless style properties.
 * repr: the root node in a document or any other node.
 */
void sp_attribute_clean_tree(Node *repr);

/**
 * Recursively clean.
 * repr: the root node in a document or any other node.
 * pref_attr, pref_style, pref_defaults: ignore, delete, or warn.
 */
void sp_attribute_clean_recursive(Node *repr, unsigned int flags);

/**
 * Clean one element (attributes and style properties).
 */
void sp_attribute_clean_element(Node *repr, unsigned int flags);

/**
 * Clean style properties for one element.
 */
void sp_attribute_clean_style(Node *repr, unsigned int flags);

/**
 * Clean style properties for one style string.
 */
Glib::ustring sp_attribute_clean_style(Node *repr, gchar const *string, unsigned int flags);

/**
 * Clean style properties for one CSS.
 */
void sp_attribute_clean_style(Node* repr, SPCSSAttr *css, unsigned int flags);

/**
 * Check one attribute on an element
 */
bool sp_attribute_check_attribute(Glib::ustring element, Glib::ustring id, Glib::ustring attribute, bool warn);

#endif /* __SP_ATTRIBUTE_REL_UTIL_H__ */

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

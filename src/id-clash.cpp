#define __ID_CLASH_C__
/** \file
 * Routines for resolving ID clashes when importing or pasting.
 *
 * Authors:
 *   Stephen Silver <sasilver@users.sourceforge.net>
 *
 * Copyright (C) 2008 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstdlib>
#include <cstring>
#include <list>
#include <map>
#include <string>
#include <utility>

#include "extract-uri.h"
#include "id-clash.h"
#include "sp-object.h"
#include "style.h"
#include "xml/node.h"
#include "xml/repr.h"

typedef enum { REF_HREF, REF_STYLE, REF_URL, REF_CLIPBOARD } ID_REF_TYPE;

struct IdReference {
    ID_REF_TYPE type;
    SPObject *elem;
    const char *attr;  // property or href-like attribute
};

typedef std::map<std::string, std::list<IdReference> > refmap_type;

typedef std::pair<SPObject*, std::string> id_changeitem_type;
typedef std::list<id_changeitem_type> id_changelist_type;

const char *href_like_attributes[] = {
    "inkscape:connection-end",
    "inkscape:connection-start",
    "inkscape:href",
    "inkscape:path-effect",
    "inkscape:perspectiveID",
    "inkscape:tiled-clone-of",
    "xlink:href",
};
#define NUM_HREF_LIKE_ATTRIBUTES (sizeof(href_like_attributes) / sizeof(*href_like_attributes))

const SPIPaint SPStyle::* SPIPaint_members[] = {
    &SPStyle::color,
    &SPStyle::fill,
    &SPStyle::stroke,
};
const char* SPIPaint_properties[] = {
    "color",
    "fill",
    "stroke",
};
#define NUM_SPIPAINT_PROPERTIES (sizeof(SPIPaint_properties) / sizeof(*SPIPaint_properties))

const char* other_url_properties[] = {
    "clip-path",
    "color-profile",
    "cursor",
    "marker-end",
    "marker-mid",
    "marker-start",
    "mask",
};
#define NUM_OTHER_URL_PROPERTIES (sizeof(other_url_properties) / sizeof(*other_url_properties))

const char* clipboard_properties[] = {
    "color",
    "fill",
    "filter",
    "stroke",
};
#define NUM_CLIPBOARD_PROPERTIES (sizeof(clipboard_properties) / sizeof(*clipboard_properties))

/**
 *  Build a table of places where IDs are referenced, for a given element.
 *  FIXME: There are some types of references not yet dealt with here
 *         (e.g., ID selectors in CSS stylesheets, and references in scripts).
 */
static void
find_references(SPObject *elem, refmap_type *refmap)
{
    if (SP_OBJECT_IS_CLONED(elem)) return;
    Inkscape::XML::Node *repr_elem = SP_OBJECT_REPR(elem);
    if (!repr_elem) return;
    if (repr_elem->type() != Inkscape::XML::ELEMENT_NODE) return;

    /* check for references in inkscape:clipboard elements */
    if (!std::strcmp(repr_elem->name(), "inkscape:clipboard")) {
        SPCSSAttr *css = sp_repr_css_attr(repr_elem, "style");
        if (css) {
            for (unsigned i = 0; i < NUM_CLIPBOARD_PROPERTIES; ++i) {
                const char *attr = clipboard_properties[i];
                const gchar *value = sp_repr_css_property(css, attr, NULL);
                if (value) {
                    gchar *uri = extract_uri(value);
                    if (uri && uri[0] == '#') {
                        IdReference idref = { REF_CLIPBOARD, elem, attr };
                        (*refmap)[uri+1].push_back(idref);
                    }
                    g_free(uri);
                }
            }
        }
        return; // nothing more to do for inkscape:clipboard elements
    }

    /* check for xlink:href="#..." and similar */
    for (unsigned i = 0; i < NUM_HREF_LIKE_ATTRIBUTES; ++i) {
        const char *attr = href_like_attributes[i];
        const gchar *val = repr_elem->attribute(attr);
        if (val && val[0] == '#') {
            std::string id(val+1);
            IdReference idref = { REF_HREF, elem, attr };
            (*refmap)[id].push_back(idref);
        }
    }

    SPStyle *style = SP_OBJECT_STYLE(elem);

    /* check for url(#...) references in 'fill' or 'stroke' */
    for (unsigned i = 0; i < NUM_SPIPAINT_PROPERTIES; ++i) {
        const SPIPaint SPStyle::*prop = SPIPaint_members[i];
        const SPIPaint *paint = &(style->*prop);
        if (paint->isPaintserver() && paint->value.href) {
            const SPObject *obj = paint->value.href->getObject();
            if (obj) {
                const gchar *id = obj->getId();
                IdReference idref = { REF_STYLE, elem, SPIPaint_properties[i] };
                (*refmap)[id].push_back(idref);
            }
        }
    }

    /* check for url(#...) references in 'filter' */
    const SPIFilter *filter = &(style->filter);
    if (filter->href) {
        const SPObject *obj = filter->href->getObject();
        if (obj) {
            const gchar *id = obj->getId();
            IdReference idref = { REF_STYLE, elem, "filter" };
            (*refmap)[id].push_back(idref);
        }
    }

    /* check for other url(#...) references */
    for (unsigned i = 0; i < NUM_OTHER_URL_PROPERTIES; ++i) {
        const char *attr = other_url_properties[i];
        const gchar *value = repr_elem->attribute(attr);
        if (value) {
            gchar *uri = extract_uri(value);
            if (uri && uri[0] == '#') {
                IdReference idref = { REF_URL, elem, attr };
                (*refmap)[uri+1].push_back(idref);
            }
            g_free(uri);
        }
    }
    
    /* recurse */
    for (SPObject *child = sp_object_first_child(elem);
         child; child = SP_OBJECT_NEXT(child) )
    {
        find_references(child, refmap);
    }
}

/**
 *  Change any IDs that clash with IDs in the current document, and make
 *  a list of those changes that will require fixing up references.
 */
static void
change_clashing_ids(SPDocument *imported_doc, SPDocument *current_doc,
                    SPObject *elem, const refmap_type *refmap,
                    id_changelist_type *id_changes)
{
    const gchar *id = elem->getId();

    if (id && current_doc->getObjectById(id)) {
        // Choose a new ID.
        // To try to preserve any meaningfulness that the original ID
        // may have had, the new ID is the old ID followed by a hyphen
        // and one or more digits.
        std::string old_id(id);
        std::string new_id(old_id + '-');
        for (;;) {
            new_id += "0123456789"[std::rand() % 10];
            const char *str = new_id.c_str();
            if (current_doc->getObjectById(str) == NULL &&
                imported_doc->getObjectById(str) == NULL) break;
        }
        // Change to the new ID
        SP_OBJECT_REPR(elem)->setAttribute("id", new_id.c_str());
        // Make a note of this change, if we need to fix up refs to it
        if (refmap->find(old_id) != refmap->end())
            id_changes->push_back(id_changeitem_type(elem, old_id));
    }

    /* recurse */
    for (SPObject *child = sp_object_first_child(elem);
         child; child = SP_OBJECT_NEXT(child) )
    {
        change_clashing_ids(imported_doc, current_doc, child, refmap, id_changes);
    }
}

/**
 *  Fix up references to changed IDs.
 */
static void
fix_up_refs(const refmap_type *refmap, const id_changelist_type &id_changes)
{
    id_changelist_type::const_iterator pp;
    const id_changelist_type::const_iterator pp_end = id_changes.end();
    for (pp = id_changes.begin(); pp != pp_end; ++pp) {
        SPObject *obj = pp->first;
        refmap_type::const_iterator pos = refmap->find(pp->second);
        std::list<IdReference>::const_iterator it;
        const std::list<IdReference>::const_iterator it_end = pos->second.end();
        for (it = pos->second.begin(); it != it_end; ++it) {
            if (it->type == REF_HREF) {
                gchar *new_uri = g_strdup_printf("#%s", obj->getId());
                SP_OBJECT_REPR(it->elem)->setAttribute(it->attr, new_uri);
                g_free(new_uri);
            }
            else if (it->type == REF_STYLE) {
                sp_style_set_property_url(it->elem, it->attr, obj, false);
            }
            else if (it->type == REF_URL) {
                gchar *url = g_strdup_printf("url(#%s)", obj->getId());
                SP_OBJECT_REPR(it->elem)->setAttribute(it->attr, url);
                g_free(url);
            }
            else if (it->type == REF_CLIPBOARD) {
                SPCSSAttr *style = sp_repr_css_attr(SP_OBJECT_REPR(it->elem), "style");
                gchar *url = g_strdup_printf("url(#%s)", obj->getId());
                sp_repr_css_set_property(style, it->attr, url);
                g_free(url);
                gchar *style_string = sp_repr_css_write_string(style);
                SP_OBJECT_REPR(it->elem)->setAttribute("style", style_string);
                g_free(style_string);
            }
            else g_assert(0); // shouldn't happen
        }
    }
}

/**
 *  This function resolves ID clashes between the document being imported
 *  and the current open document: IDs in the imported document that would
 *  clash with IDs in the existing document are changed, and references to
 *  those IDs are updated accordingly.
 */
void
prevent_id_clashes(SPDocument *imported_doc, SPDocument *current_doc)
{
    refmap_type *refmap = new refmap_type;
    id_changelist_type id_changes;
    SPObject *imported_root = SP_DOCUMENT_ROOT(imported_doc);
        
    find_references(imported_root, refmap);
    change_clashing_ids(imported_doc, current_doc, imported_root, refmap,
                        &id_changes);
    fix_up_refs(refmap, id_changes);

    delete refmap;
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

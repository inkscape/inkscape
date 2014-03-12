/*
 * Routines for resolving ID clashes when importing or pasting.
 *
 * Authors:
 *   Stephen Silver <sasilver@users.sourceforge.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
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
#include "sp-paint-server.h"
#include "xml/node.h"
#include "xml/repr.h"
#include "sp-root.h"
#include "sp-gradient.h"

typedef enum { REF_HREF, REF_STYLE, REF_URL, REF_CLIPBOARD } ID_REF_TYPE;

struct IdReference {
    ID_REF_TYPE type;
    SPObject *elem;
    const char *attr;  // property or href-like attribute
};

typedef std::map<Glib::ustring, std::list<IdReference> > refmap_type;

typedef std::pair<SPObject*, Glib::ustring> id_changeitem_type;
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
    "marker-end",
    "marker-mid",
    "marker-start"
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
    if (elem->cloned) return;
    Inkscape::XML::Node *repr_elem = elem->getRepr();
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

    SPStyle *style = elem->style;

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

    /* check for url(#...) references in markers */
    const gchar *markers[4] = { "", "marker-start", "marker-mid", "marker-end" };
    for (unsigned i = SP_MARKER_LOC_START; i < SP_MARKER_LOC_QTY; i++) {
        const gchar *value = style->marker[i].value;
        if (value) {
            gchar *uri = extract_uri(value);
            if (uri && uri[0] == '#') {
                IdReference idref = { REF_STYLE, elem, markers[i] };
                (*refmap)[uri+1].push_back(idref);
            }
            g_free(uri);
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
    
    // recurse
    for (SPObject *child = elem->firstChild(); child; child = child->getNext() )
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
    bool fix_clashing_ids = true;

    if (id && current_doc->getObjectById(id)) {
        // Choose a new ID.
        // To try to preserve any meaningfulness that the original ID
        // may have had, the new ID is the old ID followed by a hyphen
        // and one or more digits.
        
        if (SP_IS_GRADIENT(elem)) {
            SPObject *cd_obj =  current_doc->getObjectById(id);

            if (cd_obj && SP_IS_GRADIENT(cd_obj)) {
                SPGradient *cd_gr = SP_GRADIENT(cd_obj);
                if ( cd_gr->isEquivalent(SP_GRADIENT(elem)) &&
                    cd_gr->isAligned(SP_GRADIENT(elem))) {
                    fix_clashing_ids = false;
                 }
             }
        }
        
        if (fix_clashing_ids) {
            std::string old_id(id);
            std::string new_id(old_id + '-');
            for (;;) {
                new_id += "0123456789"[std::rand() % 10];
                const char *str = new_id.c_str();
                if (current_doc->getObjectById(str) == NULL &&
                    imported_doc->getObjectById(str) == NULL) break;
            }
            // Change to the new ID

            elem->getRepr()->setAttribute("id", new_id.c_str());
                // Make a note of this change, if we need to fix up refs to it
            if (refmap->find(old_id) != refmap->end())
            id_changes->push_back(id_changeitem_type(elem, old_id));
        }
    }


    // recurse
    for (SPObject *child = elem->firstChild(); child; child = child->getNext() )
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
                it->elem->getRepr()->setAttribute(it->attr, new_uri);
                g_free(new_uri);
            } else if (it->type == REF_STYLE) {
                sp_style_set_property_url(it->elem, it->attr, obj, false);
            } else if (it->type == REF_URL) {
                gchar *url = g_strdup_printf("url(#%s)", obj->getId());
                it->elem->getRepr()->setAttribute(it->attr, url);
                g_free(url);
            } else if (it->type == REF_CLIPBOARD) {
                SPCSSAttr *style = sp_repr_css_attr(it->elem->getRepr(), "style");
                gchar *url = g_strdup_printf("url(#%s)", obj->getId());
                sp_repr_css_set_property(style, it->attr, url);
                g_free(url);
                Glib::ustring style_string;
                sp_repr_css_write_string(style, style_string);
                it->elem->getRepr()->setAttribute("style", style_string.c_str());
            } else {
                g_assert(0); // shouldn't happen
            }
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
    SPObject *imported_root = imported_doc->getRoot();
        
    find_references(imported_root, refmap);
    change_clashing_ids(imported_doc, current_doc, imported_root, refmap,
                        &id_changes);
    fix_up_refs(refmap, id_changes);

    delete refmap;
}

/*
 * Change any references of svg:def from_obj into to_obj
 */
void
change_def_references(SPObject *from_obj, SPObject *to_obj)
{
    refmap_type *refmap = new refmap_type;
    SPDocument *current_doc = from_obj->document;
    std::string old_id(from_obj->getId());

    find_references(current_doc->getRoot(), refmap);

    refmap_type::const_iterator pos = refmap->find(old_id);
    if (pos != refmap->end()) {
        std::list<IdReference>::const_iterator it;
        const std::list<IdReference>::const_iterator it_end = pos->second.end();
        for (it = pos->second.begin(); it != it_end; ++it) {
            if (it->type == REF_STYLE) {
                sp_style_set_property_url(it->elem, it->attr, to_obj, false);
            }
        }
    }

    delete refmap;
}

/*
 * Change the id of a SPObject to new_name
 * If there is an id clash then rename to something similar
 */
void rename_id(SPObject *elem, Glib::ustring const &new_name)
{
    if (new_name.empty()){
        g_message("Invalid Id, will not change.");
        return;
    }
    gchar *id = g_strdup(new_name.c_str()); //id is not empty here as new_name is check to be not empty
    g_strcanon (id, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.:", '_');
    Glib::ustring new_name2 = id; //will not fail as id can not be NULL, see length check on new_name
    g_free (id);
    if (!isalnum (new_name2[0])) {
        g_message("Invalid Id, will not change.");
        return;
    }

    SPDocument *current_doc = elem->document;
    refmap_type *refmap = new refmap_type;
    id_changelist_type id_changes;
    find_references(current_doc->getRoot(), refmap);

    std::string old_id(elem->getId());
    if (current_doc->getObjectById(id)) {
        // Choose a new ID.
        // To try to preserve any meaningfulness that the original ID
        // may have had, the new ID is the old ID followed by a hyphen
        // and one or more digits.
        new_name2 += '-';
        for (;;) {
            new_name2 += "0123456789"[std::rand() % 10];
            if (current_doc->getObjectById(new_name2) == NULL)
                break;
        }
    }

    // Change to the new ID
    elem->getRepr()->setAttribute("id", new_name2.c_str());
    // Make a note of this change, if we need to fix up refs to it
    if (refmap->find(old_id) != refmap->end()) {
        id_changes.push_back(id_changeitem_type(elem, old_id));
    }

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

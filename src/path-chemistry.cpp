#define __SP_PATH_CHEMISTRY_C__

/*
 * Here are handlers for modifying selections, specific to paths
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *
 * Copyright (C) 1999-2008 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <cstring>
#include <string>
#include "xml/repr.h"
#include "svg/svg.h"
#include "display/curve.h"
#include <glib/gmem.h>
#include <glibmm/i18n.h>
#include "sp-path.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "libnr/nr-path.h"
#include "text-editing.h"
#include "style.h"
#include "inkscape.h"
#include "desktop.h"
#include "document.h"
#include "message-stack.h"
#include "selection.h"
#include "desktop-handles.h"
#include "box3d.h"
#include <2geom/pathvector.h>
#include "path-chemistry.h"

/* Helper functions for sp_selected_path_to_curves */
static void sp_selected_path_to_curves0(gboolean do_document_done, guint32 text_grouping_policy);
static bool sp_item_list_to_curves(const GSList *items, GSList **selected, GSList **to_select);

enum {
    /* Not used yet. This is the placeholder of Lauris's idea. */
    SP_TOCURVE_INTERACTIVE       = 1 << 0,
    SP_TOCURVE_GROUPING_BY_WORD  = 1 << 1,
    SP_TOCURVE_GROUPING_BY_LINE  = 1 << 2,
    SP_TOCURVE_GROUPING_BY_WHOLE = 1 << 3
};

void
sp_selected_path_combine(void)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    SPDocument *doc = sp_desktop_document(desktop);
    
    if (g_slist_length((GSList *) selection->itemList()) < 2) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select <b>at least two objects</b> to combine."));
        return;
    }

    desktop->messageStack()->flash(Inkscape::IMMEDIATE_MESSAGE, _("Combining paths..."));
    // set "busy" cursor
    desktop->setWaitingCursor();

    GSList *items = g_slist_copy((GSList *) selection->itemList());
    GSList *to_paths = NULL;
    for (GSList *i = items; i != NULL; i = i->next) {
        SPItem *item = (SPItem *) i->data;
        if (!SP_IS_PATH(item))
            to_paths = g_slist_prepend(to_paths, item);
    }
    GSList *converted = NULL;
    bool did = sp_item_list_to_curves(to_paths, &items, &converted);
    g_slist_free(to_paths);
    for (GSList *i = converted; i != NULL; i = i->next)
        items = g_slist_prepend(items, doc->getObjectByRepr((Inkscape::XML::Node*)(i->data)));

    items = g_slist_sort(items, (GCompareFunc) sp_item_repr_compare_position);
    items = g_slist_reverse(items);

    // remember the position, id, transform and style of the topmost path, they will be assigned to the combined one
    gint position = 0;
    char const *id = NULL;
    char const *transform = NULL;
    gchar *style = NULL;
    gchar *path_effect = NULL;

    SPCurve* curve = 0;
    SPItem *first = NULL;
    Inkscape::XML::Node *parent = NULL; 

    for (GSList *i = items; i != NULL; i = i->next) {  // going from top to bottom

        SPItem *item = (SPItem *) i->data;
        if (!SP_IS_PATH(item))
            continue;
        did = true;

        SPCurve *c = sp_path_get_curve_for_edit(SP_PATH(item));
        if (first == NULL) {  // this is the topmost path
            first = item;
            parent = SP_OBJECT_REPR(first)->parent();
            position = SP_OBJECT_REPR(first)->position();
            id = SP_OBJECT_REPR(first)->attribute("id");
            transform = SP_OBJECT_REPR(first)->attribute("transform");
            // FIXME: merge styles of combined objects instead of using the first one's style
            style = g_strdup(SP_OBJECT_REPR(first)->attribute("style"));
            path_effect = g_strdup(SP_OBJECT_REPR(first)->attribute("inkscape:path-effect"));
            //c->transform(item->transform);
            curve = c;
        } else {
            c->transform(item->getRelativeTransform(SP_OBJECT(first)));
            curve->append(c, false);
            c->unref();
        }

        // unless this is the topmost object,
        if (item != first) {
            // reduce position only if the same parent
            if (SP_OBJECT_REPR(item)->parent() == parent)
                position--;
            // delete the object for real, so that its clones can take appropriate action
            SP_OBJECT(item)->deleteObject();
        }
    }

    g_slist_free(items);

    if (did) {
        selection->clear();

        // delete the topmost one so that its clones don't get alerted; this object will be
        // restored shortly, with the same id
        SP_OBJECT(first)->deleteObject(false);

        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(desktop->doc());
        Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");

        // restore id, transform, path effect, and style
        repr->setAttribute("id", id);
        if (transform) repr->setAttribute("transform", transform);
        repr->setAttribute("style", style);
        g_free(style);

        // set path data corresponding to new curve
        gchar *dstring = sp_svg_write_path(curve->get_pathvector());
        curve->unref();
        repr->setAttribute("d", dstring);
        if (path_effect)
            repr->setAttribute("inkscape:original-d", dstring);
        g_free(dstring);

        repr->setAttribute("inkscape:path-effect", path_effect);
        g_free(path_effect);

        // add the new group to the parent of the topmost
        parent->appendChild(repr);

        // move to the position of the topmost, reduced by the number of deleted items
        repr->setPosition(position > 0 ? position : 0);

        sp_document_done(sp_desktop_document(desktop), SP_VERB_SELECTION_COMBINE, 
                         _("Combine"));

        selection->set(repr);

        Inkscape::GC::release(repr);

    } else {
        sp_desktop_message_stack(desktop)->flash(Inkscape::ERROR_MESSAGE, _("<b>No path(s)</b> to combine in the selection."));
    }

    desktop->clearWaitingCursor();
}

void
sp_selected_path_break_apart(void)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty()) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select <b>path(s)</b> to break apart."));
        return;
    }

    desktop->messageStack()->flash(Inkscape::IMMEDIATE_MESSAGE, _("Breaking apart paths..."));
    // set "busy" cursor
    desktop->setWaitingCursor();

    bool did = false;

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        SPItem *item = (SPItem *) items->data;

        if (!SP_IS_PATH(item))
            continue;

        SPPath *path = SP_PATH(item);

        SPCurve *curve = sp_path_get_curve_for_edit(SP_PATH(path));
        if (curve == NULL)
            continue;

        did = true;

        Inkscape::XML::Node *parent = SP_OBJECT_REPR(item)->parent();
        gint pos = SP_OBJECT_REPR(item)->position();
        char const *id = SP_OBJECT_REPR(item)->attribute("id");

        gchar *style = g_strdup(SP_OBJECT(item)->repr->attribute("style"));
        gchar *path_effect = g_strdup(SP_OBJECT(item)->repr->attribute("inkscape:path-effect"));

        Geom::PathVector apv = curve->get_pathvector() * to_2geom(SP_ITEM(path)->transform);

        curve->unref();

        // it's going to resurrect as one of the pieces, so we delete without advertisement
        SP_OBJECT(item)->deleteObject(false);

        curve = new SPCurve(apv);
        g_assert(curve != NULL);

        GSList *list = curve->split();

        curve->unref();

        GSList *reprs = NULL;
        for (GSList *l = list; l != NULL; l = l->next) {
            curve = (SPCurve *) l->data;

            Inkscape::XML::Node *repr = parent->document()->createElement("svg:path");
            repr->setAttribute("style", style);

            gchar *str = sp_svg_write_path(curve->get_pathvector());
            repr->setAttribute("d", str);
            if (path_effect)
                repr->setAttribute("inkscape:original-d", str);
            g_free(str);

            repr->setAttribute("inkscape:path-effect", path_effect);

            // add the new repr to the parent
            parent->appendChild(repr);

            // move to the saved position
            repr->setPosition(pos > 0 ? pos : 0);

            // if it's the first one, restore id
            if (l == list)
                repr->setAttribute("id", id);

            reprs = g_slist_prepend (reprs, repr);

            Inkscape::GC::release(repr);
        }

        selection->setReprList(reprs);

        g_slist_free(reprs);
        g_slist_free(list);
        g_free(style);

    }

    desktop->clearWaitingCursor();

    if (did) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_SELECTION_BREAK_APART, 
                         _("Break apart"));
    } else {
        sp_desktop_message_stack(desktop)->flash(Inkscape::ERROR_MESSAGE, _("<b>No path(s)</b> to break apart in the selection."));
    }
}

/* This function is an entry point from GUI */
void
sp_selected_path_to_curves(bool interactive)
{
    if (interactive) {
        sp_selected_path_to_curves0(TRUE, SP_TOCURVE_INTERACTIVE);
    } else {
        sp_selected_path_to_curves0(false, 0);
    }
}

static void
sp_selected_path_to_curves0(gboolean interactive, guint32 /*text_grouping_policy*/)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty()) {
        if (interactive)
            sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to convert to path."));
        return;
    }

    bool did = false;
    if (interactive) {
        desktop->messageStack()->flash(Inkscape::IMMEDIATE_MESSAGE, _("Converting objects to paths..."));
        // set "busy" cursor
        desktop->setWaitingCursor();
    }

    GSList *selected = g_slist_copy((GSList *) selection->itemList());
    GSList *to_select = NULL;
    selection->clear();
    GSList *items = g_slist_copy(selected);

    did = sp_item_list_to_curves(items, &selected, &to_select);

    g_slist_free (items);
    selection->setReprList(to_select);
    selection->addList(selected);
    g_slist_free (to_select);
    g_slist_free (selected);

    if (interactive) {
        desktop->clearWaitingCursor();
        if (did) {
            sp_document_done(sp_desktop_document(desktop), SP_VERB_OBJECT_TO_CURVE, 
                             _("Object to path"));
        } else {
            sp_desktop_message_stack(desktop)->flash(Inkscape::ERROR_MESSAGE, _("<b>No objects</b> to convert to path in the selection."));
            return;
        }
    }
}

static bool
sp_item_list_to_curves(const GSList *items, GSList **selected, GSList **to_select)
{
    bool did = false;
    
    for (;
         items != NULL;
         items = items->next) {

        SPItem *item = SP_ITEM(items->data);

        if (SP_IS_PATH(item) && !SP_PATH(item)->original_curve) {
            continue; // already a path, and no path effect
        }

        if (SP_IS_BOX3D(item)) {
            // convert 3D box to ordinary group of paths; replace the old element in 'selected' with the new group
            Inkscape::XML::Node *repr = SP_OBJECT_REPR(box3d_convert_to_group(SP_BOX3D(item)));
            
            if (repr) {
                *to_select = g_slist_prepend (*to_select, repr);
                did = true;
                *selected = g_slist_remove (*selected, item);
            }

            continue;
        }
        
        if (SP_IS_GROUP(item)) {
            sp_lpe_item_remove_all_path_effects(SP_LPE_ITEM(item), true);
            GSList *item_list = sp_item_group_item_list(SP_GROUP(item));
            
            GSList *item_to_select = NULL;
            GSList *item_selected = NULL;
            
            if (sp_item_list_to_curves(item_list, &item_selected, &item_to_select))
                did = true;

            g_slist_free(item_list);
            g_slist_free(item_to_select);
            g_slist_free(item_selected);

            continue;
        }

        Inkscape::XML::Node *repr = sp_selected_item_to_curved_repr(item, 0);
        if (!repr)
            continue;

        did = true;
        *selected = g_slist_remove (*selected, item);

        // remember the position of the item
        gint pos = SP_OBJECT_REPR(item)->position();
        // remember parent
        Inkscape::XML::Node *parent = SP_OBJECT_REPR(item)->parent();
        // remember id
        char const *id = SP_OBJECT_REPR(item)->attribute("id");

        // It's going to resurrect, so we delete without notifying listeners.
        SP_OBJECT(item)->deleteObject(false);

        // restore id
        repr->setAttribute("id", id);
        // add the new repr to the parent
        parent->appendChild(repr);
        // move to the saved position
        repr->setPosition(pos > 0 ? pos : 0);

        /* Buglet: We don't re-add the (new version of the) object to the selection of any other
         * desktops where it was previously selected. */
        *to_select = g_slist_prepend (*to_select, repr);
        Inkscape::GC::release(repr);
    }
    
    return did;
}

Inkscape::XML::Node *
sp_selected_item_to_curved_repr(SPItem *item, guint32 /*text_grouping_policy*/)
{
    if (!item)
        return NULL;

    SPCurve *curve = NULL;
    if (SP_IS_SHAPE(item)) {
        curve = sp_shape_get_curve(SP_SHAPE(item));
    } else if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item)) {
        curve = te_get_layout(item)->convertToCurves();
    }

    if (!curve)
        return NULL;

    // Prevent empty paths from being added to the document
    // otherwise we end up with zomby markup in the SVG file
    if(curve->get_length() <= 0)
    {
        curve->unref();
        return NULL;
    }

    Inkscape::XML::Document *xml_doc = SP_OBJECT_REPR(item)->document();
    Inkscape::XML::Node *repr = xml_doc->createElement("svg:path");
    /* Transformation */
    repr->setAttribute("transform", SP_OBJECT_REPR(item)->attribute("transform"));
    /* Style */
    gchar *style_str = sp_style_write_difference(SP_OBJECT_STYLE(item),
                                                 SP_OBJECT_STYLE(SP_OBJECT_PARENT(item)));
    repr->setAttribute("style", style_str);
    g_free(style_str);

    /* Mask */
    gchar *mask_str = (gchar *) SP_OBJECT_REPR(item)->attribute("mask");
    if ( mask_str )
        repr->setAttribute("mask", mask_str);

    /* Clip path */
    gchar *clip_path_str = (gchar *) SP_OBJECT_REPR(item)->attribute("clip-path");
    if ( clip_path_str )
        repr->setAttribute("clip-path", clip_path_str);

    /* Rotation center */
    repr->setAttribute("inkscape:transform-center-x", SP_OBJECT_REPR(item)->attribute("inkscape:transform-center-x"), false);
    repr->setAttribute("inkscape:transform-center-y", SP_OBJECT_REPR(item)->attribute("inkscape:transform-center-y"), false);

    /* Definition */
    gchar *def_str = sp_svg_write_path(curve->get_pathvector());
    repr->setAttribute("d", def_str);
    g_free(def_str);
    curve->unref();
    return repr;
}


// FIXME: THIS DOES NOT REVERSE THE NODETYPES ORDER!
void
sp_selected_path_reverse()
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    GSList *items = (GSList *) selection->itemList();

    if (!items) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select <b>path(s)</b> to reverse."));
        return;
    }


    // set "busy" cursor
    desktop->setWaitingCursor();

    bool did = false;
    desktop->messageStack()->flash(Inkscape::IMMEDIATE_MESSAGE, _("Reversing paths..."));

    for (GSList *i = items; i != NULL; i = i->next) {

        if (!SP_IS_PATH(i->data))
            continue;

        did = true;
        SPPath *path = SP_PATH(i->data);

        SPCurve *rcurve = sp_path_get_curve_reference(path)->create_reverse();

        gchar *str = sp_svg_write_path(rcurve->get_pathvector());
        if ( sp_lpe_item_has_path_effect_recursive(SP_LPE_ITEM(path)) ) {
            SP_OBJECT_REPR(path)->setAttribute("inkscape:original-d", str);
        } else {
            SP_OBJECT_REPR(path)->setAttribute("d", str);
        }
        g_free(str);

        rcurve->unref();
    }

    desktop->clearWaitingCursor();

    if (did) {
        sp_document_done(sp_desktop_document(desktop), SP_VERB_SELECTION_REVERSE,
                         _("Reverse path"));
    } else {
        sp_desktop_message_stack(desktop)->flash(Inkscape::ERROR_MESSAGE, _("<b>No paths</b> to reverse in the selection."));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

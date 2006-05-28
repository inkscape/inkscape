#define __SP_PATH_CHEMISTRY_C__

/*
 * Here are handlers for modifying selections, specific to paths
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2004 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
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
#include "document.h"
#include "message-stack.h"
#include "selection.h"
#include "desktop-handles.h"

/* Helper functions for sp_selected_path_to_curves */
static void sp_selected_path_to_curves0(gboolean do_document_done, guint32 text_grouping_policy);
static Inkscape::XML::Node *sp_selected_item_to_curved_repr(SPItem *item, guint32 text_grouping_policy);
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
    GSList *items = (GSList *) selection->itemList();

    if (g_slist_length(items) < 2) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select <b>at least two objects</b> to combine."));
        return;
    }

    for (GSList *i = items; i != NULL; i = i->next) {
        SPItem *item = (SPItem *) i->data;
        if (!SP_IS_SHAPE(item) && !SP_IS_TEXT(item)) {
            sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("At least one of the objects is <b>not a path</b>, cannot combine."));
            return;
        }
    }

    Inkscape::XML::Node *parent = SP_OBJECT_REPR((SPItem *) items->data)->parent();
    for (GSList *i = items; i != NULL; i = i->next) {
        if ( SP_OBJECT_REPR((SPItem *) i->data)->parent() != parent ) {
            sp_desktop_message_stack(desktop)->flash(Inkscape::ERROR_MESSAGE, _("You cannot combine objects from <b>different groups</b> or <b>layers</b>."));
            return;
        }
    }

    sp_selected_path_to_curves0(FALSE, 0);

    items = (GSList *) selection->itemList();

    items = g_slist_copy(items);

    items = g_slist_sort(items, (GCompareFunc) sp_item_repr_compare_position);

    // remember the position of the topmost object
    gint topmost = (SP_OBJECT_REPR((SPItem *) g_slist_last(items)->data))->position();

    // remember the id of the bottomost object
    char const *id = SP_OBJECT_REPR((SPItem *) items->data)->attribute("id");

    // FIXME: merge styles of combined objects instead of using the first one's style
    gchar *style = g_strdup(SP_OBJECT_REPR((SPItem *) items->data)->attribute("style"));

    GString *dstring = g_string_new("");
    for (GSList *i = items; i != NULL; i = i->next) {

        SPPath *path = (SPPath *) i->data;
        SPCurve *c = sp_shape_get_curve(SP_SHAPE(path));

        NArtBpath *abp = nr_artpath_affine(SP_CURVE_BPATH(c), SP_ITEM(path)->transform);
        sp_curve_unref(c);
        gchar *str = sp_svg_write_path(abp);
        g_free(abp);

        dstring = g_string_append(dstring, str);
        g_free(str);

        // if this is the bottommost object,
        if (!strcmp(SP_OBJECT_REPR(path)->attribute("id"), id)) {
            // delete it so that its clones don't get alerted; this object will be restored shortly, with the same id
            SP_OBJECT(path)->deleteObject(false);
        } else {
            // delete the object for real, so that its clones can take appropriate action
            SP_OBJECT(path)->deleteObject();
        }

        topmost--;
    }

    g_slist_free(items);

    Inkscape::XML::Node *repr = sp_repr_new("svg:path");

    // restore id
    repr->setAttribute("id", id);

    repr->setAttribute("style", style);
    g_free(style);

    repr->setAttribute("d", dstring->str);
    g_string_free(dstring, TRUE);

    // add the new group to the group members' common parent
    parent->appendChild(repr);

    // move to the position of the topmost, reduced by the number of deleted items
    repr->setPosition(topmost > 0 ? topmost + 1 : 0);

    sp_document_done(sp_desktop_document(desktop));

    selection->set(repr);

    Inkscape::GC::release(repr);
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

    bool did = false;

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        SPItem *item = (SPItem *) items->data;

        if (!SP_IS_PATH(item))
            continue;

        SPPath *path = SP_PATH(item);

        SPCurve *curve = sp_shape_get_curve(SP_SHAPE(path));
        if (curve == NULL)
            continue;

        did = true;

        Inkscape::XML::Node *parent = SP_OBJECT_REPR(item)->parent();
        gint pos = SP_OBJECT_REPR(item)->position();
        char const *id = SP_OBJECT_REPR(item)->attribute("id");

        gchar *style = g_strdup(SP_OBJECT(item)->repr->attribute("style"));

        NArtBpath *abp = nr_artpath_affine(SP_CURVE_BPATH(curve), (SP_ITEM(path))->transform);

        sp_curve_unref(curve);

        // it's going to resurrect as one of the pieces, so we delete without advertisement
        SP_OBJECT(item)->deleteObject(false);

        curve = sp_curve_new_from_bpath(abp);
        g_assert(curve != NULL);

        GSList *list = sp_curve_split(curve);

        sp_curve_unref(curve);

        GSList *reprs = NULL;
        for (GSList *l = list; l != NULL; l = l->next) {
            curve = (SPCurve *) l->data;

            Inkscape::XML::Node *repr = sp_repr_new("svg:path");
            repr->setAttribute("style", style);

            gchar *str = sp_svg_write_path(SP_CURVE_BPATH(curve));
            repr->setAttribute("d", str);
            g_free(str);

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

    if (did) {
        sp_document_done(sp_desktop_document(desktop));
    } else {
        sp_desktop_message_stack(desktop)->flash(Inkscape::ERROR_MESSAGE, _("<b>No path(s)</b> to break apart in the selection."));
        return;
    }
}

/* This function is an entry point from GUI */
void
sp_selected_path_to_curves(void)
{
    sp_selected_path_to_curves0(TRUE, SP_TOCURVE_INTERACTIVE);
}

static void
sp_selected_path_to_curves0(gboolean interactive, guint32 text_grouping_policy)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    if (selection->isEmpty()) {
        if (interactive)
            sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select <b>object(s)</b> to convert to path."));
        return;
    }

    bool did = false;

    for (GSList *items = g_slist_copy((GSList *) selection->itemList());
         items != NULL;
         items = items->next) {

        SPItem *item = SP_ITEM(items->data);

        Inkscape::XML::Node *repr = sp_selected_item_to_curved_repr(item, 0);
        if (!repr)
            continue;

        did = true;

        // remember the position of the item
        gint pos = SP_OBJECT_REPR(item)->position();
        // remember parent
        Inkscape::XML::Node *parent = SP_OBJECT_REPR(item)->parent();
        // remember id
        char const *id = SP_OBJECT_REPR(item)->attribute("id");

        selection->remove(item);

        // it's going to resurrect, so we delete without advertisement
        SP_OBJECT(item)->deleteObject(false);

        // restore id
        repr->setAttribute("id", id);
        // add the new repr to the parent
        parent->appendChild(repr);
        // move to the saved position
        repr->setPosition(pos > 0 ? pos : 0);

        selection->add(repr);
        Inkscape::GC::release(repr);
    }

    if (interactive) {
        if (did) {
            sp_document_done(sp_desktop_document(desktop));
        } else {
            sp_desktop_message_stack(desktop)->flash(Inkscape::ERROR_MESSAGE, _("<b>No objects</b> to convert to path in the selection."));
            return;
        }
    }
}

static Inkscape::XML::Node *
sp_selected_item_to_curved_repr(SPItem *item, guint32 text_grouping_policy)
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

    Inkscape::XML::Node *repr = sp_repr_new("svg:path");
    /* Transformation */
    repr->setAttribute("transform", SP_OBJECT_REPR(item)->attribute("transform"));
    /* Style */
    gchar *style_str = sp_style_write_difference(SP_OBJECT_STYLE(item),
                                                 SP_OBJECT_STYLE(SP_OBJECT_PARENT(item)));
    repr->setAttribute("style", style_str);
    g_free(style_str);
    /* Rotation center */
    sp_repr_set_attr(repr, "inkscape:transform-center-x", SP_OBJECT_REPR(item)->attribute("inkscape:transform-center-x"));
    sp_repr_set_attr(repr, "inkscape:transform-center-y", SP_OBJECT_REPR(item)->attribute("inkscape:transform-center-y"));

    /* Definition */
    gchar *def_str = sp_svg_write_path(SP_CURVE_BPATH(curve));
    repr->setAttribute("d", def_str);
    g_free(def_str);
    sp_curve_unref(curve);
    return repr;
}

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


    bool did = false;
    for (GSList *i = items; i != NULL; i = i->next) {

        if (!SP_IS_SHAPE(i->data))
            continue;

        did = true;
        SPShape *shape = SP_SHAPE(i->data);

        SPCurve *rcurve = sp_curve_reverse(shape->curve);

        gchar *str = sp_svg_write_path(SP_CURVE_BPATH(rcurve));
        SP_OBJECT_REPR(shape)->setAttribute("d", str);
        g_free(str);

        sp_curve_unref(rcurve);
    }

    if (did) {
        sp_document_done(sp_desktop_document(desktop));
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

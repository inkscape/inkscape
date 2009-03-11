/*
 * LPEToolContext: a context for a generic tool composed of subtools that are given by LPEs
 *
 * Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2008 Maximilian Albert
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <2geom/sbasis-geometric.h>
#include <gdk/gdkkeysyms.h>

#include "macros.h"
#include "forward.h"
#include "pixmaps/cursor-node.xpm"
#include "pixmaps/cursor-crosshairs.xpm"
#include <gtk/gtk.h>
#include "desktop.h"
#include "message-context.h"
#include "preferences.h"
#include "shape-editor.h"
#include "selection.h"
#include "desktop-handles.h"
#include "document.h"
#include "display/curve.h"
#include "display/canvas-bpath.h"
#include "display/canvas-text.h"
#include "message-stack.h"
#include "sp-path.h"
#include "helper/units.h"

#include "lpe-tool-context.h"

static void sp_lpetool_context_class_init(SPLPEToolContextClass *klass);
static void sp_lpetool_context_init(SPLPEToolContext *erc);
static void sp_lpetool_context_dispose(GObject *object);

static void sp_lpetool_context_setup(SPEventContext *ec);
static void sp_lpetool_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *);
static gint sp_lpetool_context_item_handler(SPEventContext *ec, SPItem *item, GdkEvent *event);
static gint sp_lpetool_context_root_handler(SPEventContext *ec, GdkEvent *event);

void sp_lpetool_context_selection_changed(Inkscape::Selection *selection, gpointer data);

const int num_subtools = 8;

Inkscape::LivePathEffect::EffectType lpesubtools[] = {
    Inkscape::LivePathEffect::INVALID_LPE, // this must be here to account for the "all inactive" action
    Inkscape::LivePathEffect::LINE_SEGMENT,
    Inkscape::LivePathEffect::CIRCLE_3PTS,
    Inkscape::LivePathEffect::CIRCLE_WITH_RADIUS,
    Inkscape::LivePathEffect::PARALLEL,
    Inkscape::LivePathEffect::PERP_BISECTOR,
    Inkscape::LivePathEffect::ANGLE_BISECTOR,
    Inkscape::LivePathEffect::MIRROR_SYMMETRY,
};

static SPPenContextClass *lpetool_parent_class = 0;

GType sp_lpetool_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPLPEToolContextClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_lpetool_context_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPLPEToolContext),
            0, // n_preallocs
            (GInstanceInitFunc)sp_lpetool_context_init,
            0 // value_table
        };
        type = g_type_register_static(SP_TYPE_PEN_CONTEXT, "SPLPEToolContext", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

static void
sp_lpetool_context_class_init(SPLPEToolContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    lpetool_parent_class = (SPPenContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_lpetool_context_dispose;

    event_context_class->setup = sp_lpetool_context_setup;
    event_context_class->set = sp_lpetool_context_set;
    event_context_class->root_handler = sp_lpetool_context_root_handler;
    event_context_class->item_handler = sp_lpetool_context_item_handler;
}

static void
sp_lpetool_context_init(SPLPEToolContext *lc)
{
    lc->cursor_shape = cursor_crosshairs_xpm;
    lc->hot_x = 7;
    lc->hot_y = 7;

    lc->canvas_bbox = NULL;
    lc->measuring_items = new std::map<SPPath *, SPCanvasItem*>;

    new (&lc->sel_changed_connection) sigc::connection();
}

static void
sp_lpetool_context_dispose(GObject *object)
{
    SPLPEToolContext *lc = SP_LPETOOL_CONTEXT(object);
    delete lc->shape_editor;

    if (lc->canvas_bbox) {
        gtk_object_destroy(GTK_OBJECT(lc->canvas_bbox));
        lc->canvas_bbox = NULL;
    }

    lpetool_delete_measuring_items(lc);
    delete lc->measuring_items;
    lc->measuring_items = NULL;

    lc->sel_changed_connection.disconnect();
    lc->sel_changed_connection.~connection();

    if (lc->_lpetool_message_context) {
        delete lc->_lpetool_message_context;
    }

    G_OBJECT_CLASS(lpetool_parent_class)->dispose(object);
}

static void
sp_lpetool_context_setup(SPEventContext *ec)
{
    SPLPEToolContext *lc = SP_LPETOOL_CONTEXT(ec);

    if (((SPEventContextClass *) lpetool_parent_class)->setup)
        ((SPEventContextClass *) lpetool_parent_class)->setup(ec);

    Inkscape::Selection *selection = sp_desktop_selection (ec->desktop);
    SPItem *item = selection->singleItem();

    lc->sel_changed_connection.disconnect();
    lc->sel_changed_connection =
        selection->connectChanged(sigc::bind(sigc::ptr_fun(&sp_lpetool_context_selection_changed), (gpointer)lc));

    lc->shape_editor = new ShapeEditor(ec->desktop);

    lpetool_context_switch_mode(lc, Inkscape::LivePathEffect::INVALID_LPE);
    lpetool_context_reset_limiting_bbox(lc);
    lpetool_create_measuring_items(lc);

// TODO temp force:
    ec->enableSelectionCue();
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (item) {
        lc->shape_editor->set_item(item, SH_NODEPATH);
        lc->shape_editor->set_item(item, SH_KNOTHOLDER);
    }

    if (prefs->getBool("/tools/lpetool/selcue")) {
        ec->enableSelectionCue();
    }

    lc->_lpetool_message_context = new Inkscape::MessageContext((ec->desktop)->messageStack());


    lc->shape_editor->update_statusbar();
}

/**
\brief  Callback that processes the "changed" signal on the selection;
destroys old and creates new nodepath and reassigns listeners to the new selected item's repr
*/
void
sp_lpetool_context_selection_changed(Inkscape::Selection *selection, gpointer data)
{
    SPLPEToolContext *lc = SP_LPETOOL_CONTEXT(data);

    // TODO: update ShapeEditorsCollective instead
    lc->shape_editor->unset_item(SH_NODEPATH);
    lc->shape_editor->unset_item(SH_KNOTHOLDER);
    SPItem *item = selection->singleItem();
    lc->shape_editor->set_item(item, SH_NODEPATH);
    lc->shape_editor->set_item(item, SH_KNOTHOLDER);
    lc->shape_editor->update_statusbar();
}

static void
sp_lpetool_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *val)
{
    if (val->getEntryName() == "mode") {
        Inkscape::Preferences::get()->setString("/tools/geometric/mode", "drag");
        SP_PEN_CONTEXT(ec)->mode = SP_PEN_CONTEXT_MODE_DRAG;
    }

    /*
    //pass on up to parent class to handle common attributes.
    if ( lpetool_parent_class->set ) {
        lpetool_parent_class->set(ec, key, val);
    }
    */
}

static gint 
sp_lpetool_context_item_handler(SPEventContext *ec, SPItem *item, GdkEvent *event)
{
    gint ret = FALSE;

    switch (event->type) {
        case GDK_BUTTON_PRESS:
        {
            // select the clicked item but do nothing else
            Inkscape::Selection * const selection = sp_desktop_selection(ec->desktop);
            selection->clear();
            selection->add(item);
            ret = TRUE;
            break;
        }
        case GDK_BUTTON_RELEASE:
            // TODO: do we need to catch this or can we pass it on to the parent handler?
            ret = TRUE;
            break;
        default:
            break;
    }

    if (!ret) {
        if (((SPEventContextClass *) lpetool_parent_class)->item_handler)
            ret = ((SPEventContextClass *) lpetool_parent_class)->item_handler(ec, item, event);
    }

    return ret;
}

gint
sp_lpetool_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    SPLPEToolContext *lc = SP_LPETOOL_CONTEXT(event_context);
    SPDesktop *desktop = event_context->desktop;
    Inkscape::Selection *selection = sp_desktop_selection (desktop);

    bool ret = false;

    if (sp_pen_context_has_waiting_LPE(lc)) {
        // quit when we are waiting for a LPE to be applied
        ret = ((SPEventContextClass *) lpetool_parent_class)->root_handler(event_context, event);
        return ret;
    }

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {
                if (lc->mode == Inkscape::LivePathEffect::INVALID_LPE) {
                    // don't do anything for now if we are inactive (except clearing the selection
                    // since this was a click into empty space)
                    selection->clear();
                    desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Choose a construction tool from the toolbar."));
                    ret = true;
                    break;
                }

                // save drag origin
                event_context->xp = (gint) event->button.x;
                event_context->yp = (gint) event->button.y;
                event_context->within_tolerance = true;
                lc->shape_editor->cancel_hit();

                using namespace Inkscape::LivePathEffect;

                Inkscape::Preferences *prefs = Inkscape::Preferences::get();
                int mode = prefs->getInt("/tools/lpetool/mode");
                EffectType type = lpesubtools[mode];

                //bool over_stroke = lc->shape_editor->is_over_stroke(Geom::Point(event->button.x, event->button.y), true);

                sp_pen_context_wait_for_LPE_mouse_clicks(lc, type, Inkscape::LivePathEffect::Effect::acceptsNumClicks(type));

                // we pass the mouse click on to pen tool as the first click which it should collect
                ret = ((SPEventContextClass *) lpetool_parent_class)->root_handler(event_context, event);
            }
            break;
        case GDK_MOTION_NOTIFY:
        {
            if (!lc->shape_editor->has_nodepath() || selection->singleItem() == NULL) {
                break;
            }

            bool over_stroke = false;
            over_stroke = lc->shape_editor->is_over_stroke(Geom::Point(event->motion.x, event->motion.y), false);

            if (over_stroke) {
                event_context->cursor_shape = cursor_node_xpm;
                event_context->hot_x = 1;
                event_context->hot_y = 1;
                sp_event_context_update_cursor(event_context);
            } else {
                lc->cursor_shape = cursor_crosshairs_xpm;
                lc->hot_x = 7;
                lc->hot_y = 7;
                sp_event_context_update_cursor(event_context);
            }
        }
        break;


    case GDK_BUTTON_RELEASE:
    {
        /**
        break;
        **/
    }

    case GDK_KEY_PRESS:
        /**
        switch (get_group0_keyval (&event->key)) {
        }
        break;
        **/

    case GDK_KEY_RELEASE:
        /**
        switch (get_group0_keyval(&event->key)) {
            case GDK_Control_L:
            case GDK_Control_R:
                dc->_message_context->clear();
                break;
            default:
                break;
        }
        **/

    default:
        break;
    }

    if (!ret) {
        if (((SPEventContextClass *) lpetool_parent_class)->root_handler) {
            ret = ((SPEventContextClass *) lpetool_parent_class)->root_handler(event_context, event);
        }
    }

    return ret;
}

/*
 * Finds the index in the list of geometric subtools corresponding to the given LPE type.
 * Returns -1 if no subtool is found.
 */
int
lpetool_mode_to_index(Inkscape::LivePathEffect::EffectType const type) {
    for (int i = 0; i < num_subtools; ++i) {
        if (lpesubtools[i] == type) {
            return i;
        }
    }
    return -1;
}

/*
 * Checks whether an item has a construction applied as LPE and if so returns the index in
 * lpesubtools of this construction
 */
int lpetool_item_has_construction(SPLPEToolContext */*lc*/, SPItem *item)
{
    if (!SP_IS_LPE_ITEM(item)) {
        return -1;
    }

    Inkscape::LivePathEffect::Effect* lpe = sp_lpe_item_get_current_lpe(SP_LPE_ITEM(item));
    if (!lpe) {
        return -1;
    }
    return lpetool_mode_to_index(lpe->effectType());
}

/*
 * Attempts to perform the construction of the given type (i.e., to apply the corresponding LPE) to
 * a single selected item. Returns whether we succeeded.
 */
bool
lpetool_try_construction(SPLPEToolContext *lc, Inkscape::LivePathEffect::EffectType const type)
{
    Inkscape::Selection *selection = sp_desktop_selection(lc->desktop);
    SPItem *item = selection->singleItem();

    // TODO: should we check whether type represents a valid geometric construction?
    if (item && SP_IS_LPE_ITEM(item) && Inkscape::LivePathEffect::Effect::acceptsNumClicks(type) == 0) {
        Inkscape::LivePathEffect::Effect::createAndApply(type, sp_desktop_document(lc->desktop), item);
        return true;
    }
    return false;
}

void
lpetool_context_switch_mode(SPLPEToolContext *lc, Inkscape::LivePathEffect::EffectType const type)
{
    int index = lpetool_mode_to_index(type);
    if (index != -1) {
        lc->mode = type;
        lc->desktop->setToolboxSelectOneValue ("lpetool_mode_action", index);
    } else {
        g_warning ("Invalid mode selected: %d", type);
        return;
    }
}

void
lpetool_get_limiting_bbox_corners(SPDocument *document, Geom::Point &A, Geom::Point &B) {
    Geom::Coord w = sp_document_width(document);
    Geom::Coord h = sp_document_height(document);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    double ulx = prefs->getDouble("/tools/lpetool/bbox_upperleftx", 0);
    double uly = prefs->getDouble("/tools/lpetool/bbox_upperlefty", 0);
    double lrx = prefs->getDouble("/tools/lpetool/bbox_lowerrightx", w);
    double lry = prefs->getDouble("/tools/lpetool/bbox_lowerrighty", h);

    A = Geom::Point(ulx, uly);
    B = Geom::Point(lrx, lry);
}

/*
 * Reads the limiting bounding box from preferences and draws it on the screen
 */
// TODO: Note that currently the bbox is not user-settable; we simply use the page borders
void
lpetool_context_reset_limiting_bbox(SPLPEToolContext *lc)
{
    if (lc->canvas_bbox) {
        gtk_object_destroy(GTK_OBJECT(lc->canvas_bbox));
        lc->canvas_bbox = NULL;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (!prefs->getBool("/tools/lpetool/show_bbox", true))
        return;

    SPDocument *document = sp_desktop_document(lc->desktop);

    Geom::Point A, B;
    lpetool_get_limiting_bbox_corners(document, A, B);
    Geom::Matrix doc2dt(lc->desktop->doc2dt());
    A *= doc2dt;
    B *= doc2dt;

    Geom::Rect rect(A, B);
    SPCurve *curve = SPCurve::new_from_rect(rect);

    lc->canvas_bbox = sp_canvas_bpath_new (sp_desktop_controls(lc->desktop), curve);
    sp_canvas_bpath_set_stroke(SP_CANVAS_BPATH(lc->canvas_bbox), 0x0000ffff, 0.8, SP_STROKE_LINEJOIN_MITER, SP_STROKE_LINECAP_BUTT, 5, 5);
}

static void
set_pos_and_anchor(SPCanvasText *canvas_text, const Geom::Piecewise<Geom::D2<Geom::SBasis> > &pwd2,
                   const double t, const double length, bool /*use_curvature*/ = false)
{
    using namespace Geom;

    Piecewise<D2<SBasis> > pwd2_reparam = arc_length_parametrization(pwd2, 2 , 0.1);
    double t_reparam = pwd2_reparam.cuts.back() * t;
    Point pos = pwd2_reparam.valueAt(t_reparam);
    Point dir = unit_vector(derivative(pwd2_reparam).valueAt(t_reparam));
    Point n = -rot90(dir);
    double angle = Geom::angle_between(dir, Point(1,0));

    sp_canvastext_set_coords(canvas_text, pos + n * length);
    sp_canvastext_set_anchor(canvas_text, std::sin(angle), -std::cos(angle));
}

void
lpetool_create_measuring_items(SPLPEToolContext *lc, Inkscape::Selection *selection)
{
    if (!selection) {
        selection = sp_desktop_selection(lc->desktop);
    }
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    bool show = prefs->getBool("/tools/lpetool/show_measuring_info",  true);

    SPPath *path;
    SPCurve *curve;
    SPCanvasText *canvas_text;
    SPCanvasGroup *tmpgrp = sp_desktop_tempgroup(lc->desktop);
    gchar *arc_length;
    double lengthval;

    for (GSList const *i = selection->itemList(); i != NULL; i = i->next) {
        if (SP_IS_PATH(i->data)) {
            path = SP_PATH(i->data);
            curve = sp_shape_get_curve(SP_SHAPE(path));
            Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2 = paths_to_pw(curve->get_pathvector());
            canvas_text = (SPCanvasText *) sp_canvastext_new(tmpgrp, lc->desktop, Geom::Point(0,0), "");
            if (!show)
                sp_canvas_item_hide(SP_CANVAS_ITEM(canvas_text));

            SPUnitId unitid = static_cast<SPUnitId>(prefs->getInt("/tools/lpetool/unitid", SP_UNIT_PX));
            SPUnit unit = sp_unit_get_by_id(unitid);

            lengthval = Geom::length(pwd2);
            gboolean success;
            success = sp_convert_distance(&lengthval, &sp_unit_get_by_id(SP_UNIT_PX), &unit);
            arc_length = g_strdup_printf("%.2f %s", lengthval, success ? sp_unit_get_abbreviation(&unit) : "px");
            sp_canvastext_set_text (canvas_text, arc_length);
            set_pos_and_anchor(canvas_text, pwd2, 0.5, 10);
            // TODO: must we free arc_length?
            (*lc->measuring_items)[path] = SP_CANVAS_ITEM(canvas_text);
        }
    }
}

void
lpetool_delete_measuring_items(SPLPEToolContext *lc)
{
    std::map<SPPath *, SPCanvasItem*>::iterator i;
    for (i = lc->measuring_items->begin(); i != lc->measuring_items->end(); ++i) {
        gtk_object_destroy(GTK_OBJECT(i->second));
    }
    lc->measuring_items->clear();
}

void
lpetool_update_measuring_items(SPLPEToolContext *lc)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    SPPath *path;
    SPCurve *curve;
    double lengthval;
    gchar *arc_length;
    std::map<SPPath *, SPCanvasItem*>::iterator i;
    for (i = lc->measuring_items->begin(); i != lc->measuring_items->end(); ++i) {
        path = i->first;
        curve = sp_shape_get_curve(SP_SHAPE(path));
        Geom::Piecewise<Geom::D2<Geom::SBasis> > pwd2 = Geom::paths_to_pw(curve->get_pathvector());
        SPUnitId unitid = static_cast<SPUnitId>(prefs->getInt("/tools/lpetool/unitid", SP_UNIT_PX));
        SPUnit unit = sp_unit_get_by_id(unitid);
        lengthval = Geom::length(pwd2);
        gboolean success;
        success = sp_convert_distance(&lengthval, &sp_unit_get_by_id(SP_UNIT_PX), &unit);
        arc_length = g_strdup_printf("%.2f %s", lengthval, success ? sp_unit_get_abbreviation(&unit) : "px");
        sp_canvastext_set_text (SP_CANVASTEXT(i->second), arc_length);
        set_pos_and_anchor(SP_CANVASTEXT(i->second), pwd2, 0.5, 10);
        // TODO: must we free arc_length?
    }
}

void
lpetool_show_measuring_info(SPLPEToolContext *lc, bool show)
{
    std::map<SPPath *, SPCanvasItem*>::iterator i;
    for (i = lc->measuring_items->begin(); i != lc->measuring_items->end(); ++i) {
        if (show) {
            sp_canvas_item_show(i->second);
        } else {
            sp_canvas_item_hide(i->second);
        }
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

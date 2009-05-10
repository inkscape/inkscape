#define __SP_SELECT_CONTEXT_C__

/*
 * Selection and transformation context
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2006      Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 1999-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <cstring>
#include <string>
#include <gdk/gdkkeysyms.h>
#include "macros.h"
#include "rubberband.h"
#include "document.h"
#include "selection.h"
#include "seltrans-handles.h"
#include "sp-cursor.h"
#include "pixmaps/cursor-select-m.xpm"
#include "pixmaps/cursor-select-d.xpm"
#include "pixmaps/handles.xpm"
#include <glibmm/i18n.h>

#include "select-context.h"
#include "selection-chemistry.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "sp-root.h"
#include "preferences.h"
#include "tools-switch.h"
#include "message-stack.h"
#include "selection-describer.h"
#include "seltrans.h"
#include "box3d.h"

static void sp_select_context_class_init(SPSelectContextClass *klass);
static void sp_select_context_init(SPSelectContext *select_context);
static void sp_select_context_dispose(GObject *object);

static void sp_select_context_setup(SPEventContext *ec);
static void sp_select_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *val);
static gint sp_select_context_root_handler(SPEventContext *event_context, GdkEvent *event);
static gint sp_select_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event);

static SPEventContextClass *parent_class;

static GdkCursor *CursorSelectMouseover = NULL;
static GdkCursor *CursorSelectDragging = NULL;
GdkPixbuf *handles[13];

static gint rb_escaped = 0; // if non-zero, rubberband was canceled by esc, so the next button release should not deselect
static gint drag_escaped = 0; // if non-zero, drag was canceled by esc

static gint xp = 0, yp = 0; // where drag started
static gint tolerance = 0;
static bool within_tolerance = false;

GtkType
sp_select_context_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPSelectContextClass),
            NULL, NULL,
            (GClassInitFunc) sp_select_context_class_init,
            NULL, NULL,
            sizeof(SPSelectContext),
            4,
            (GInstanceInitFunc) sp_select_context_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_EVENT_CONTEXT, "SPSelectContext", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_select_context_class_init(SPSelectContextClass *klass)
{
    GObjectClass *object_class = (GObjectClass *) klass;
    SPEventContextClass *event_context_class = (SPEventContextClass *) klass;

    parent_class = (SPEventContextClass*)g_type_class_peek_parent(klass);

    object_class->dispose = sp_select_context_dispose;

    event_context_class->setup = sp_select_context_setup;
    event_context_class->set = sp_select_context_set;
    event_context_class->root_handler = sp_select_context_root_handler;
    event_context_class->item_handler = sp_select_context_item_handler;

    // cursors in select context
    CursorSelectMouseover = sp_cursor_new_from_xpm(cursor_select_m_xpm , 1, 1);
    CursorSelectDragging = sp_cursor_new_from_xpm(cursor_select_d_xpm , 1, 1);
    // selection handles
    handles[0]  = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_scale_nw_xpm);
    handles[1]  = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_scale_ne_xpm);
    handles[2]  = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_scale_h_xpm);
    handles[3]  = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_scale_v_xpm);
    handles[4]  = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_rotate_nw_xpm);
    handles[5]  = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_rotate_n_xpm);
    handles[6]  = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_rotate_ne_xpm);
    handles[7]  = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_rotate_e_xpm);
    handles[8]  = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_rotate_se_xpm);
    handles[9]  = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_rotate_s_xpm);
    handles[10] = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_rotate_sw_xpm);
    handles[11] = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_rotate_w_xpm);
    handles[12] = gdk_pixbuf_new_from_xpm_data((gchar const **)handle_center_xpm);

}

static void
sp_select_context_init(SPSelectContext *sc)
{
    sc->dragging = FALSE;
    sc->moved = FALSE;
    sc->button_press_shift = false;
    sc->button_press_ctrl = false;
    sc->button_press_alt = false;
    sc->_seltrans = NULL;
    sc->_describer = NULL;
}

static void
sp_select_context_dispose(GObject *object)
{
    SPSelectContext *sc = SP_SELECT_CONTEXT(object);
    SPEventContext * ec = SP_EVENT_CONTEXT (object);

    ec->enableGrDrag(false);

    if (sc->grabbed) {
        sp_canvas_item_ungrab(sc->grabbed, GDK_CURRENT_TIME);
        sc->grabbed = NULL;
    }

    delete sc->_seltrans;
    sc->_seltrans = NULL;
    delete sc->_describer;
    sc->_describer = NULL;

    if (CursorSelectDragging) {
        gdk_cursor_unref (CursorSelectDragging);
        CursorSelectDragging = NULL;
    }
    if (CursorSelectMouseover) {
        gdk_cursor_unref (CursorSelectMouseover);
        CursorSelectMouseover = NULL;
    }

    G_OBJECT_CLASS(parent_class)->dispose(object);
}

static void
sp_select_context_setup(SPEventContext *ec)
{
    SPSelectContext *select_context = SP_SELECT_CONTEXT(ec);

    if (((SPEventContextClass *) parent_class)->setup) {
        ((SPEventContextClass *) parent_class)->setup(ec);
    }

    SPDesktop *desktop = ec->desktop;

    select_context->_describer = new Inkscape::SelectionDescriber(desktop->selection, desktop->messageStack());

    select_context->_seltrans = new Inkscape::SelTrans(desktop);

    sp_event_context_read(ec, "show");
    sp_event_context_read(ec, "transform");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/select/gradientdrag")) {
        ec->enableGrDrag();
    }
}

static void
sp_select_context_set(SPEventContext *ec, Inkscape::Preferences::Entry *val)
{
    SPSelectContext *sc = SP_SELECT_CONTEXT(ec);
    Glib::ustring path = val->getEntryName();

    if (path == "show") {
        if (val->getString() == "outline") {
            sc->_seltrans->setShow(Inkscape::SelTrans::SHOW_OUTLINE);
        } else {
            sc->_seltrans->setShow(Inkscape::SelTrans::SHOW_CONTENT);
        }
    }
}

static bool
sp_select_context_abort(SPEventContext *event_context)
{
    SPDesktop *desktop = event_context->desktop;
    SPSelectContext *sc = SP_SELECT_CONTEXT(event_context);
    Inkscape::SelTrans *seltrans = sc->_seltrans;

    if (sc->dragging) {
        if (sc->moved) { // cancel dragging an object
            seltrans->ungrab();
            sc->moved = FALSE;
            sc->dragging = FALSE;
            sp_event_context_snap_window_closed(event_context);
            drag_escaped = 1;

            if (sc->item) {
                // only undo if the item is still valid
                if (SP_OBJECT_DOCUMENT( SP_OBJECT(sc->item))) {
                    sp_document_undo(sp_desktop_document(desktop));
                }

                sp_object_unref( SP_OBJECT(sc->item), NULL);
            } else if (sc->button_press_ctrl) {
                // NOTE:  This is a workaround to a bug.
                // When the ctrl key is held, sc->item is not defined
                // so in this case (only), we skip the object doc check
                sp_document_undo(sp_desktop_document(desktop));
            }
            sc->item = NULL;

            SP_EVENT_CONTEXT(sc)->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Move canceled."));
            return true;
        }
    } else {
        if (Inkscape::Rubberband::get(desktop)->is_started()) {
            Inkscape::Rubberband::get(desktop)->stop();
            rb_escaped = 1;
            SP_EVENT_CONTEXT(sc)->defaultMessageContext()->clear();
            SP_EVENT_CONTEXT(sc)->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Selection canceled."));
            return true;
        }
    }
    return false;
}

bool
key_is_a_modifier (guint key) {
    return (key == GDK_Alt_L ||
                key == GDK_Alt_R ||
                key == GDK_Control_L ||
                key == GDK_Control_R ||
                key == GDK_Shift_L ||
                key == GDK_Shift_R ||
                key == GDK_Meta_L ||  // Meta is when you press Shift+Alt (at least on my machine)
            key == GDK_Meta_R);
}

void
sp_select_context_up_one_layer(SPDesktop *desktop)
{
    /* Click in empty place, go up one level -- but don't leave a layer to root.
     *
     * (Rationale: we don't usually allow users to go to the root, since that
     * detracts from the layer metaphor: objects at the root level can in front
     * of or behind layers.  Whereas it's fine to go to the root if editing
     * a document that has no layers (e.g. a non-Inkscape document).)
     *
     * Once we support editing SVG "islands" (e.g. <svg> embedded in an xhtml
     * document), we might consider further restricting the below to disallow
     * leaving a layer to go to a non-layer.
     */
    SPObject *const current_layer = desktop->currentLayer();
    if (current_layer) {
        SPObject *const parent = SP_OBJECT_PARENT(current_layer);
        if ( parent
             && ( SP_OBJECT_PARENT(parent)
                  || !( SP_IS_GROUP(current_layer)
                        && ( SPGroup::LAYER
                             == SP_GROUP(current_layer)->layerMode() ) ) ) )
        {
            desktop->setCurrentLayer(parent);
            if (SP_IS_GROUP(current_layer) && SPGroup::LAYER != SP_GROUP(current_layer)->layerMode())
                sp_desktop_selection(desktop)->set(current_layer);
        }
    }
}

static gint
sp_select_context_item_handler(SPEventContext *event_context, SPItem *item, GdkEvent *event)
{
    gint ret = FALSE;

    SPDesktop *desktop = event_context->desktop;
    SPSelectContext *sc = SP_SELECT_CONTEXT(event_context);
    Inkscape::SelTrans *seltrans = sc->_seltrans;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    // make sure we still have valid objects to move around
    if (sc->item && SP_OBJECT_DOCUMENT( SP_OBJECT(sc->item))==NULL) {
        sp_select_context_abort(event_context);
    }

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {
                /* Left mousebutton */

                // save drag origin
                xp = (gint) event->button.x;
                yp = (gint) event->button.y;
                within_tolerance = true;

                // remember what modifiers were on before button press
                sc->button_press_shift = (event->button.state & GDK_SHIFT_MASK) ? true : false;
                sc->button_press_ctrl = (event->button.state & GDK_CONTROL_MASK) ? true : false;
                sc->button_press_alt = (event->button.state & GDK_MOD1_MASK) ? true : false;

                if (event->button.state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) {
                    // if shift or ctrl was pressed, do not move objects;
                    // pass the event to root handler which will perform rubberband, shift-click, ctrl-click, ctrl-drag
                } else {
                    sc->dragging = TRUE;
                    sp_event_context_snap_window_open(event_context);
                    sc->moved = FALSE;

                    sp_canvas_force_full_redraw_after_interruptions(desktop->canvas, 5);

                    // remember the clicked item in sc->item:
                    if (sc->item) {
                        sp_object_unref(sc->item, NULL);
                        sc->item = NULL;
                    }
                    sc->item = sp_event_context_find_item (desktop,
                                              Geom::Point(event->button.x, event->button.y), event->button.state & GDK_MOD1_MASK, FALSE);
                    sp_object_ref(sc->item, NULL);

                    rb_escaped = drag_escaped = 0;

                    if (sc->grabbed) {
                        sp_canvas_item_ungrab(sc->grabbed, event->button.time);
                        sc->grabbed = NULL;
                    }
                    sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->drawing),
                                        GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_PRESS_MASK |
                                        GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK,
                                        NULL, event->button.time);
                    sc->grabbed = SP_CANVAS_ITEM(desktop->drawing);

                    sp_canvas_force_full_redraw_after_interruptions(desktop->canvas, 5);

                    ret = TRUE;
                }
            } else if (event->button.button == 3) {
                // right click; do not eat it so that right-click menu can appear, but cancel dragging & rubberband
                sp_select_context_abort(event_context);
            }
            break;

        case GDK_ENTER_NOTIFY:
        {
            if (!desktop->isWaitingCursor()) {
                GdkCursor *cursor = gdk_cursor_new(GDK_FLEUR);
                gdk_window_set_cursor(GTK_WIDGET(sp_desktop_canvas(desktop))->window, cursor);
                gdk_cursor_destroy(cursor);
            }
            break;
        }

        case GDK_LEAVE_NOTIFY:
            if (!desktop->isWaitingCursor())
                gdk_window_set_cursor(GTK_WIDGET(sp_desktop_canvas(desktop))->window, event_context->cursor);
            break;

        case GDK_KEY_PRESS:
            if (get_group0_keyval (&event->key) == GDK_space) {
                if (sc->dragging && sc->grabbed) {
                    /* stamping mode: show content mode moving */
                    seltrans->stamp();
                    ret = TRUE;
                }
            }
            break;

        default:
            break;
    }

    if (!ret) {
        if (((SPEventContextClass *) parent_class)->item_handler)
            ret = ((SPEventContextClass *) parent_class)->item_handler(event_context, item, event);
    }

    return ret;
}

static gint
sp_select_context_root_handler(SPEventContext *event_context, GdkEvent *event)
{
    SPItem *item = NULL;
    SPItem *item_at_point = NULL, *group_at_point = NULL, *item_in_group = NULL;
    gint ret = FALSE;

    SPDesktop *desktop = event_context->desktop;
    SPSelectContext *sc = SP_SELECT_CONTEXT(event_context);
    Inkscape::SelTrans *seltrans = sc->_seltrans;
    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // make sure we still have valid objects to move around
    if (sc->item && SP_OBJECT_DOCUMENT( SP_OBJECT(sc->item))==NULL) {
        sp_select_context_abort(event_context);
    }

    switch (event->type) {
        case GDK_2BUTTON_PRESS:
            if (event->button.button == 1) {
                if (!selection->isEmpty()) {
                    SPItem *clicked_item = (SPItem *) selection->itemList()->data;
                    if (SP_IS_GROUP(clicked_item) && !SP_IS_BOX3D(clicked_item)) { // enter group if it's not a 3D box
                        desktop->setCurrentLayer(reinterpret_cast<SPObject *>(clicked_item));
                        sp_desktop_selection(desktop)->clear();
                        sc->dragging = false;
                        sp_event_context_snap_window_closed(event_context);

                        sp_canvas_end_forced_full_redraws(desktop->canvas);
                    } else { // switch tool
                        Geom::Point const button_pt(event->button.x, event->button.y);
                        Geom::Point const p(desktop->w2d(button_pt));
                        tools_switch_by_item (desktop, clicked_item, p);
                    }
                } else {
                    sp_select_context_up_one_layer(desktop);
                }
                ret = TRUE;
            }
            break;
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !event_context->space_panning) {

                // save drag origin
                xp = (gint) event->button.x;
                yp = (gint) event->button.y;
                within_tolerance = true;

                Geom::Point const button_pt(event->button.x, event->button.y);
                Geom::Point const p(desktop->w2d(button_pt));
                if (event->button.state & GDK_MOD1_MASK)
                    Inkscape::Rubberband::get(desktop)->setMode(RUBBERBAND_MODE_TOUCHPATH);
                Inkscape::Rubberband::get(desktop)->start(desktop, p);
                if (sc->grabbed) {
                    sp_canvas_item_ungrab(sc->grabbed, event->button.time);
                    sc->grabbed = NULL;
                }
                sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                    GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK,
                                    NULL, event->button.time);
                sc->grabbed = SP_CANVAS_ITEM(desktop->acetate);

                // remember what modifiers were on before button press
                sc->button_press_shift = (event->button.state & GDK_SHIFT_MASK) ? true : false;
                sc->button_press_ctrl = (event->button.state & GDK_CONTROL_MASK) ? true : false;
                sc->button_press_alt = (event->button.state & GDK_MOD1_MASK) ? true : false;

                sc->moved = FALSE;

                rb_escaped = drag_escaped = 0;

                ret = TRUE;
            } else if (event->button.button == 3) {
                // right click; do not eat it so that right-click menu can appear, but cancel dragging & rubberband
                sp_select_context_abort(event_context);
            }
            break;

        case GDK_MOTION_NOTIFY:
        	tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);
        	if (event->motion.state & GDK_BUTTON1_MASK && !event_context->space_panning) {
                Geom::Point const motion_pt(event->motion.x, event->motion.y);
                Geom::Point const p(desktop->w2d(motion_pt));

                if ( within_tolerance
                     && ( abs( (gint) event->motion.x - xp ) < tolerance )
                     && ( abs( (gint) event->motion.y - yp ) < tolerance ) ) {
                    break; // do not drag if we're within tolerance from origin
                }
                // Once the user has moved farther than tolerance from the original location
                // (indicating they intend to move the object, not click), then always process the
                // motion notify coordinates as given (no snapping back to origin)
                within_tolerance = false;

                if (sc->button_press_ctrl || (sc->button_press_alt && !sc->button_press_shift && !selection->isEmpty())) {
                    // if it's not click and ctrl or alt was pressed (the latter with some selection
                    // but not with shift) we want to drag rather than rubberband
                	if (sc->dragging == FALSE) {
                		sp_event_context_snap_window_open(event_context);
                	}
                	sc->dragging = TRUE;

                    sp_canvas_force_full_redraw_after_interruptions(desktop->canvas, 5);
                }

                if (sc->dragging) {
                    /* User has dragged fast, so we get events on root (lauris)*/
                    // not only that; we will end up here when ctrl-dragging as well
                    // and also when we started within tolerance, but trespassed tolerance outside of item
                    Inkscape::Rubberband::get(desktop)->stop();
                    SP_EVENT_CONTEXT(sc)->defaultMessageContext()->clear();
                    item_at_point = desktop->item_at_point(Geom::Point(event->button.x, event->button.y), FALSE);
                    if (!item_at_point) // if no item at this point, try at the click point (bug 1012200)
                        item_at_point = desktop->item_at_point(Geom::Point(xp, yp), FALSE);
                    if (item_at_point || sc->moved || sc->button_press_alt) {
                        // drag only if starting from an item, or if something is already grabbed, or if alt-dragging
                        if (!sc->moved) {
                            item_in_group = desktop->item_at_point(Geom::Point(event->button.x, event->button.y), TRUE);
                            group_at_point = desktop->group_at_point(Geom::Point(event->button.x, event->button.y));
                            if (SP_IS_LAYER(selection->single()))
                                group_at_point = SP_GROUP(selection->single());

                            // group-at-point is meant to be topmost item if it's a group,
                            // not topmost group of all items at point
                            if (group_at_point != item_in_group &&
                                !(group_at_point && item_at_point &&
                                  group_at_point->isAncestorOf(item_at_point)))
                                group_at_point = NULL;

                            // if neither a group nor an item (possibly in a group) at point are selected, set selection to the item at point
                            if ((!item_in_group || !selection->includes(item_in_group)) &&
                                (!group_at_point || !selection->includes(group_at_point))
                                && !sc->button_press_alt) {
                                // select what is under cursor
                                if (!seltrans->isEmpty()) {
                                    seltrans->resetState();
                                }
                                // when simply ctrl-dragging, we don't want to go into groups
                                if (item_at_point && !selection->includes(item_at_point))
                                    selection->set(item_at_point);
                            } // otherwise, do not change selection so that dragging selected-within-group items, as well as alt-dragging, is possible
                            seltrans->grab(p, -1, -1, FALSE);
                            sc->moved = TRUE;
                        }
                        if (!seltrans->isEmpty())
                            seltrans->moveTo(p, event->button.state);
                        desktop->scroll_to_point(p);
                        gobble_motion_events(GDK_BUTTON1_MASK);
                        ret = TRUE;
                    } else {
                        sc->dragging = FALSE;
                        sp_event_context_snap_window_closed(event_context);
                        sp_canvas_end_forced_full_redraws(desktop->canvas);
                    }
                } else {
                    if (Inkscape::Rubberband::get(desktop)->is_started()) {
                        Inkscape::Rubberband::get(desktop)->move(p);
                        if (Inkscape::Rubberband::get(desktop)->getMode() == RUBBERBAND_MODE_TOUCHPATH) {
                            event_context->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Draw over</b> objects to select them; release <b>Alt</b> to switch to rubberband selection"));
                        } else {
                            event_context->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Drag around</b> objects to select them; press <b>Alt</b> to switch to touch selection"));
                        }
                        gobble_motion_events(GDK_BUTTON1_MASK);
                    }
                }
            }
            break;
        case GDK_BUTTON_RELEASE:
            xp = yp = 0;
            if ((event->button.button == 1) && (sc->grabbed) && !event_context->space_panning) {
                if (sc->dragging) {
                    if (sc->moved) {
                        // item has been moved
                        seltrans->ungrab();
                        sc->moved = FALSE;
                    } else if (sc->item && !drag_escaped) {
                        // item has not been moved -> simply a click, do selecting
                        if (!selection->isEmpty()) {
                            if (event->button.state & GDK_SHIFT_MASK) {
                                // with shift, toggle selection
                                seltrans->resetState();
                                selection->toggle(sc->item);
                            } else {
                                SPObject* single = selection->single();
                                // without shift, increase state (i.e. toggle scale/rotation handles)
                                if (selection->includes(sc->item)) {
                                    seltrans->increaseState();
                                } else if (SP_IS_LAYER(single) && single->isAncestorOf(sc->item)) {
                                    seltrans->increaseState();
                                } else {
                                    seltrans->resetState();
                                    selection->set(sc->item);
                                }
                            }
                        } else { // simple or shift click, no previous selection
                            seltrans->resetState();
                            selection->set(sc->item);
                        }
                    }
                    sc->dragging = FALSE;
                    sp_event_context_snap_window_closed(event_context);
                    sp_canvas_end_forced_full_redraws(desktop->canvas);

                    if (sc->item) {
                        sp_object_unref( SP_OBJECT(sc->item), NULL);
                    }
                    sc->item = NULL;
                } else {
                    Inkscape::Rubberband::Rubberband *r = Inkscape::Rubberband::get(desktop);
                    if (r->is_started() && !within_tolerance) {
                        // this was a rubberband drag
                        GSList *items = NULL;
                        if (r->getMode() == RUBBERBAND_MODE_RECT) {
                            Geom::OptRect const b = r->getRectangle();
                            items = sp_document_items_in_box(sp_desktop_document(desktop), desktop->dkey, *b);
                        } else if (r->getMode() == RUBBERBAND_MODE_TOUCHPATH) {
                            items = sp_document_items_at_points(sp_desktop_document(desktop), desktop->dkey, r->getPoints());
                        }

                        seltrans->resetState();
                        r->stop();
                        SP_EVENT_CONTEXT(sc)->defaultMessageContext()->clear();

                        if (event->button.state & GDK_SHIFT_MASK) {
                            // with shift, add to selection
                            selection->addList (items);
                        } else {
                            // without shift, simply select anew
                            selection->setList (items);
                        }
                        g_slist_free (items);
                    } else { // it was just a click, or a too small rubberband
                        r->stop();
                        if (sc->button_press_shift && !rb_escaped && !drag_escaped) {
                            // this was a shift+click or alt+shift+click, select what was clicked upon

                            sc->button_press_shift = false;

                            if (sc->button_press_ctrl) {
                                // go into groups, honoring Alt
                                item = sp_event_context_find_item (desktop,
                                                   Geom::Point(event->button.x, event->button.y), event->button.state & GDK_MOD1_MASK, TRUE);
                                sc->button_press_ctrl = FALSE;
                            } else {
                                // don't go into groups, honoring Alt
                                item = sp_event_context_find_item (desktop,
                                                   Geom::Point(event->button.x, event->button.y), event->button.state & GDK_MOD1_MASK, FALSE);
                            }

                            if (item) {
                                selection->toggle(item);
                                item = NULL;
                            }

                        } else if ((sc->button_press_ctrl || sc->button_press_alt) && !rb_escaped && !drag_escaped) { // ctrl+click, alt+click

                            item = sp_event_context_find_item (desktop,
                                         Geom::Point(event->button.x, event->button.y), sc->button_press_alt, sc->button_press_ctrl);

                            sc->button_press_ctrl = FALSE;
                            sc->button_press_alt = FALSE;

                            if (item) {
                                if (selection->includes(item)) {
                                    seltrans->increaseState();
                                } else {
                                    seltrans->resetState();
                                    selection->set(item);
                                }
                                item = NULL;
                            }

                        } else { // click without shift, simply deselect, unless with Alt or something was cancelled
                            if (!selection->isEmpty()) {
                                if (!(rb_escaped) && !(drag_escaped) && !(event->button.state & GDK_MOD1_MASK))
                                    selection->clear();
                                rb_escaped = 0;
                                ret = TRUE;
                            }
                        }
                    }
                    ret = TRUE;
                }
                if (sc->grabbed) {
                    sp_canvas_item_ungrab(sc->grabbed, event->button.time);
                    sc->grabbed = NULL;
                }

                desktop->updateNow();
            }
            if (event->button.button == 1) {
                Inkscape::Rubberband::get(desktop)->stop(); // might have been started in another tool!
            }
            sc->button_press_shift = false;
            sc->button_press_ctrl = false;
            sc->button_press_alt = false;
            break;

        case GDK_KEY_PRESS: // keybindings for select context

			{
			{
        	guint keyval = get_group0_keyval(&event->key);
            bool alt = ( MOD__ALT
                                    || (keyval == GDK_Alt_L)
                                    || (keyval == GDK_Alt_R)
                                    || (keyval == GDK_Meta_L)
                                    || (keyval == GDK_Meta_R));

            if (!key_is_a_modifier (keyval)) {
                    event_context->defaultMessageContext()->clear();
            } else if (sc->grabbed || seltrans->isGrabbed()) {
                if (Inkscape::Rubberband::get(desktop)->is_started()) {
                    // if Alt then change cursor to moving cursor:
                    if (alt) {
                        Inkscape::Rubberband::get(desktop)->setMode(RUBBERBAND_MODE_TOUCHPATH);
                    }
                } else {
                // do not change the statusbar text when mousekey is down to move or transform the object,
                // because the statusbar text is already updated somewhere else.
                   break;
                }
            } else {
                    sp_event_show_modifier_tip (event_context->defaultMessageContext(), event,
                                                _("<b>Ctrl</b>: click to select in groups; drag to move hor/vert"),
                                                _("<b>Shift</b>: click to toggle select; drag for rubberband selection"),
                                                _("<b>Alt</b>: click to select under; drag to move selected or select by touch"));
                    // if Alt and nonempty selection, show moving cursor ("move selected"):
                    if (alt && !selection->isEmpty() && !desktop->isWaitingCursor()) {
                        GdkCursor *cursor = gdk_cursor_new(GDK_FLEUR);
                        gdk_window_set_cursor(GTK_WIDGET(sp_desktop_canvas(desktop))->window, cursor);
                        gdk_cursor_destroy(cursor);
                    }
                    //*/
                    break;
            }
			}

            gdouble const nudge = prefs->getDoubleLimited("/options/nudgedistance/value", 2, 0, 1000); // in px
			gdouble const offset = prefs->getDoubleLimited("/options/defaultscale/value", 2, 0, 1000);
			int const snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);

			switch (get_group0_keyval (&event->key)) {
                case GDK_Left: // move selection left
                case GDK_KP_Left:
                case GDK_KP_4:
                    if (!MOD__CTRL) { // not ctrl
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        if (MOD__ALT) { // alt
                            if (MOD__SHIFT) sp_selection_move_screen(desktop, mul*-10, 0); // shift
                            else sp_selection_move_screen(desktop, mul*-1, 0); // no shift
                        }
                        else { // no alt
                            if (MOD__SHIFT) sp_selection_move(desktop, mul*-10*nudge, 0); // shift
                            else sp_selection_move(desktop, mul*-nudge, 0); // no shift
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_Up: // move selection up
                case GDK_KP_Up:
                case GDK_KP_8:
                    if (!MOD__CTRL) { // not ctrl
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        if (MOD__ALT) { // alt
                            if (MOD__SHIFT) sp_selection_move_screen(desktop, 0, mul*10); // shift
                            else sp_selection_move_screen(desktop, 0, mul*1); // no shift
                        }
                        else { // no alt
                            if (MOD__SHIFT) sp_selection_move(desktop, 0, mul*10*nudge); // shift
                            else sp_selection_move(desktop, 0, mul*nudge); // no shift
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_Right: // move selection right
                case GDK_KP_Right:
                case GDK_KP_6:
                    if (!MOD__CTRL) { // not ctrl
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        if (MOD__ALT) { // alt
                            if (MOD__SHIFT) sp_selection_move_screen(desktop, mul*10, 0); // shift
                            else sp_selection_move_screen(desktop, mul*1, 0); // no shift
                        }
                        else { // no alt
                            if (MOD__SHIFT) sp_selection_move(desktop, mul*10*nudge, 0); // shift
                            else sp_selection_move(desktop, mul*nudge, 0); // no shift
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_Down: // move selection down
                case GDK_KP_Down:
                case GDK_KP_2:
                    if (!MOD__CTRL) { // not ctrl
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        if (MOD__ALT) { // alt
                            if (MOD__SHIFT) sp_selection_move_screen(desktop, 0, mul*-10); // shift
                            else sp_selection_move_screen(desktop, 0, mul*-1); // no shift
                        }
                        else { // no alt
                            if (MOD__SHIFT) sp_selection_move(desktop, 0, mul*-10*nudge); // shift
                            else sp_selection_move(desktop, 0, mul*-nudge); // no shift
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_Escape:
                    if (!sp_select_context_abort(event_context))
                        selection->clear();
                    ret = TRUE;
                    break;
                case GDK_a:
                case GDK_A:
                    if (MOD__CTRL_ONLY) {
                        sp_edit_select_all(desktop);
                        ret = TRUE;
                    }
                    break;
                case GDK_space:
                    /* stamping mode: show outline mode moving */
                    /* FIXME: Is next condition ok? (lauris) */
                    if (sc->dragging && sc->grabbed) {
                        seltrans->stamp();
                        ret = TRUE;
                    }
                    break;
                case GDK_x:
                case GDK_X:
                    if (MOD__ALT_ONLY) {
                        desktop->setToolboxFocusTo ("altx");
                        ret = TRUE;
                    }
                    break;
                case GDK_bracketleft:
                    if (MOD__ALT) {
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        sp_selection_rotate_screen(selection, mul*1);
                    } else if (MOD__CTRL) {
                        sp_selection_rotate(selection, 90);
                    } else if (snaps) {
                        sp_selection_rotate(selection, 180/snaps);
                    }
                    ret = TRUE;
                    break;
                case GDK_bracketright:
                    if (MOD__ALT) {
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        sp_selection_rotate_screen(selection, -1*mul);
                    } else if (MOD__CTRL) {
                        sp_selection_rotate(selection, -90);
                    } else if (snaps) {
                        sp_selection_rotate(selection, -180/snaps);
                    }
                    ret = TRUE;
                    break;
                case GDK_less:
                case GDK_comma:
                    if (MOD__ALT) {
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        sp_selection_scale_screen(selection, -2*mul);
                    } else if (MOD__CTRL) {
                        sp_selection_scale_times(selection, 0.5);
                    } else {
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        sp_selection_scale(selection, -offset*mul);
                    }
                    ret = TRUE;
                    break;
                case GDK_greater:
                case GDK_period:
                    if (MOD__ALT) {
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        sp_selection_scale_screen(selection, 2*mul);
                    } else if (MOD__CTRL) {
                        sp_selection_scale_times(selection, 2);
                    } else {
                        gint mul = 1 + gobble_key_events(
                            get_group0_keyval(&event->key), 0); // with any mask
                        sp_selection_scale(selection, offset*mul);
                    }
                    ret = TRUE;
                    break;
                case GDK_Return:
                    if (MOD__CTRL_ONLY) {
                        if (selection->singleItem()) {
                            SPItem *clicked_item = selection->singleItem();
                            if ( SP_IS_GROUP(clicked_item) ||
                                 SP_IS_BOX3D(clicked_item)) { // enter group or a 3D box
                                desktop->setCurrentLayer(reinterpret_cast<SPObject *>(clicked_item));
                                sp_desktop_selection(desktop)->clear();
                            } else {
                                SP_EVENT_CONTEXT(sc)->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Selected object is not a group. Cannot enter."));
                            }
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_BackSpace:
                    if (MOD__CTRL_ONLY) {
                        sp_select_context_up_one_layer(desktop);
                        ret = TRUE;
                    }
                    break;
                case GDK_s:
                case GDK_S:
                    if (MOD__SHIFT_ONLY) {
                        if (!selection->isEmpty()) {
                            seltrans->increaseState();
                        }
                        ret = TRUE;
                    }
                    break;
                case GDK_g:
                case GDK_G:
                    if (MOD__SHIFT_ONLY) {
                        sp_selection_to_guides(desktop);
                        ret = true;
                    }
                    break;
                default:
                    break;
            }
            break;
			}
        case GDK_KEY_RELEASE:
            {
            guint keyval = get_group0_keyval(&event->key);
            if (key_is_a_modifier (keyval))
                event_context->defaultMessageContext()->clear();

            bool alt = ( MOD__ALT
                         || (keyval == GDK_Alt_L)
                         || (keyval == GDK_Alt_R)
                         || (keyval == GDK_Meta_L)
                         || (keyval == GDK_Meta_R));

            if (Inkscape::Rubberband::get(desktop)->is_started()) {
                // if Alt then change cursor to moving cursor:
                if (alt) {
                    Inkscape::Rubberband::get(desktop)->setMode(RUBBERBAND_MODE_RECT);
                }
            }
            }
            // set cursor to default.
            if (!desktop->isWaitingCursor())
                gdk_window_set_cursor(GTK_WIDGET(sp_desktop_canvas(desktop))->window, event_context->cursor);
            break;
        default:
            break;
    }

    if (!ret) {
        if (((SPEventContextClass *) parent_class)->root_handler)
            ret = ((SPEventContextClass *) parent_class)->root_handler(event_context, event);
    }

    return ret;
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

/*
 * Selection and transformation context
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010      authors
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
#include "document-undo.h"
#include "selection.h"
#include "sp-cursor.h"
#include "style.h"
#include "pixmaps/cursor-select-m.xpm"
#include "pixmaps/cursor-select-d.xpm"
#include "pixmaps/handles.xpm"
#include <glibmm/i18n.h>

#include "ui/tools/select-tool.h"
#include "selection-chemistry.h"
#ifdef WITH_DBUS
#include "extension/dbus/document-interface.h"
#endif
#include "desktop.h"
#include "desktop-handles.h"
#include "sp-root.h"
#include "preferences.h"
#include "tools-switch.h"
#include "message-stack.h"
#include "selection-describer.h"
#include "seltrans.h"
#include "box3d.h"
#include "display/sp-canvas.h"
#include "display/sp-canvas-item.h"
#include "display/drawing-item.h"
#include "tool-factory.h"

using Inkscape::DocumentUndo;

GdkPixbuf *handles[13];

namespace Inkscape {
namespace UI {
namespace Tools {

static GdkCursor *CursorSelectMouseover = NULL;
static GdkCursor *CursorSelectDragging = NULL;

static gint rb_escaped = 0; // if non-zero, rubberband was canceled by esc, so the next button release should not deselect
static gint drag_escaped = 0; // if non-zero, drag was canceled by esc

namespace {
	ToolBase* createSelectContext() {
		return new SelectTool();
	}

	bool selectContextRegistered = ToolFactory::instance().registerObject("/tools/select", createSelectContext);
}

const std::string& SelectTool::getPrefsPath() {
	return SelectTool::prefsPath;
}

const std::string SelectTool::prefsPath = "/tools/select";


//Creates rotated variations for handles
static void
sp_load_handles(int start, int count, char const **xpm) {
    handles[start] = gdk_pixbuf_new_from_xpm_data((gchar const **)xpm);
    for(int i = start + 1; i < start + count; i++) {
        // We use either the original at *start or previous loop item to rotate
        handles[i] = gdk_pixbuf_rotate_simple(handles[i-1], GDK_PIXBUF_ROTATE_CLOCKWISE);
    }
}

SelectTool::SelectTool()
    // Don't load a default cursor
    : ToolBase(NULL, 0, 0)
    , dragging(false)
    , moved(false)
    , button_press_shift(false)
    , button_press_ctrl(false)
    , button_press_alt(false)
    , cycling_items(NULL)
    , cycling_items_cmp(NULL)
    , cycling_items_selected_before(NULL)
    , cycling_cur_item(NULL)
    , cycling_wrap(true)
    , item(NULL)
    , grabbed(NULL)
    , _seltrans(NULL)
    , _describer(NULL)
{
    // cursors in select context
    CursorSelectMouseover = sp_cursor_new_from_xpm(cursor_select_m_xpm , 1, 1);
    CursorSelectDragging = sp_cursor_new_from_xpm(cursor_select_d_xpm , 1, 1);
    
    // selection handles
    sp_load_handles(0, 2, handle_scale_xpm);
    sp_load_handles(2, 2, handle_stretch_xpm);
    sp_load_handles(4, 4, handle_rotate_xpm);
    sp_load_handles(8, 4, handle_skew_xpm);
    sp_load_handles(12, 1, handle_center_xpm);
}

//static gint xp = 0, yp = 0; // where drag started
//static gint tolerance = 0;
//static bool within_tolerance = false;
static bool is_cycling = false;
static bool moved_while_cycling = false;


SelectTool::~SelectTool() {
    this->enableGrDrag(false);

    if (this->grabbed) {
        sp_canvas_item_ungrab(this->grabbed, GDK_CURRENT_TIME);
        this->grabbed = NULL;
    }

    delete this->_seltrans;
    this->_seltrans = NULL;

    delete this->_describer;
    this->_describer = NULL;

    if (CursorSelectDragging) {
#if GTK_CHECK_VERSION(3,0,0)
        g_object_unref(CursorSelectDragging);
#else
        gdk_cursor_unref (CursorSelectDragging);
#endif
        CursorSelectDragging = NULL;
    }
    
    if (CursorSelectMouseover) {
#if GTK_CHECK_VERSION(3,0,0)
        g_object_unref(CursorSelectMouseover);
#else
        gdk_cursor_unref (CursorSelectMouseover);
#endif
        CursorSelectMouseover = NULL;
    }
}

void SelectTool::setup() {
    ToolBase::setup();

    this->_describer = new Inkscape::SelectionDescriber(
                desktop->selection, 
                desktop->messageStack(),
                _("Click selection to toggle scale/rotation handles"),
                _("No objects selected. Click, Shift+click, Alt+scroll mouse on top of objects, or drag around objects to select.")
    );

    this->_seltrans = new Inkscape::SelTrans(desktop);

    sp_event_context_read(this, "show");
    sp_event_context_read(this, "transform");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (prefs->getBool("/tools/select/gradientdrag")) {
        this->enableGrDrag();
    }
}

void SelectTool::set(const Inkscape::Preferences::Entry& val) {
    Glib::ustring path = val.getEntryName();

    if (path == "show") {
        if (val.getString() == "outline") {
            this->_seltrans->setShow(Inkscape::SelTrans::SHOW_OUTLINE);
        } else {
            this->_seltrans->setShow(Inkscape::SelTrans::SHOW_CONTENT);
        }
    }
}

bool SelectTool::sp_select_context_abort() {
    Inkscape::SelTrans *seltrans = this->_seltrans;

    if (this->dragging) {
        if (this->moved) { // cancel dragging an object
            seltrans->ungrab();
            this->moved = FALSE;
            this->dragging = FALSE;
            sp_event_context_discard_delayed_snap_event(this);
            drag_escaped = 1;

            if (this->item) {
                // only undo if the item is still valid
                if (this->item->document) {
                    DocumentUndo::undo(sp_desktop_document(desktop));
                }

                sp_object_unref( this->item, NULL);
            } else if (this->button_press_ctrl) {
                // NOTE:  This is a workaround to a bug.
                // When the ctrl key is held, sc->item is not defined
                // so in this case (only), we skip the object doc check
                DocumentUndo::undo(sp_desktop_document(desktop));
            }
            this->item = NULL;

            SP_EVENT_CONTEXT(this)->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Move canceled."));
            return true;
        }
    } else {
        if (Inkscape::Rubberband::get(desktop)->is_started()) {
            Inkscape::Rubberband::get(desktop)->stop();
            rb_escaped = 1;
            SP_EVENT_CONTEXT(this)->defaultMessageContext()->clear();
            SP_EVENT_CONTEXT(this)->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Selection canceled."));
            return true;
        }
    }
    return false;
}

static bool
key_is_a_modifier (guint key) {
    return (key == GDK_KEY_Alt_L ||
                key == GDK_KEY_Alt_R ||
                key == GDK_KEY_Control_L ||
                key == GDK_KEY_Control_R ||
                key == GDK_KEY_Shift_L ||
                key == GDK_KEY_Shift_R ||
                key == GDK_KEY_Meta_L ||  // Meta is when you press Shift+Alt (at least on my machine)
            key == GDK_KEY_Meta_R);
}

static void
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
        SPObject *const parent = current_layer->parent;
        if ( parent
             && ( parent->parent
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

bool SelectTool::item_handler(SPItem* item, GdkEvent* event) {
    gint ret = FALSE;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    // make sure we still have valid objects to move around
    if (this->item && this->item->document == NULL) {
        this->sp_select_context_abort();
    }

    switch (event->type) {
        case GDK_BUTTON_PRESS:
            if (event->button.button == 1 && !this->space_panning) {
                /* Left mousebutton */

                // save drag origin
                xp = (gint) event->button.x;
                yp = (gint) event->button.y;
                within_tolerance = true;

                // remember what modifiers were on before button press
                this->button_press_shift = (event->button.state & GDK_SHIFT_MASK) ? true : false;
                this->button_press_ctrl = (event->button.state & GDK_CONTROL_MASK) ? true : false;
                this->button_press_alt = (event->button.state & GDK_MOD1_MASK) ? true : false;

                if (event->button.state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) {
                    // if shift or ctrl was pressed, do not move objects;
                    // pass the event to root handler which will perform rubberband, shift-click, ctrl-click, ctrl-drag
                } else {
                    GdkWindow* window = gtk_widget_get_window (GTK_WIDGET (sp_desktop_canvas(desktop)));
                   
                	this->dragging = TRUE;
                    this->moved = FALSE;

                    gdk_window_set_cursor(window, CursorSelectDragging);

                    desktop->canvas->forceFullRedrawAfterInterruptions(5);

                    // remember the clicked item in this->item:
                    if (this->item) {
                        sp_object_unref(this->item, NULL);
                        this->item = NULL;
                    }

                    this->item = sp_event_context_find_item (desktop,
                                              Geom::Point(event->button.x, event->button.y), event->button.state & GDK_MOD1_MASK, FALSE);
                    sp_object_ref(this->item, NULL);

                    rb_escaped = drag_escaped = 0;

                    if (this->grabbed) {
                        sp_canvas_item_ungrab(this->grabbed, event->button.time);
                        this->grabbed = NULL;
                    }

                    sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->drawing),
                                        GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_PRESS_MASK |
                                        GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK,
                                        NULL, event->button.time);

                    this->grabbed = SP_CANVAS_ITEM(desktop->drawing);

                    desktop->canvas->forceFullRedrawAfterInterruptions(5);

                    ret = TRUE;
                }
            } else if (event->button.button == 3) {
                // right click; do not eat it so that right-click menu can appear, but cancel dragging & rubberband
                this->sp_select_context_abort();
            }
            break;

        case GDK_ENTER_NOTIFY: {
            if (!desktop->isWaitingCursor() && !this->dragging) {
                GdkWindow* window = gtk_widget_get_window (GTK_WIDGET (sp_desktop_canvas(desktop)));

                gdk_window_set_cursor(window, CursorSelectMouseover);
            }
            break;
        }
        case GDK_LEAVE_NOTIFY:
        	if (!desktop->isWaitingCursor() && !this->dragging) {
                GdkWindow* window = gtk_widget_get_window (GTK_WIDGET (sp_desktop_canvas(desktop)));

                gdk_window_set_cursor(window, this->cursor);
        	}
            break;

        case GDK_KEY_PRESS:
            if (get_group0_keyval (&event->key) == GDK_KEY_space) {
                if (this->dragging && this->grabbed) {
                    /* stamping mode: show content mode moving */
                    _seltrans->stamp();
                    ret = TRUE;
                }
            } else if (get_group0_keyval (&event->key) == GDK_KEY_Tab) {
                if (this->dragging && this->grabbed) {
                    _seltrans->getNextClosestPoint(false);
                    ret = TRUE;
                }
            } else if (get_group0_keyval (&event->key) == GDK_KEY_ISO_Left_Tab) {
                if (this->dragging && this->grabbed) {
                    _seltrans->getNextClosestPoint(true);
                    ret = TRUE;
                }
            }
            break;

        default:
            break;
    }

    if (!ret) {
    	ret = ToolBase::item_handler(item, event);
    }

    return ret;
}

void SelectTool::sp_select_context_cycle_through_items(Inkscape::Selection *selection, GdkEventScroll *scroll_event, bool shift_pressed) {
    if (!this->cycling_cur_item) {
        return;
    }

    Inkscape::DrawingItem *arenaitem;
    SPItem *item = SP_ITEM(this->cycling_cur_item->data);

    // Deactivate current item
    if (!g_list_find(this->cycling_items_selected_before, item) && selection->includes(item)) {
        selection->remove(item);
    }

    arenaitem = item->get_arenaitem(desktop->dkey);
    arenaitem->setOpacity(0.3);

    // Find next item and activate it
    GList *next;
    if (scroll_event->direction == GDK_SCROLL_UP) {
        next = this->cycling_cur_item->next;
        if (next == NULL && this->cycling_wrap)
            next = this->cycling_items;
    } else {
        next = this->cycling_cur_item->prev;
        if (next == NULL && this->cycling_wrap)
            next = g_list_last(this->cycling_items);
    }

    if (next) {
        this->cycling_cur_item = next;
        item = SP_ITEM(this->cycling_cur_item->data);
    }

    arenaitem = item->get_arenaitem(desktop->dkey);
    arenaitem->setOpacity(1.0);

    if (shift_pressed) {
        selection->add(item);
    } else {
        selection->set(item);
    }
}

void SelectTool::sp_select_context_reset_opacities() {
    for (GList *l = this->cycling_items; l != NULL; l = g_list_next(l)) {
        Inkscape::DrawingItem *arenaitem = SP_ITEM(l->data)->get_arenaitem(this->desktop->dkey);
        arenaitem->setOpacity(SP_SCALE24_TO_FLOAT(SP_ITEM(l->data)->style->opacity.value));
    }

    g_list_free(this->cycling_items);
    g_list_free(this->cycling_items_selected_before);
    g_list_free(this->cycling_items_cmp);

    this->cycling_items = NULL;
    this->cycling_items_selected_before = NULL;
    this->cycling_cur_item = NULL;
    this->cycling_items_cmp = NULL;
}

bool SelectTool::root_handler(GdkEvent* event) {
    SPItem *item = NULL;
    SPItem *item_at_point = NULL, *group_at_point = NULL, *item_in_group = NULL;
    gint ret = FALSE;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    // make sure we still have valid objects to move around
    if (this->item && this->item->document == NULL) {
        this->sp_select_context_abort();
    }

    switch (event->type) {
        case GDK_2BUTTON_PRESS:
            if (event->button.button == 1) {
                if (!selection->isEmpty()) {
                    SPItem *clicked_item = static_cast<SPItem *>(selection->itemList()->data);

                    if (SP_IS_GROUP(clicked_item) && !SP_IS_BOX3D(clicked_item)) { // enter group if it's not a 3D box
                        desktop->setCurrentLayer(reinterpret_cast<SPObject *>(clicked_item));
                        sp_desktop_selection(desktop)->clear();
                        this->dragging = false;
                        sp_event_context_discard_delayed_snap_event(this);

                        desktop->canvas->endForcedFullRedraws();
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
            if (event->button.button == 1 && !this->space_panning) {
                // save drag origin
                xp = (gint) event->button.x;
                yp = (gint) event->button.y;
                within_tolerance = true;

                Geom::Point const button_pt(event->button.x, event->button.y);
                Geom::Point const p(desktop->w2d(button_pt));

                if (event->button.state & GDK_MOD1_MASK) {
                    Inkscape::Rubberband::get(desktop)->setMode(RUBBERBAND_MODE_TOUCHPATH);
                }

                Inkscape::Rubberband::get(desktop)->start(desktop, p);

                if (this->grabbed) {
                    sp_canvas_item_ungrab(this->grabbed, event->button.time);
                    this->grabbed = NULL;
                }

                sp_canvas_item_grab(SP_CANVAS_ITEM(desktop->acetate),
                                    GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_BUTTON_PRESS_MASK,
                                    NULL, event->button.time);

                this->grabbed = SP_CANVAS_ITEM(desktop->acetate);

                // remember what modifiers were on before button press
                this->button_press_shift = (event->button.state & GDK_SHIFT_MASK) ? true : false;
                this->button_press_ctrl = (event->button.state & GDK_CONTROL_MASK) ? true : false;
                this->button_press_alt = (event->button.state & GDK_MOD1_MASK) ? true : false;

                this->moved = FALSE;

                rb_escaped = drag_escaped = 0;

                ret = TRUE;
            } else if (event->button.button == 3) {
                // right click; do not eat it so that right-click menu can appear, but cancel dragging & rubberband
                this->sp_select_context_abort();
            }
            break;

        case GDK_MOTION_NOTIFY:
        {
            if (is_cycling) {
                moved_while_cycling = true;
            }
            
			tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

            if ((event->motion.state & GDK_BUTTON1_MASK) && !this->space_panning) {
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

                if (this->button_press_ctrl || (this->button_press_alt && !this->button_press_shift && !selection->isEmpty())) {
                    // if it's not click and ctrl or alt was pressed (the latter with some selection
                    // but not with shift) we want to drag rather than rubberband
                    this->dragging = TRUE;

                    GdkWindow* window = gtk_widget_get_window (GTK_WIDGET (sp_desktop_canvas(desktop)));

                    gdk_window_set_cursor(window, CursorSelectDragging);

                    desktop->canvas->forceFullRedrawAfterInterruptions(5);
                }

                if (this->dragging) {
                    /* User has dragged fast, so we get events on root (lauris)*/
                    // not only that; we will end up here when ctrl-dragging as well
                    // and also when we started within tolerance, but trespassed tolerance outside of item
                    Inkscape::Rubberband::get(desktop)->stop();
                    this->defaultMessageContext()->clear();

                    item_at_point = desktop->getItemAtPoint(Geom::Point(event->button.x, event->button.y), FALSE);

                    if (!item_at_point) { // if no item at this point, try at the click point (bug 1012200)
                        item_at_point = desktop->getItemAtPoint(Geom::Point(xp, yp), FALSE);
                    }

                    if (item_at_point || this->moved || this->button_press_alt) {
                        // drag only if starting from an item, or if something is already grabbed, or if alt-dragging
                        if (!this->moved) {
                            item_in_group = desktop->getItemAtPoint(Geom::Point(event->button.x, event->button.y), TRUE);
                            group_at_point = desktop->getGroupAtPoint(Geom::Point(event->button.x, event->button.y));

                            if (SP_IS_LAYER(selection->single())) {
                                group_at_point = SP_GROUP(selection->single());
                            }

                            // group-at-point is meant to be topmost item if it's a group,
                            // not topmost group of all items at point
                            if (group_at_point != item_in_group &&
                                !(group_at_point && item_at_point &&
                                  group_at_point->isAncestorOf(item_at_point))) {
                                group_at_point = NULL;
                            }

                            // if neither a group nor an item (possibly in a group) at point are selected, set selection to the item at point
                            if ((!item_in_group || !selection->includes(item_in_group)) &&
                                (!group_at_point || !selection->includes(group_at_point))
                                && !this->button_press_alt) {
                                // select what is under cursor
                                if (!_seltrans->isEmpty()) {
                                    _seltrans->resetState();
                                }

                                // when simply ctrl-dragging, we don't want to go into groups
                                if (item_at_point && !selection->includes(item_at_point)) {
                                    selection->set(item_at_point);
                                }
                            } // otherwise, do not change selection so that dragging selected-within-group items, as well as alt-dragging, is possible

                            _seltrans->grab(p, -1, -1, FALSE, TRUE);
                            this->moved = TRUE;
                        }

                        if (!_seltrans->isEmpty()) {
                            _seltrans->moveTo(p, event->button.state);
                        }

                        desktop->scroll_to_point(p);
                        gobble_motion_events(GDK_BUTTON1_MASK);
                        ret = TRUE;
                    } else {
                        this->dragging = FALSE;
                        sp_event_context_discard_delayed_snap_event(this);
                        desktop->canvas->endForcedFullRedraws();
                    }
                } else {
                    if (Inkscape::Rubberband::get(desktop)->is_started()) {
                        Inkscape::Rubberband::get(desktop)->move(p);

                        if (Inkscape::Rubberband::get(desktop)->getMode() == RUBBERBAND_MODE_TOUCHPATH) {
                            this->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Draw over</b> objects to select them; release <b>Alt</b> to switch to rubberband selection"));
                        } else {
                            this->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE, _("<b>Drag around</b> objects to select them; press <b>Alt</b> to switch to touch selection"));
                        }

                        gobble_motion_events(GDK_BUTTON1_MASK);
                    }
                }
            }
            break;
        }
        case GDK_BUTTON_RELEASE:
            xp = yp = 0;

            if ((event->button.button == 1) && (this->grabbed) && !this->space_panning) {
                if (this->dragging) {
                    GdkWindow* window;

                    if (this->moved) {
                        // item has been moved
                        _seltrans->ungrab();
                        this->moved = FALSE;
#ifdef WITH_DBUS
                        dbus_send_ping(desktop, this->item);
#endif
                    } else if (this->item && !drag_escaped) {
                        // item has not been moved -> simply a click, do selecting
                        if (!selection->isEmpty()) {
                            if (event->button.state & GDK_SHIFT_MASK) {
                                // with shift, toggle selection
                                _seltrans->resetState();
                                selection->toggle(this->item);
                            } else {
                                SPObject* single = selection->single();
                                // without shift, increase state (i.e. toggle scale/rotation handles)
                                if (selection->includes(this->item)) {
                                    _seltrans->increaseState();
                                } else if (SP_IS_LAYER(single) && single->isAncestorOf(this->item)) {
                                    _seltrans->increaseState();
                                } else {
                                    _seltrans->resetState();
                                    selection->set(this->item);
                                }
                            }
                        } else { // simple or shift click, no previous selection
                            _seltrans->resetState();
                            selection->set(this->item);
                        }
                    }

                    this->dragging = FALSE;
                    window = gtk_widget_get_window (GTK_WIDGET (sp_desktop_canvas(desktop)));

                    gdk_window_set_cursor(window, CursorSelectMouseover);
                    sp_event_context_discard_delayed_snap_event(this);
                    desktop->canvas->endForcedFullRedraws();

                    if (this->item) {
                        sp_object_unref( this->item, NULL);
                    }

                    this->item = NULL;
                } else {
                    Inkscape::Rubberband *r = Inkscape::Rubberband::get(desktop);

                    if (r->is_started() && !within_tolerance) {
                        // this was a rubberband drag
                        GSList *items = NULL;

                        if (r->getMode() == RUBBERBAND_MODE_RECT) {
                            Geom::OptRect const b = r->getRectangle();
                            items = sp_desktop_document(desktop)->getItemsInBox(desktop->dkey, *b);
                        } else if (r->getMode() == RUBBERBAND_MODE_TOUCHPATH) {
                            items = sp_desktop_document(desktop)->getItemsAtPoints(desktop->dkey, r->getPoints());
                        }

                        _seltrans->resetState();
                        r->stop();
                        this->defaultMessageContext()->clear();

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

                        if (this->button_press_shift && !rb_escaped && !drag_escaped) {
                            // this was a shift+click or alt+shift+click, select what was clicked upon
                            this->button_press_shift = false;

                            if (this->button_press_ctrl) {
                                // go into groups, honoring Alt
                                item = sp_event_context_find_item (desktop,
                                                   Geom::Point(event->button.x, event->button.y), event->button.state & GDK_MOD1_MASK, TRUE);
                                this->button_press_ctrl = FALSE;
                            } else {
                                // don't go into groups, honoring Alt
                                item = sp_event_context_find_item (desktop,
                                                   Geom::Point(event->button.x, event->button.y), event->button.state & GDK_MOD1_MASK, FALSE);
                            }

                            if (item) {
                                selection->toggle(item);
                                item = NULL;
                            }

                        } else if ((this->button_press_ctrl || this->button_press_alt) && !rb_escaped && !drag_escaped) { // ctrl+click, alt+click
                            item = sp_event_context_find_item (desktop,
                                         Geom::Point(event->button.x, event->button.y), this->button_press_alt, this->button_press_ctrl);

                            this->button_press_ctrl = FALSE;
                            this->button_press_alt = FALSE;

                            if (item) {
                                if (selection->includes(item)) {
                                    _seltrans->increaseState();
                                } else {
                                    _seltrans->resetState();
                                    selection->set(item);
                                }

                                item = NULL;
                            }
                        } else { // click without shift, simply deselect, unless with Alt or something was cancelled
                            if (!selection->isEmpty()) {
                                if (!(rb_escaped) && !(drag_escaped) && !(event->button.state & GDK_MOD1_MASK)) {
                                    selection->clear();
                                }

                                rb_escaped = 0;
                                ret = TRUE;
                            }
                        }
                    }

                    ret = TRUE;
                }

                if (this->grabbed) {
                    sp_canvas_item_ungrab(this->grabbed, event->button.time);
                    this->grabbed = NULL;
                }

                desktop->updateNow();
            }

            if (event->button.button == 1) {
                Inkscape::Rubberband::get(desktop)->stop(); // might have been started in another tool!
            }

            this->button_press_shift = false;
            this->button_press_ctrl = false;
            this->button_press_alt = false;
            break;

        case GDK_SCROLL: {
            GdkEventScroll *scroll_event = (GdkEventScroll*) event;

            if (scroll_event->state & GDK_MOD1_MASK) { // alt modified pressed
                if (moved_while_cycling) {
                    moved_while_cycling = false;
                    this->sp_select_context_reset_opacities();
                }

                is_cycling = true;
        
                bool shift_pressed = scroll_event->state & GDK_SHIFT_MASK;

                /* Rebuild list of items underneath the mouse pointer */
                Geom::Point p = desktop->d2w(desktop->point());
                SPItem *item = desktop->getItemAtPoint(p, true, NULL);

                // Save pointer to current cycle-item so that we can find it again later, in the freshly built list
                SPItem *tmp_cur_item = this->cycling_cur_item ? SP_ITEM(this->cycling_cur_item->data) : NULL;
                g_list_free(this->cycling_items);
                this->cycling_items = NULL;
                this->cycling_cur_item = NULL;

                while(item != NULL) {
                    this->cycling_items = g_list_append(this->cycling_items, item);
                    item = desktop->getItemAtPoint(p, true, item);
                }

                /* Compare current item list with item list during previous scroll ... */
                GList *l1, *l2;
                bool item_lists_differ = false;

                // Note that we can do an 'or' comparison in the loop because it is safe to call g_list_next with a NULL pointer.
                for (l1 = this->cycling_items, l2 = this->cycling_items_cmp; l1 != NULL || l2 != NULL; l1 = g_list_next(l1), l2 = g_list_next(l2)) {
                    if ((l1 !=NULL && l2 == NULL) || (l1 == NULL && l2 != NULL) || (l1->data != l2->data)) {
                        item_lists_differ = true;
                        break;
                    }
                }

                /* If list of items under mouse pointer hasn't changed ... */
                if (!item_lists_differ) {
                    // ... find current item in the freshly built list and continue cycling ...
                    // TODO: This wouldn't be necessary if cycling_cur_item pointed to an element of cycling_items_cmp instead
                    this->cycling_cur_item = g_list_find(this->cycling_items, tmp_cur_item);
                    g_assert(this->cycling_cur_item != NULL || this->cycling_items == NULL);
                } else {
                    // ... otherwise reset opacities for outdated items ...
                    Inkscape::DrawingItem *arenaitem;

                    for(GList *l = this->cycling_items_cmp; l != NULL; l = l->next) {
                        arenaitem = SP_ITEM(l->data)->get_arenaitem(desktop->dkey);
                        arenaitem->setOpacity(1.0);
                        //if (!shift_pressed && !g_list_find(this->cycling_items_selected_before, SP_ITEM(l->data)) && selection->includes(SP_ITEM(l->data)))
                        if (!g_list_find(this->cycling_items_selected_before, SP_ITEM(l->data)) && selection->includes(SP_ITEM(l->data))) {
                            selection->remove(SP_ITEM(l->data));
                        }
                    }

                    // ... clear the lists ...
                    g_list_free(this->cycling_items_cmp);
                    g_list_free(this->cycling_items_selected_before);

                    this->cycling_items_cmp = NULL;
                    this->cycling_items_selected_before = NULL;
                    this->cycling_cur_item = NULL;

                    // ... and rebuild them with the new items.
                    this->cycling_items_cmp = g_list_copy(this->cycling_items);
                    SPItem *item;

                    for(GList *l = this->cycling_items; l != NULL; l = l->next) {
                        item = SP_ITEM(l->data);
                        arenaitem = item->get_arenaitem(desktop->dkey);
                        arenaitem->setOpacity(0.3);

                        if (selection->includes(item)) {
                            // already selected items are stored separately, too
                            this->cycling_items_selected_before = g_list_append(this->cycling_items_selected_before, item);
                        }
                    }

                    // set the current item to the bottommost one so that the cycling step below re-starts at the top
                    this->cycling_cur_item = g_list_last(this->cycling_items);
                }

                this->cycling_wrap = prefs->getBool("/options/selection/cycleWrap", true);

                // Cycle through the items underneath the mouse pointer, one-by-one
                this->sp_select_context_cycle_through_items(selection, scroll_event, shift_pressed);

                ret = TRUE;

        GtkWindow *w =GTK_WINDOW(gtk_widget_get_toplevel( GTK_WIDGET(desktop->canvas) ));
        if (w)
            {
            gtk_window_present(w);
            gtk_widget_grab_focus (GTK_WIDGET(desktop->canvas)); 
            }
            }
            break;
        }

        case GDK_KEY_PRESS: // keybindings for select context
			{
			{
        	guint keyval = get_group0_keyval(&event->key);
        	
                bool alt = ( MOD__ALT(event)
                                    || (keyval == GDK_KEY_Alt_L)
                                    || (keyval == GDK_KEY_Alt_R)
                                    || (keyval == GDK_KEY_Meta_L)
                                    || (keyval == GDK_KEY_Meta_R));

            if (!key_is_a_modifier (keyval)) {
                    this->defaultMessageContext()->clear();
            } else if (this->grabbed || _seltrans->isGrabbed()) {
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
                    sp_event_show_modifier_tip (this->defaultMessageContext(), event,
                                                _("<b>Ctrl</b>: click to select in groups; drag to move hor/vert"),
                                                _("<b>Shift</b>: click to toggle select; drag for rubberband selection"),
                                                _("<b>Alt</b>: click to select under; scroll mouse-wheel to cycle-select; drag to move selected or select by touch"));
                    
                    // if Alt and nonempty selection, show moving cursor ("move selected"):
                    if (alt && !selection->isEmpty() && !desktop->isWaitingCursor()) {
                    	GdkWindow* window = gtk_widget_get_window (GTK_WIDGET (sp_desktop_canvas(desktop)));

                        gdk_window_set_cursor(window, CursorSelectDragging);
                    }
                    //*/
                    break;
            }
            }

            gdouble const nudge = prefs->getDoubleLimited("/options/nudgedistance/value", 2, 0, 1000, "px"); // in px
            gdouble const offset = prefs->getDoubleLimited("/options/defaultscale/value", 2, 0, 1000, "px");
            int const snaps = prefs->getInt("/options/rotationsnapsperpi/value", 12);

            switch (get_group0_keyval (&event->key)) {
                case GDK_KEY_Left: // move selection left
                case GDK_KEY_KP_Left:
                    if (!MOD__CTRL(event)) { // not ctrl
                        gint mul = 1 + gobble_key_events( get_group0_keyval(&event->key), 0); // with any mask
                        
                        if (MOD__ALT(event)) { // alt
                            if (MOD__SHIFT(event)) {
                            	sp_selection_move_screen(sp_desktop_selection(desktop), mul*-10, 0); // shift
                            } else {
                            	sp_selection_move_screen(sp_desktop_selection(desktop), mul*-1, 0); // no shift
                            }
                        } else { // no alt
                            if (MOD__SHIFT(event)) {
                            	sp_selection_move(sp_desktop_selection(desktop), mul*-10*nudge, 0); // shift
                            } else {
                            	sp_selection_move(sp_desktop_selection(desktop), mul*-nudge, 0); // no shift
                            }
                        }
                        
                        ret = TRUE;
                    }
                    break;
                    
                case GDK_KEY_Up: // move selection up
                case GDK_KEY_KP_Up:
                    if (!MOD__CTRL(event)) { // not ctrl
                        gint mul = 1 + gobble_key_events(get_group0_keyval(&event->key), 0); // with any mask
                        
                        if (MOD__ALT(event)) { // alt
                            if (MOD__SHIFT(event)) {
                            	sp_selection_move_screen(sp_desktop_selection(desktop), 0, mul*10); // shift
                            } else {
                            	sp_selection_move_screen(sp_desktop_selection(desktop), 0, mul*1); // no shift
                            }
                        } else { // no alt
                            if (MOD__SHIFT(event)) {
                            	sp_selection_move(sp_desktop_selection(desktop), 0, mul*10*nudge); // shift
                            } else {
                            	sp_selection_move(sp_desktop_selection(desktop), 0, mul*nudge); // no shift
                            }
                        }
                        
                        ret = TRUE;
                    }
                    break;
                    
                case GDK_KEY_Right: // move selection right
                case GDK_KEY_KP_Right:
                    if (!MOD__CTRL(event)) { // not ctrl
                        gint mul = 1 + gobble_key_events(get_group0_keyval(&event->key), 0); // with any mask
                        
                        if (MOD__ALT(event)) { // alt
                            if (MOD__SHIFT(event)) {
                            	sp_selection_move_screen(sp_desktop_selection(desktop), mul*10, 0); // shift
                            } else {
                            	sp_selection_move_screen(sp_desktop_selection(desktop), mul*1, 0); // no shift
                            }
                        } else { // no alt
                            if (MOD__SHIFT(event)) {
                            	sp_selection_move(sp_desktop_selection(desktop), mul*10*nudge, 0); // shift
                            } else {
                            	sp_selection_move(sp_desktop_selection(desktop), mul*nudge, 0); // no shift
                            }
                        }
                        
                        ret = TRUE;
                    }
                    break;
                    
                case GDK_KEY_Down: // move selection down
                case GDK_KEY_KP_Down:
                    if (!MOD__CTRL(event)) { // not ctrl
                        gint mul = 1 + gobble_key_events(get_group0_keyval(&event->key), 0); // with any mask
                        
                        if (MOD__ALT(event)) { // alt
                            if (MOD__SHIFT(event)) {
                            	sp_selection_move_screen(sp_desktop_selection(desktop), 0, mul*-10); // shift
                            } else {
                            	sp_selection_move_screen(sp_desktop_selection(desktop), 0, mul*-1); // no shift
                            }
                        } else { // no alt
                            if (MOD__SHIFT(event)) {
                            	sp_selection_move(sp_desktop_selection(desktop), 0, mul*-10*nudge); // shift
                            } else {
                            	sp_selection_move(sp_desktop_selection(desktop), 0, mul*-nudge); // no shift
                            }
                        }
                        
                        ret = TRUE;
                    }
                    break;
                    
                case GDK_KEY_Escape:
                    if (!this->sp_select_context_abort()) {
                        selection->clear();
                    }
                    
                    ret = TRUE;
                    break;

                case GDK_KEY_a:
                case GDK_KEY_A:
                    if (MOD__CTRL_ONLY(event)) {
                        sp_edit_select_all(desktop);
                        ret = TRUE;
                    }
                    break;
                    
                case GDK_KEY_space:
                    /* stamping mode: show outline mode moving */
                    /* FIXME: Is next condition ok? (lauris) */
                    if (this->dragging && this->grabbed) {
                        _seltrans->stamp();
                        ret = TRUE;
                    }
                    break;
                    
                case GDK_KEY_x:
                case GDK_KEY_X:
                    if (MOD__ALT_ONLY(event)) {
                        desktop->setToolboxFocusTo ("altx");
                        ret = TRUE;
                    }
                    break;
                    
                case GDK_KEY_bracketleft:
                    if (MOD__ALT(event)) {
                        gint mul = 1 + gobble_key_events(get_group0_keyval(&event->key), 0); // with any mask
                        sp_selection_rotate_screen(selection, mul*1);
                    } else if (MOD__CTRL(event)) {
                        sp_selection_rotate(selection, 90);
                    } else if (snaps) {
                        sp_selection_rotate(selection, 180.0/snaps);
                    }
                    
                    ret = TRUE;
                    break;
                    
                case GDK_KEY_bracketright:
                    if (MOD__ALT(event)) {
                        gint mul = 1 + gobble_key_events(get_group0_keyval(&event->key), 0); // with any mask
                        sp_selection_rotate_screen(selection, -1*mul);
                    } else if (MOD__CTRL(event)) {
                        sp_selection_rotate(selection, -90);
                    } else if (snaps) {
                        sp_selection_rotate(selection, -180.0/snaps);
                    }
                    
                    ret = TRUE;
                    break;
                    
                case GDK_KEY_less:
                case GDK_KEY_comma:
                    if (MOD__ALT(event)) {
                        gint mul = 1 + gobble_key_events(get_group0_keyval(&event->key), 0); // with any mask
                        sp_selection_scale_screen(selection, -2*mul);
                    } else if (MOD__CTRL(event)) {
                        sp_selection_scale_times(selection, 0.5);
                    } else {
                        gint mul = 1 + gobble_key_events(get_group0_keyval(&event->key), 0); // with any mask
                        sp_selection_scale(selection, -offset*mul);
                    }
                    
                    ret = TRUE;
                    break;
                    
                case GDK_KEY_greater:
                case GDK_KEY_period:
                    if (MOD__ALT(event)) {
                        gint mul = 1 + gobble_key_events(get_group0_keyval(&event->key), 0); // with any mask
                        sp_selection_scale_screen(selection, 2*mul);
                    } else if (MOD__CTRL(event)) {
                        sp_selection_scale_times(selection, 2);
                    } else {
                        gint mul = 1 + gobble_key_events(get_group0_keyval(&event->key), 0); // with any mask
                        sp_selection_scale(selection, offset*mul);
                    }
                    
                    ret = TRUE;
                    break;
                    
                case GDK_KEY_Return:
                    if (MOD__CTRL_ONLY(event)) {
                        if (selection->singleItem()) {
                            SPItem *clicked_item = selection->singleItem();
                            
                            if ( SP_IS_GROUP(clicked_item) || SP_IS_BOX3D(clicked_item)) { // enter group or a 3D box
                                desktop->setCurrentLayer(reinterpret_cast<SPObject *>(clicked_item));
                                sp_desktop_selection(desktop)->clear();
                            } else {
                                this->desktop->messageStack()->flash(Inkscape::NORMAL_MESSAGE, _("Selected object is not a group. Cannot enter."));
                            }
                        }
                        
                        ret = TRUE;
                    }
                    break;
                    
                case GDK_KEY_BackSpace:
                    if (MOD__CTRL_ONLY(event)) {
                        sp_select_context_up_one_layer(desktop);
                        ret = TRUE;
                    }
                    break;
                    
                case GDK_KEY_s:
                case GDK_KEY_S:
                    if (MOD__SHIFT_ONLY(event)) {
                        if (!selection->isEmpty()) {
                            _seltrans->increaseState();
                        }
                        
                        ret = TRUE;
                    }
                    break;
                    
                case GDK_KEY_g:
                case GDK_KEY_G:
                    if (MOD__SHIFT_ONLY(event)) {
                        sp_selection_to_guides(desktop);
                        ret = true;
                    }
                    break;
                    
                default:
                    break;
            }
            break;
			}
        case GDK_KEY_RELEASE: {
            guint keyval = get_group0_keyval(&event->key);
            if (key_is_a_modifier (keyval)) {
                this->defaultMessageContext()->clear();
            }
            
            bool alt = ( MOD__ALT(event)
                         || (keyval == GDK_KEY_Alt_L)
                         || (keyval == GDK_KEY_Alt_R)
                         || (keyval == GDK_KEY_Meta_L)
                         || (keyval == GDK_KEY_Meta_R));

            if (Inkscape::Rubberband::get(desktop)->is_started()) {
                // if Alt then change cursor to moving cursor:
                if (alt) {
                    Inkscape::Rubberband::get(desktop)->setMode(RUBBERBAND_MODE_RECT);
                }
            } else {
                if (alt) {
                    // quit cycle-selection and reset opacities
                    if (is_cycling) {
                        this->sp_select_context_reset_opacities();
                        is_cycling = false;
                    }
                }
            }

            // set cursor to default.
            if (!desktop->isWaitingCursor()) {
                // Do we need to reset the cursor here on key release ?
                //GdkWindow* window = gtk_widget_get_window (GTK_WIDGET (sp_desktop_canvas(desktop)));
                //gdk_window_set_cursor(window, event_context->cursor);
            }
            break;
        }
        default:
            break;
    }

    if (!ret) {
    	ret = ToolBase::root_handler(event);
    }

    return ret;
}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

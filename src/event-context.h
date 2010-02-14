#ifndef __SP_EVENT_CONTEXT_H__
#define __SP_EVENT_CONTEXT_H__

/** \file
 * SPEventContext: base class for event processors
 *
 * This is per desktop object, which (its derivatives) implements
 * different actions bound to mouse events.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib-object.h>
#include <gdk/gdktypes.h>
#include <gdk/gdkevents.h>
#include "knot.h"

#include "2geom/forward.h"
#include "preferences.h"

struct GrDrag;
struct SPDesktop;
struct SPItem;
class ShapeEditor;
struct SPEventContext;

namespace Inkscape {
    class MessageContext;
    class SelCue;
    namespace XML {
        class Node;
    }
}

gboolean sp_event_context_snap_watchdog_callback(gpointer data);
void sp_event_context_discard_delayed_snap_event(SPEventContext *ec);

class DelayedSnapEvent
{
public:
    enum DelayedSnapEventOrigin {
        UNDEFINED_HANDLER = 0,
        EVENTCONTEXT_ROOT_HANDLER,
        EVENTCONTEXT_ITEM_HANDLER,
        KNOT_HANDLER,
        CONTROL_POINT_HANDLER,
        GUIDE_HANDLER,
        GUIDE_HRULER,
        GUIDE_VRULER
    };

    DelayedSnapEvent(SPEventContext *event_context, gpointer const dse_item, gpointer dse_item2, GdkEventMotion const *event, DelayedSnapEvent::DelayedSnapEventOrigin const origin)
    : _timer_id(0), _event(NULL), _item(dse_item), _item2(dse_item2), _origin(origin), _event_context(event_context)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        double value = prefs->getDoubleLimited("/options/snapdelay/value", 0, 0, 1000);
        _timer_id = g_timeout_add(value, &sp_event_context_snap_watchdog_callback, this);
        _event = gdk_event_copy((GdkEvent*) event);
        ((GdkEventMotion *)_event)->time = GDK_CURRENT_TIME;
    }

    ~DelayedSnapEvent()    {
        if (_timer_id > 0) g_source_remove(_timer_id); // Kill the watchdog
        if (_event != NULL) gdk_event_free(_event); // Remove the copy of the original event
    }

    SPEventContext* getEventContext() {return _event_context;}
    DelayedSnapEventOrigin getOrigin() {return _origin;}
    GdkEvent* getEvent() {return _event;}
    gpointer getItem() {return _item;}
    gpointer getItem2() {return _item2;}

private:
    guint _timer_id;
    GdkEvent* _event;
    gpointer _item;
    gpointer _item2;
    DelayedSnapEventOrigin _origin;
    SPEventContext* _event_context;
};

void sp_event_context_snap_delay_handler(SPEventContext *ec, gpointer const dse_item, gpointer const dse_item2, GdkEventMotion *event, DelayedSnapEvent::DelayedSnapEventOrigin origin);

/**
 * Base class for Event processors.
 */
struct SPEventContext : public GObject {
    void enableSelectionCue (bool enable=true);
    void enableGrDrag (bool enable=true);

    /// Desktop eventcontext stack
    SPEventContext *next;
    unsigned key;
    SPDesktop *desktop;
    Inkscape::Preferences::Observer *pref_observer;
    gchar const *const *cursor_shape;
    gint hot_x, hot_y; ///< indicates the cursor's hot spot
    GdkCursor *cursor;

    gint xp, yp;           ///< where drag started
    gint tolerance;
    bool within_tolerance;  ///< are we still within tolerance of origin

    SPItem *item_to_select; ///< the item where mouse_press occurred, to
                            ///< be selected if this is a click not drag

    Inkscape::MessageContext *defaultMessageContext() {
        return _message_context;
    }

    Inkscape::MessageContext *_message_context;

    Inkscape::SelCue *_selcue;

    GrDrag *_grdrag;
    GrDrag *get_drag () {return _grdrag;}

    ShapeEditor* shape_editor;

    bool space_panning;

    DelayedSnapEvent *_delayed_snap_event;
    bool _dse_callback_in_process;
};

/**
 * The SPEvent vtable.
 */
struct SPEventContextClass : public GObjectClass {
    void (* setup)(SPEventContext *ec);
    void (* finish)(SPEventContext *ec);
    void (* set)(SPEventContext *ec, Inkscape::Preferences::Entry *val);
    void (* activate)(SPEventContext *ec);
    void (* deactivate)(SPEventContext *ec);
    gint (* root_handler)(SPEventContext *ec, GdkEvent *event);
    gint (* item_handler)(SPEventContext *ec, SPItem *item, GdkEvent *event);
};

#define SP_EVENT_CONTEXT_DESKTOP(e) (SP_EVENT_CONTEXT(e)->desktop)
#define SP_EVENT_CONTEXT_DOCUMENT(e) ((SP_EVENT_CONTEXT_DESKTOP(e))->doc())

#define SP_EVENT_CONTEXT_STATIC 0

SPEventContext *sp_event_context_new(GType type, SPDesktop *desktop, gchar const *pref_path, unsigned key);
void sp_event_context_finish(SPEventContext *ec);
void sp_event_context_read(SPEventContext *ec, gchar const *key);
void sp_event_context_activate(SPEventContext *ec);
void sp_event_context_deactivate(SPEventContext *ec);

gint sp_event_context_root_handler(SPEventContext *ec, GdkEvent *event);
gint sp_event_context_virtual_root_handler(SPEventContext *ec, GdkEvent *event);
gint sp_event_context_item_handler(SPEventContext *ec, SPItem *item, GdkEvent *event);
gint sp_event_context_virtual_item_handler(SPEventContext *ec, SPItem *item, GdkEvent *event);

void sp_event_root_menu_popup(SPDesktop *desktop, SPItem *item, GdkEvent *event);

gint gobble_key_events(guint keyval, gint mask);
gint gobble_motion_events(gint mask);

void sp_event_context_update_cursor(SPEventContext *ec);

void sp_event_show_modifier_tip(Inkscape::MessageContext *message_context, GdkEvent *event,
                                gchar const *ctrl_tip, gchar const *shift_tip, gchar const *alt_tip);

guint get_group0_keyval(GdkEventKey *event);

SPItem *sp_event_context_find_item (SPDesktop *desktop, Geom::Point const &p, bool select_under, bool into_groups);
SPItem *sp_event_context_over_item (SPDesktop *desktop, SPItem *item, Geom::Point const &p);

ShapeEditor *sp_event_context_get_shape_editor (SPEventContext *ec);
bool sp_event_context_knot_mouseover(SPEventContext *ec);

void ec_shape_event_attr_changed(Inkscape::XML::Node *shape_repr,
                                     gchar const *name, gchar const *old_value, gchar const *new_value,
                                 bool const is_interactive, gpointer const data);

void event_context_print_event_info(GdkEvent *event, bool print_return = true);

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

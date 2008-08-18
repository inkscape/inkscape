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

#include <libnr/nr-forward.h>

struct GrDrag;
struct SPDesktop;
struct SPItem;
class KnotHolder;
class ShapeEditor;

namespace Inkscape {
    class MessageContext;
    class SelCue;
    namespace XML {
        class Node;
    }
}

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
    Inkscape::XML::Node *prefs_repr;
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

    KnotHolder *shape_knot_holder;
    Inkscape::XML::Node *shape_repr;

    bool space_panning;
};

/**
 * The SPEvent vtable.
 */
struct SPEventContextClass : public GObjectClass {
    void (* setup)(SPEventContext *ec);
    void (* finish)(SPEventContext *ec);
    void (* set)(SPEventContext *ec, gchar const *key, gchar const *val);
    void (* activate)(SPEventContext *ec);
    void (* deactivate)(SPEventContext *ec);
    gint (* root_handler)(SPEventContext *ec, GdkEvent *event);
    gint (* item_handler)(SPEventContext *ec, SPItem *item, GdkEvent *event);
};

#define SP_EVENT_CONTEXT_DESKTOP(e) (SP_EVENT_CONTEXT(e)->desktop)
#define SP_EVENT_CONTEXT_DOCUMENT(e) ((SP_EVENT_CONTEXT_DESKTOP(e))->doc())

#define SP_EVENT_CONTEXT_STATIC 0

SPEventContext *sp_event_context_new(GType type, SPDesktop *desktop, Inkscape::XML::Node *prefs_repr, unsigned key);
void sp_event_context_finish(SPEventContext *ec);
void sp_event_context_read(SPEventContext *ec, gchar const *key);
void sp_event_context_activate(SPEventContext *ec);
void sp_event_context_deactivate(SPEventContext *ec);

gint sp_event_context_root_handler(SPEventContext *ec, GdkEvent *event);
gint sp_event_context_item_handler(SPEventContext *ec, SPItem *item, GdkEvent *event);

void sp_event_root_menu_popup(SPDesktop *desktop, SPItem *item, GdkEvent *event);

gint gobble_key_events(guint keyval, gint mask);
gint gobble_motion_events(gint mask);

void sp_event_context_update_cursor(SPEventContext *ec);

void sp_event_show_modifier_tip(Inkscape::MessageContext *message_context, GdkEvent *event,
                                gchar const *ctrl_tip, gchar const *shift_tip, gchar const *alt_tip);

guint get_group0_keyval(GdkEventKey *event);

SPItem *sp_event_context_find_item (SPDesktop *desktop, NR::Point const p, bool select_under, bool into_groups);
SPItem *sp_event_context_over_item (SPDesktop *desktop, SPItem *item, NR::Point const p);

ShapeEditor *sp_event_context_get_shape_editor (SPEventContext *ec);

void ec_shape_event_attr_changed(Inkscape::XML::Node *shape_repr,
                                     gchar const *name, gchar const *old_value, gchar const *new_value,
                                 bool const is_interactive, gpointer const data);

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

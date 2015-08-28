#ifndef SEEN_SP_EVENT_CONTEXT_H
#define SEEN_SP_EVENT_CONTEXT_H

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <stddef.h>
#include <string>

#include <2geom/point.h>
#include <gdk/gdk.h>
#include <glib-object.h>
#include <sigc++/trackable.h>
#include "knot.h"

#include "preferences.h"

namespace Glib {
    class ustring;
}

class GrDrag;
class SPDesktop;
class SPItem;

namespace Inkscape {
    class MessageContext;
    class SelCue;
}

#define SP_EVENT_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::ToolBase*>((Inkscape::UI::Tools::ToolBase*)obj))
#define SP_IS_EVENT_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::ToolBase*>((const Inkscape::UI::Tools::ToolBase*)obj) != NULL)

namespace Inkscape {
namespace UI {

class ShapeEditor;

namespace Tools {

class ToolBase;

gboolean sp_event_context_snap_watchdog_callback(gpointer data);
void sp_event_context_discard_delayed_snap_event(ToolBase *ec);

class DelayedSnapEvent {
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

    DelayedSnapEvent(ToolBase *event_context, gpointer const dse_item, gpointer dse_item2, GdkEventMotion const *event, DelayedSnapEvent::DelayedSnapEventOrigin const origin)
        : _timer_id(0)
        , _event(NULL)
        , _item(dse_item)
        , _item2(dse_item2)
        , _origin(origin)
        , _event_context(event_context)
    {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        double value = prefs->getDoubleLimited("/options/snapdelay/value", 0, 0, 1000);

        // We used to have this specified in milliseconds; this has changed to seconds now for consistency's sake
        if (value > 1) { // Apparently we have an old preference file, this value must have been in milliseconds;
            value = value / 1000.0; // now convert this value to seconds
        }

        _timer_id = g_timeout_add(value*1000.0, &sp_event_context_snap_watchdog_callback, this);
        _event = gdk_event_copy((GdkEvent*) event);

        ((GdkEventMotion *)_event)->time = GDK_CURRENT_TIME;
    }

    ~DelayedSnapEvent()    {
        if (_timer_id > 0) g_source_remove(_timer_id); // Kill the watchdog
        if (_event != NULL) gdk_event_free(_event); // Remove the copy of the original event
    }

    ToolBase* getEventContext() {
        return _event_context;
    }

    DelayedSnapEventOrigin getOrigin() {
        return _origin;
    }

    GdkEvent* getEvent() {
        return _event;
    }

    gpointer getItem() {
        return _item;
    }

    gpointer getItem2() {
        return _item2;
    }

private:
    guint _timer_id;
    GdkEvent* _event;
    gpointer _item;
    gpointer _item2;
    DelayedSnapEventOrigin _origin;
    ToolBase* _event_context;
};

void sp_event_context_snap_delay_handler(ToolBase *ec, gpointer const dse_item, gpointer const dse_item2, GdkEventMotion *event, DelayedSnapEvent::DelayedSnapEventOrigin origin);


/**
 * Base class for Event processors.
 *
 * This is per desktop object, which (its derivatives) implements
 * different actions bound to mouse events.
 *
 * ToolBase is an abstract base class of all tools. As the name
 * indicates, event context implementations process UI events (mouse
 * movements and keypresses) and take actions (like creating or modifying
 * objects).  There is one event context implementation for each tool,
 * plus few abstract base classes. Writing a new tool involves
 * subclassing ToolBase.
 */
class ToolBase
    : public sigc::trackable
{
public:
    void enableSelectionCue (bool enable=true);
    void enableGrDrag (bool enable=true);
    bool deleteSelectedDrag(bool just_one);

    ToolBase(gchar const *const *cursor_shape, gint hot_x, gint hot_y, bool uses_snap = true);

    virtual ~ToolBase();

    Inkscape::Preferences::Observer *pref_observer;
    GdkCursor *cursor;

    gint xp, yp;           ///< where drag started
    gint tolerance;

    bool within_tolerance;  ///< are we still within tolerance of origin

    SPItem *item_to_select; ///< the item where mouse_press occurred, to
                            ///< be selected if this is a click not drag

    Inkscape::MessageContext *defaultMessageContext() {
        return message_context;
    }

    Inkscape::MessageContext *message_context;

    Inkscape::SelCue *_selcue;

    GrDrag *_grdrag;

    GrDrag *get_drag () {
        return _grdrag;
    }

    ShapeEditor* shape_editor;

    bool space_panning;

    DelayedSnapEvent *_delayed_snap_event;
    bool _dse_callback_in_process;

	virtual void setup();
	virtual void finish();

	// Is called by our pref_observer if a preference has been changed.
	virtual void set(const Inkscape::Preferences::Entry& val);

	virtual bool root_handler(GdkEvent* event);
	virtual bool item_handler(SPItem* item, GdkEvent* event);

	virtual const std::string& getPrefsPath() = 0;

	/**
	 * An observer that relays pref changes to the derived classes.
	 */
	class ToolPrefObserver: public Inkscape::Preferences::Observer {
	public:
	    ToolPrefObserver(Glib::ustring const &path, ToolBase *ec)
	        : Inkscape::Preferences::Observer(path)
	        , ec(ec)
	    {
	    }

	    virtual void notify(Inkscape::Preferences::Entry const &val) {
	    	ec->set(val);
	    }

	private:
	    ToolBase * const ec;
	};

	SPDesktop const& getDesktop() const;


//protected:
	void sp_event_context_update_cursor();

	SPDesktop *desktop;
    bool _uses_snap; // TODO: make protected or private

protected:
	/// An xpm containing the shape of the tool's cursor.
    gchar const *const *cursor_shape;

    /// The cursor's hot spot
    gint hot_x, hot_y;

    bool sp_event_context_knot_mouseover() const;

private:
	ToolBase(const ToolBase&);
	ToolBase& operator=(const ToolBase&);

	void sp_event_context_set_cursor(GdkCursorType cursor_type);

};

void sp_event_context_read(ToolBase *ec, gchar const *key);

gint sp_event_context_root_handler(ToolBase *ec, GdkEvent *event);
gint sp_event_context_virtual_root_handler(ToolBase *ec, GdkEvent *event);
gint sp_event_context_item_handler(ToolBase *ec, SPItem *item, GdkEvent *event);
gint sp_event_context_virtual_item_handler(ToolBase *ec, SPItem *item, GdkEvent *event);

void sp_event_root_menu_popup(SPDesktop *desktop, SPItem *item, GdkEvent *event);

gint gobble_key_events(guint keyval, gint mask);
gint gobble_motion_events(gint mask);

void sp_event_show_modifier_tip(Inkscape::MessageContext *message_context, GdkEvent *event,
                                gchar const *ctrl_tip, gchar const *shift_tip, gchar const *alt_tip);

guint get_group0_keyval(GdkEventKey const *event);

SPItem *sp_event_context_find_item (SPDesktop *desktop, Geom::Point const &p, bool select_under, bool into_groups);
SPItem *sp_event_context_over_item (SPDesktop *desktop, SPItem *item, Geom::Point const &p);

void sp_toggle_dropper(SPDesktop *dt);

bool sp_event_context_knot_mouseover(ToolBase *ec);

} // namespace Tools
} // namespace UI
} // namespace Inkscape

#endif // SEEN_SP_EVENT_CONTEXT_H


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

/** @file
 * Desktop-bound visual control object - implementation
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <iostream>
#include <gdkmm.h>
#include <gtkmm.h>
#include <2geom/point.h>
#include "desktop.h"
#include "desktop-handles.h"
#include "display/snap-indicator.h"
#include "event-context.h"
#include "message-context.h"
#include "preferences.h"
#include "ui/tool/control-point.h"
#include "ui/tool/event-utils.h"

namespace Inkscape {
namespace UI {

// class and member documentation goes here...

/**
 * @class ControlPoint
 * @brief Draggable point, the workhorse of on-canvas editing.
 *
 * Control points (formerly known as knots) are graphical representations of some significant
 * point in the drawing. The drawing can be changed by dragging the point and the things that are
 * attached to it with the mouse. Example things that could be edited with draggable points
 * are gradient stops, the place where text is attached to a path, text kerns, nodes and handles
 * in a path, and many more. Control points use signals heavily - <b>read the libsigc++
 * tutorial on the wiki</b> before using this class.</b>
 *
 * @par Control point signals
 * @par
 * The control point has several signals which allow you to react to things that happen to it.
 * The most important singals are the grabbed, dragged, ungrabbed and moved signals.
 * When a drag happens, the order of emission is as follows:
 * - <tt>signal_grabbed</tt>
 * - <tt>signal_dragged</tt>
 * - <tt>signal_dragged</tt>
 * - <tt>signal_dragged</tt>
 * - ...
 * - <tt>signal_dragged</tt>
 * - <tt>signal_ungrabbed</tt>
 *
 * The control point can also respond to clicks and double clicks. On a double click,
 * <tt>signal_clicked</tt> is emitted, followed by <tt>signal_doubleclicked</tt>.
 *
 * A few signal usage hints if you can't be bothered to read the tutorial:
 * - If you want some other object or a global function to react to signals of a control point
 *   from some other object, and you want to access the control point that emitted the signal
 *   in the handler, use <tt>sigc::bind</tt> like this:
 *   @code
 *   void handle_clicked_signal(ControlPoint *point, int button);
 *   point->signal_clicked.connect(
 *       sigc::bind<0>( sigc::ptr_fun(handle_clicked_signal),
 *                      point ));
 *   @endcode
 * - You can ignore unneeded parameters using sigc::hide.
 * - If you want to get rid of the handlers added by constructors in superclasses,
 *   use the <tt>clear()</tt> method: @code signal_clicked.clear(); @endcode
 * - To connect at the front of the slot list instead of at the end, use:
 *   @code
 *   signal_clicked.slots().push_front(
 *       sigc::mem_fun(*this, &FunkyPoint::_clickedHandler));
 *   @endcode
 * - Note that calling <tt>slots()</tt> does not copy anything. You can disconnect
 *   and reorder slots by manipulating the elements of the slot list. The returned object is
 *   of type @verbatim (signal type)::slot_list @endverbatim.
 *
 * @par Which method to override?
 * @par
 * You might wonder which hook to use when you want to do things when the point is relocated.
 * Here are some tips:
 * - If the point is used to edit an object, override the move() method.
 * - If the point can usually be dragged wherever you like but can optionally be constrained
 *   to axes or the like, add a handler for <tt>signal_dragged</tt> that modifies its new
 *   position argument.
 * - If the point has additional canvas items tied to it (like handle lines), override
 *   the setPosition() method.
 */

/**
 * @var ControlPoint::signal_dragged
 * Emitted while dragging, but before moving the knot to new position.
 * Old position will always be the same as position() - there are two parameters
 * only for convenience.
 * - First parameter: old position, always equal to position()
 * - Second parameter: new position (after drag). This is passed as a non-const reference,
 *   so you can change it from the handler - that's how constrained dragging is implemented.
 * - Third parameter: motion event
 */

/**
 * @var ControlPoint::signal_clicked
 * Emitted when the control point is clicked, at mouse button release. The parameter contains
 * the event that caused the signal to be emitted. Your signal handler should return true
 * if the click had some effect. If it did nothing, return false. Improperly handling this signal
 * can cause the context menu not to appear when a control point is right-clicked.
 */

/**
 * @var ControlPoint::signal_doubleclicked
 * Emitted when the control point is doubleclicked, at mouse button release. The parameter
 * contains the event that caused the signal to be emitted. Your signal handler should return true
 * if the double click had some effect. If it did nothing, return false.
 */

/**
 * @var ControlPoint::signal_grabbed
 * Emitted when the control point is grabbed and a drag starts. The parameter contains
 * the causing event. Return true to prevent further processing. Because all control points
 * handle drag tolerance, <tt>signal_dragged</tt> will be emitted immediately after this signal
 * to move the point to its new position.
 */

/**
 * @var ControlPoint::signal_ungrabbed
 * Emitted when the control point finishes a drag. The parameter contains the event which
 * caused the signal, but it can be NULL if the grab was broken.
 */

/**
 * @enum ControlPoint::State
 * Enumeration representing the possible states of the control point, used to determine
 * its appearance.
 * @var ControlPoint::STATE_NORMAL
 *      Normal state
 * @var ControlPoint::STATE_MOUSEOVER
 *      Mouse is hovering over the control point
 * @var ControlPoint::STATE_CLICKED
 *      First mouse button pressed over the control point
 */

// Default colors for control points
static ControlPoint::ColorSet default_color_set = {
    {0xffffff00, 0x01000000}, // normal fill, stroke
    {0xff0000ff, 0x01000000}, // mouseover fill, stroke
    {0x0000ffff, 0x01000000}  // clicked fill, stroke
};

/** Holds the currently mouseovered control point. */
ControlPoint *ControlPoint::mouseovered_point = 0;

/** Emitted when the mouseovered point changes. The parameter is the new mouseovered point.
 * When a point ceases to be mouseovered, the parameter will be NULL. */
sigc::signal<void, ControlPoint*> ControlPoint::signal_mouseover_change;

/** Stores the window point over which the cursor was during the last mouse button press */
Geom::Point ControlPoint::_drag_event_origin(Geom::infinity(), Geom::infinity());

/** Stores the desktop point from which the last drag was initiated */
Geom::Point ControlPoint::_drag_origin(Geom::infinity(), Geom::infinity());

/** Events which should be captured when a handle is being dragged. */
int const ControlPoint::_grab_event_mask = (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
        GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_KEY_PRESS_MASK |
        GDK_KEY_RELEASE_MASK);

bool ControlPoint::_drag_initiated = false;
bool ControlPoint::_event_grab = false;

/** A color set which you can use to create an invisible control that can still receive events.
 * @relates ControlPoint */
ControlPoint::ColorSet invisible_cset = {
    {0x00000000, 0x00000000},
    {0x00000000, 0x00000000},
    {0x00000000, 0x00000000}
};

/**
 * Create a regular control point.
 * Derive to have constructors with a reasonable number of parameters.
 *
 * @param d Desktop for this control
 * @param initial_pos Initial position of the control point in desktop coordinates
 * @param anchor Where is the control point rendered relative to its desktop coordinates
 * @param shape Shape of the control point: square, diamond, circle...
 * @param size Pixel size of the visual representation
 * @param cset Colors of the point
 * @param group The canvas group the point's canvas item should be created in
 */
ControlPoint::ControlPoint(SPDesktop *d, Geom::Point const &initial_pos,
        Gtk::AnchorType anchor, SPCtrlShapeType shape,
        unsigned int size, ColorSet *cset, SPCanvasGroup *group)
    : _desktop (d)
    , _canvas_item (NULL)
    , _cset (cset ? cset : &default_color_set)
    , _state (STATE_NORMAL)
    , _position (initial_pos)
{
    _canvas_item = sp_canvas_item_new(
        group ? group : sp_desktop_controls (_desktop), SP_TYPE_CTRL,
        "anchor", (GtkAnchorType) anchor, "size", (gdouble) size, "shape", shape,
        "filled", TRUE, "fill_color", _cset->normal.fill,
        "stroked", TRUE, "stroke_color", _cset->normal.stroke,
        "mode", SP_CTRL_MODE_XOR, NULL);
    _commonInit();
}

/**
 * Create a control point with a pixbuf-based visual representation.
 *
 * @param d Desktop for this control
 * @param initial_pos Initial position of the control point in desktop coordinates
 * @param anchor Where is the control point rendered relative to its desktop coordinates
 * @param pixbuf Pixbuf to be used as the visual representation
 * @param cset Colors of the point
 * @param group The canvas group the point's canvas item should be created in
 */
ControlPoint::ControlPoint(SPDesktop *d, Geom::Point const &initial_pos,
        Gtk::AnchorType anchor, Glib::RefPtr<Gdk::Pixbuf> pixbuf,
        ColorSet *cset, SPCanvasGroup *group)
    : _desktop (d)
    , _canvas_item (NULL)
    , _cset(cset ? cset : &default_color_set)
    , _position (initial_pos)
{
    _canvas_item = sp_canvas_item_new(
        group ? group : sp_desktop_controls(_desktop), SP_TYPE_CTRL,
        "anchor", (GtkAnchorType) anchor, "size", (gdouble) pixbuf->get_width(),
        "shape", SP_CTRL_SHAPE_BITMAP, "pixbuf", pixbuf->gobj(),
        "filled", TRUE, "fill_color", _cset->normal.fill,
        "stroked", TRUE, "stroke_color", _cset->normal.stroke,
        "mode", SP_CTRL_MODE_XOR, NULL);
    _commonInit();
}

ControlPoint::~ControlPoint()
{
    // avoid storing invalid points in mouseovered_point
    if (this == mouseovered_point) {
        _clearMouseover();
    }

    g_signal_handler_disconnect(G_OBJECT(_canvas_item), _event_handler_connection);
    //sp_canvas_item_hide(_canvas_item);
    gtk_object_destroy(_canvas_item);
}

void ControlPoint::_commonInit()
{
    _event_handler_connection = g_signal_connect(G_OBJECT(_canvas_item), "event",
                                                 G_CALLBACK(_event_handler), this);
    SP_CTRL(_canvas_item)->moveto(_position);
}

/** Relocate the control point without side effects.
 * Overload this method only if there is an additional graphical representation
 * that must be updated (like the lines that connect handles to nodes). If you override it,
 * you must also call the superclass implementation of the method.
 * @todo Investigate whether this method should be protected */
void ControlPoint::setPosition(Geom::Point const &pos)
{
    _position = pos;
    SP_CTRL(_canvas_item)->moveto(pos);
}

/** Move the control point to new position with side effects.
 * This is called after each drag. Override this method if only some positions make sense
 * for a control point (like a point that must always be on a path and can't modify it),
 * or when moving a control point changes the positions of other points. */
void ControlPoint::move(Geom::Point const &pos)
{
    setPosition(pos);
}

/** Apply an arbitrary affine transformation to a control point. This is used
 * by ControlPointSelection, and is important for things like nodes with handles.
 * The default implementation simply moves the point according to the transform. */
void ControlPoint::transform(Geom::Matrix const &m) {
    move(position() * m);
}

bool ControlPoint::visible() const
{
    return sp_canvas_item_is_visible(_canvas_item);
}

/** Set the visibility of the control point. An invisible point is not drawn on the canvas
 * and cannot receive any events. If you want to have an invisible point that can respond
 * to events, use <tt>invisible_cset</tt> as its color set. */
void ControlPoint::setVisible(bool v)
{
    if (v) sp_canvas_item_show(_canvas_item);
    else sp_canvas_item_hide(_canvas_item);
}

Glib::ustring ControlPoint::format_tip(char const *format, ...)
{
    va_list args;
    va_start(args, format);
    char *dyntip = g_strdup_vprintf(format, args);
    va_end(args);
    Glib::ustring ret = dyntip;
    g_free(dyntip);
    return ret;
}

unsigned int ControlPoint::_size() const
{
    double ret;
    g_object_get(_canvas_item, "size", &ret, NULL);
    return static_cast<unsigned int>(ret);
}

SPCtrlShapeType ControlPoint::_shape() const
{
    SPCtrlShapeType ret;
    g_object_get(_canvas_item, "shape", &ret, NULL);
    return ret;
}

GtkAnchorType ControlPoint::_anchor() const
{
    GtkAnchorType ret;
    g_object_get(_canvas_item, "anchor", &ret, NULL);
    return ret;
}

Glib::RefPtr<Gdk::Pixbuf> ControlPoint::_pixbuf()
{
    GdkPixbuf *ret;
    g_object_get(_canvas_item, "pixbuf", &ret, NULL);
    return Glib::wrap(ret);
}

// Same for setters.

void ControlPoint::_setSize(unsigned int size)
{
    g_object_set(_canvas_item, "size", (gdouble) size, NULL);
}

void ControlPoint::_setShape(SPCtrlShapeType shape)
{
    g_object_set(_canvas_item, "shape", shape, NULL);
}

void ControlPoint::_setAnchor(GtkAnchorType anchor)
{
    g_object_set(_canvas_item, "anchor", anchor, NULL);
}

void ControlPoint::_setPixbuf(Glib::RefPtr<Gdk::Pixbuf> p)
{
    g_object_set(_canvas_item, "pixbuf", Glib::unwrap(p), NULL);
}

// re-routes events into the virtual function
int ControlPoint::_event_handler(SPCanvasItem *item, GdkEvent *event, ControlPoint *point)
{
    return point->_eventHandler(event) ? TRUE : FALSE;
}

// main event callback, which emits all other callbacks.
bool ControlPoint::_eventHandler(GdkEvent *event)
{
    // NOTE the static variables below are shared for all points!

    // offset from the pointer hotspot to the center of the grabbed knot in desktop coords
    static Geom::Point pointer_offset;
    // number of last doubleclicked button, to be
    static unsigned next_release_doubleclick = 0;
    
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int drag_tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);
    
    switch(event->type)
    {
    case GDK_2BUTTON_PRESS:
        // store the button number for next release
        next_release_doubleclick = event->button.button;
        return true;
        
    case GDK_BUTTON_PRESS:
        next_release_doubleclick = 0;
        if (event->button.button == 1) {
            // mouse click. internally, start dragging, but do not emit signals
            // or change position until drag tolerance is exceeded.
            _drag_event_origin[Geom::X] = event->button.x;
            _drag_event_origin[Geom::Y] = event->button.y;
            pointer_offset = _position - _desktop->w2d(_drag_event_origin);
            _drag_initiated = false;
            // route all events to this handler
            sp_canvas_item_grab(_canvas_item, _grab_event_mask, NULL, event->button.time);
            _event_grab = true;
            _setState(STATE_CLICKED);
        }
        return true;
        
    case GDK_MOTION_NOTIFY:
        if (held_button<1>(event->motion) && !_desktop->event_context->space_panning) {
            _desktop->snapindicator->remove_snaptarget(); 
            bool transferred = false;
            if (!_drag_initiated) {
                bool t = fabs(event->motion.x - _drag_event_origin[Geom::X]) <= drag_tolerance &&
                         fabs(event->motion.y - _drag_event_origin[Geom::Y]) <= drag_tolerance;
                if (t) return true;

                // if we are here, it means the tolerance was just exceeded.
                next_release_doubleclick = 0;
                _drag_origin = _position;
                transferred = signal_grabbed.emit(&event->motion);
                // _drag_initiated might change during the above signal emission
                if (!_drag_initiated) {
                    // this guarantees smooth redraws while dragging
                    sp_canvas_force_full_redraw_after_interruptions(_desktop->canvas, 5);
                    _drag_initiated = true;
                }
            }
            if (!transferred) {
                // dragging in progress
                Geom::Point new_pos = _desktop->w2d(event_point(event->motion)) + pointer_offset;
                
                // the new position is passed by reference and can be changed in the handlers.
                signal_dragged.emit(_position, new_pos, &event->motion);
                move(new_pos);
                _updateDragTip(&event->motion); // update dragging tip after moving to new position
                
                _desktop->scroll_to_point(new_pos);
                _desktop->set_coordinate_status(_position);
                sp_event_context_snap_delay_handler(_desktop->event_context, NULL,
                    reinterpret_cast<SPKnot*>(this), &event->motion,
                    DelayedSnapEvent::CONTROL_POINT_HANDLER);
            }
            return true;
        }
        break;
        
    case GDK_BUTTON_RELEASE:
        if (!_event_grab) break;

        // TODO I think snapping on release is wrong, or at least counter-intuitive.
        sp_event_context_snap_watchdog_callback(_desktop->event_context->_delayed_snap_event);
        sp_event_context_discard_delayed_snap_event(_desktop->event_context);
        _desktop->snapindicator->remove_snaptarget();

        sp_canvas_item_ungrab(_canvas_item, event->button.time);
        _setMouseover(this, event->button.state);
        _event_grab = false;

        if (_drag_initiated) {
            sp_canvas_end_forced_full_redraws(_desktop->canvas);
        }

        if (event->button.button == next_release_doubleclick) {
            _drag_initiated = false;
            return signal_doubleclicked.emit(&event->button);
        }
        if (event->button.button == 1) {
            if (_drag_initiated) {
                // it is the end of a drag
                signal_ungrabbed.emit(&event->button);
                _drag_initiated = false;
                return true;
            } else {
                // it is the end of a click
                return signal_clicked.emit(&event->button);
            }
        }
        _drag_initiated = false;
        break;

    case GDK_ENTER_NOTIFY:
        _setMouseover(this, event->crossing.state);
        return true;
    case GDK_LEAVE_NOTIFY:
        _clearMouseover();
        return true;

    case GDK_GRAB_BROKEN:
        if (!event->grab_broken.keyboard && _event_grab) {
            {
                signal_ungrabbed.emit(0);
                if (_drag_initiated)
                    sp_canvas_end_forced_full_redraws(_desktop->canvas);
            }
            _setState(STATE_NORMAL);
            _event_grab = false;
            _drag_initiated = false;
            return true;
        }
        break;

    // update tips on modifier state change
    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE: 
        if (mouseovered_point != this) return false;
        if (_drag_initiated) {
            return true; // this prevents the tool from overwriting the drag tip
        } else {
            unsigned state = state_after_event(event);
            if (state != event->key.state) {
                // we need to return true if there was a tip available, otherwise the tool's
                // handler will process this event and set the tool's message, overwriting
                // the point's message
                return _updateTip(state);
            }
        }
        break;

    default: break;
    }
    
    return false;
}

void ControlPoint::_setMouseover(ControlPoint *p, unsigned state)
{
    bool visible = p->visible();
    if (visible) { // invisible points shouldn't get mouseovered
        p->_setState(STATE_MOUSEOVER);
    }
    p->_updateTip(state);

    if (visible && mouseovered_point != p) {
        mouseovered_point = p;
        signal_mouseover_change.emit(mouseovered_point);
    }
}

bool ControlPoint::_updateTip(unsigned state)
{
    Glib::ustring tip = _getTip(state);
    if (!tip.empty()) {
        _desktop->event_context->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE,
            tip.data());
        return true;
    } else {
        _desktop->event_context->defaultMessageContext()->clear();
        return false;
    }
}

bool ControlPoint::_updateDragTip(GdkEventMotion *event)
{
    if (!_hasDragTips()) return false;
    Glib::ustring tip = _getDragTip(event);
    if (!tip.empty()) {
        _desktop->event_context->defaultMessageContext()->set(Inkscape::NORMAL_MESSAGE,
            tip.data());
        return true;
    } else {
        _desktop->event_context->defaultMessageContext()->clear();
        return false;
    }
}

void ControlPoint::_clearMouseover()
{
    if (mouseovered_point) {
        mouseovered_point->_desktop->event_context->defaultMessageContext()->clear();
        mouseovered_point->_setState(STATE_NORMAL);
        mouseovered_point = 0;
        signal_mouseover_change.emit(mouseovered_point);
    }
}

/** Transfer the grab to another point. This method allows one to create a draggable point
 * that should be dragged instead of the one that received the grabbed signal.
 * This is used to implement dragging out handles in the new node tool, for example.
 *
 * This method will NOT emit the ungrab signal of @c prev_point, because this would complicate
 * using it with selectable control points. If you use this method while dragging, you must emit
 * the ungrab signal yourself.
 *
 * Note that this will break horribly if you try to transfer grab between points in different
 * desktops, which doesn't make much sense anyway. */
void ControlPoint::transferGrab(ControlPoint *prev_point, GdkEventMotion *event)
{
    if (!_event_grab) return;

    signal_grabbed.emit(event);
    sp_canvas_item_ungrab(prev_point->_canvas_item, event->time);
    sp_canvas_item_grab(_canvas_item, _grab_event_mask, NULL, event->time);

    if (!_drag_initiated) {
        sp_canvas_force_full_redraw_after_interruptions(_desktop->canvas, 5);
        _drag_initiated = true;
    }

    prev_point->_setState(STATE_NORMAL);
    _setMouseover(this, event->state);
}

/**
 * @brief Change the state of the knot
 * Alters the appearance of the knot to match one of the states: normal, mouseover
 * or clicked.
 */
void ControlPoint::_setState(State state)
{
    ColorEntry current = {0, 0};
    switch(state) {
    case STATE_NORMAL:
        current = _cset->normal; break;
    case STATE_MOUSEOVER:
        current = _cset->mouseover; break;
    case STATE_CLICKED:
        current = _cset->clicked; break;
    };
    _setColors(current);
    _state = state;
}
void ControlPoint::_setColors(ColorEntry colors)
{
    g_object_set(_canvas_item, "fill_color", colors.fill, "stroke_color", colors.stroke, NULL);
}

} // namespace UI
} // namespace Inkscape

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

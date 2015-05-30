/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2012 Authors
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_CONTROL_POINT_H
#define SEEN_UI_TOOL_CONTROL_POINT_H

#include <gdkmm/pixbuf.h>
#include <boost/utility.hpp>
#include <stddef.h>
#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <2geom/point.h>

#include "ui/control-types.h"
#include "util/accumulators.h"
#include "display/sodipodi-ctrl.h"
#include "enums.h"

class SPDesktop;

namespace Inkscape {
namespace UI {
namespace Tools {

class ToolBase;

}
}
}

namespace Inkscape {
namespace UI {

/**
 * Draggable point, the workhorse of on-canvas editing.
 *
 * Control points (formerly known as knots) are graphical representations of some significant
 * point in the drawing. The drawing can be changed by dragging the point and the things that are
 * attached to it with the mouse. Example things that could be edited with draggable points
 * are gradient stops, the place where text is attached to a path, text kerns, nodes and handles
 * in a path, and many more.
 *
 * @par Control point event handlers
 * @par
 * The control point has several virtual methods which allow you to react to things that
 * happen to it. The most important ones are the grabbed, dragged, ungrabbed and moved functions.
 * When a drag happens, the order of calls is as follows:
 * - <tt>grabbed()</tt>
 * - <tt>dragged()</tt>
 * - <tt>dragged()</tt>
 * - <tt>dragged()</tt>
 * - ...
 * - <tt>dragged()</tt>
 * - <tt>ungrabbed()</tt>
 *
 * The control point can also respond to clicks and double clicks. On a double click,
 * clicked() is called, followed by doubleclicked(). When deriving from SelectableControlPoint,
 * you need to manually call the superclass version at the appropriate point in your handler.
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
class ControlPoint : boost::noncopyable, public sigc::trackable {
public:
    typedef Inkscape::Util::ReverseInterruptible RInt;
    typedef Inkscape::Util::Interruptible Int;

    /**
     * Enumeration representing the possible states of the control point, used to determine
     * its appearance.
     *
     * @todo resolve this to be in sync with the five standard GTK states.
     */
    enum State {
        /** Normal state. */
        STATE_NORMAL,

        /** Mouse is hovering over the control point. */
        STATE_MOUSEOVER,

        /** First mouse button pressed over the control point. */
        STATE_CLICKED
    };

    /**
     * Destructor
     */
    virtual ~ControlPoint();
    
    /// @name Adjust the position of the control point
    /// @{
    /** Current position of the control point. */
    Geom::Point const &position() const { return _position; }

    operator Geom::Point const &() { return _position; }

    /**
     * Move the control point to new position with side effects.
     * This is called after each drag. Override this method if only some positions make sense
     * for a control point (like a point that must always be on a path and can't modify it),
     * or when moving a control point changes the positions of other points.
     */
    virtual void move(Geom::Point const &pos);

    /**
     * Relocate the control point without side effects.
     * Overload this method only if there is an additional graphical representation
     * that must be updated (like the lines that connect handles to nodes). If you override it,
     * you must also call the superclass implementation of the method.
     * @todo Investigate whether this method should be protected
     */
    virtual void setPosition(Geom::Point const &pos);

    /**
     * Apply an arbitrary affine transformation to a control point. This is used
     * by ControlPointSelection, and is important for things like nodes with handles.
     * The default implementation simply moves the point according to the transform.
     */
    virtual void transform(Geom::Affine const &m);
    /// @}
    
    /// @name Toggle the point's visibility
    /// @{
    bool visible() const;

    /**
     * Set the visibility of the control point. An invisible point is not drawn on the canvas
     * and cannot receive any events. If you want to have an invisible point that can respond
     * to events, use <tt>invisible_cset</tt> as its color set.
     */
    virtual void setVisible(bool v);
    /// @}
    
    /// @name Transfer grab from another event handler
    /// @{
    /**
     * Transfer the grab to another point. This method allows one to create a draggable point
     * that should be dragged instead of the one that received the grabbed signal.
     * This is used to implement dragging out handles in the new node tool, for example.
     *
     * This method will NOT emit the ungrab signal of @c prev_point, because this would complicate
     * using it with selectable control points. If you use this method while dragging, you must emit
     * the ungrab signal yourself.
     *
     * Note that this will break horribly if you try to transfer grab between points in different
     * desktops, which doesn't make much sense anyway.
     */
    void transferGrab(ControlPoint *from, GdkEventMotion *event);
    /// @}

    /// @name Receive notifications about control point events
    /// @{
    /*sigc::signal<void, Geom::Point const &, Geom::Point &, GdkEventMotion*> signal_dragged;
    sigc::signal<bool, GdkEventButton*>::accumulated<RInt> signal_clicked;
    sigc::signal<bool, GdkEventButton*>::accumulated<RInt> signal_doubleclicked;
    sigc::signal<bool, GdkEventMotion*>::accumulated<Int> signal_grabbed;
    sigc::signal<void, GdkEventButton*> signal_ungrabbed;*/
    /// @}

    /// @name Inspect the state of the control point
    /// @{
    State state() const { return _state; }

    bool mouseovered() const { return this == mouseovered_point; }
    /// @}

    /** Holds the currently mouseovered control point. */
    static ControlPoint *mouseovered_point;

    /**
     * Emitted when the mouseovered point changes. The parameter is the new mouseovered point.
     * When a point ceases to be mouseovered, the parameter will be NULL.
     */
    static sigc::signal<void, ControlPoint*> signal_mouseover_change;

    static Glib::ustring format_tip(char const *format, ...) G_GNUC_PRINTF(1,2);

    // temporarily public, until snap delay is refactored a little
    virtual bool _eventHandler(Inkscape::UI::Tools::ToolBase *event_context, GdkEvent *event);
    SPDesktop *const _desktop; ///< The desktop this control point resides on.

    bool doubleClicked() {return _double_clicked;}

protected:

    struct ColorEntry {
        guint32 fill;
        guint32 stroke;
    };

    /**
     * Color entries for each possible state.
     * @todo resolve this to be in sync with the five standard GTK states.
     */
    struct ColorSet {
        ColorEntry normal;
        ColorEntry mouseover;
        ColorEntry clicked;
        ColorEntry selected_normal;
        ColorEntry selected_mouseover;
        ColorEntry selected_clicked;
    };

    /**
     * A color set which you can use to create an invisible control that can still receive events.
     */
    static ColorSet invisible_cset;

    /**
     * Create a regular control point.
     * Derive to have constructors with a reasonable number of parameters.
     *
     * @param d Desktop for this control
     * @param initial_pos Initial position of the control point in desktop coordinates
     * @param anchor Where is the control point rendered relative to its desktop coordinates
     * @param type Logical type of the control point.
     * @param cset Colors of the point
     * @param group The canvas group the point's canvas item should be created in
     */
    ControlPoint(SPDesktop *d, Geom::Point const &initial_pos, SPAnchorType anchor,
                 ControlType type,
                 ColorSet const &cset = _default_color_set, SPCanvasGroup *group = 0);

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
    ControlPoint(SPDesktop *d, Geom::Point const &initial_pos, SPAnchorType anchor,
                 Glib::RefPtr<Gdk::Pixbuf> pixbuf,
                 ColorSet const &cset = _default_color_set, SPCanvasGroup *group = 0);

    /// @name Handle control point events in subclasses
    /// @{
    /**
     * Called when the user moves the point beyond the drag tolerance with the first button held
     * down.
     *
     * @param event Motion event when drag tolerance was exceeded.
     * @return true if you called transferGrab() during this method.
     */
    virtual bool grabbed(GdkEventMotion *event);

    /**
     * Called while dragging, but before moving the knot to new position.
     *
     * @param pos Old position, always equal to position()
     * @param new_pos New position (after drag). This is passed as a non-const reference,
     *   so you can change it from the handler - that's how constrained dragging is implemented.
     * @param event Motion event.
     */
    virtual void dragged(Geom::Point &new_pos, GdkEventMotion *event);

    /**
     * Called when the control point finishes a drag.
     *
     * @param event Button release event
     */
    virtual void ungrabbed(GdkEventButton *event);

    /**
     * Called when the control point is clicked, at mouse button release.
     * Improperly implementing this method can cause the default context menu not to appear when a control
     * point is right-clicked.
     *
     * @param event Button release event
     * @return true if the click had some effect, false if it did nothing.
     */
    virtual bool clicked(GdkEventButton *event);

    /**
     * Called when the control point is doubleclicked, at mouse button release.
     *
     * @param event Button release event
     */
    virtual bool doubleclicked(GdkEventButton *event);
    /// @}

    /// @name Manipulate the control point's appearance in subclasses
    /// @{

    /**
     * Change the state of the knot.
     * Alters the appearance of the knot to match one of the states: normal, mouseover
     * or clicked.
     */
    virtual void _setState(State state);

    void _handleControlStyling();

    void _setColors(ColorEntry c);

    unsigned int _size() const;

    SPCtrlShapeType _shape() const;

    SPAnchorType _anchor() const;

    Glib::RefPtr<Gdk::Pixbuf> _pixbuf();

    void _setSize(unsigned int size);

    bool _setControlType(Inkscape::ControlType type);

    void _setAnchor(SPAnchorType anchor);

    void _setPixbuf(Glib::RefPtr<Gdk::Pixbuf>);

    /**
     * Determins if the control point is not visible yet still reacting to events.
     *
     * @return true if non-visible, false otherwise.
     */
    bool _isLurking();

    /**
     * Sets the control point to be non-visible yet still reacting to events.
     *
     * @param lurking true to make non-visible, false otherwise.
     */
    void _setLurking(bool lurking);

    /// @}

    virtual Glib::ustring _getTip(unsigned /*state*/) const { return ""; }

    virtual Glib::ustring _getDragTip(GdkEventMotion */*event*/) const { return ""; }

    virtual bool _hasDragTips() const { return false; }


    SPCanvasItem * _canvas_item; ///< Visual representation of the control point.

    ColorSet const &_cset; ///< Colors used to represent the point

    State _state;

    static Geom::Point const &_last_click_event_point() { return _drag_event_origin; }

    static Geom::Point const &_last_drag_origin() { return _drag_origin; }

    static bool _is_drag_cancelled(GdkEventMotion *event);

    /** Events which should be captured when a handle is being dragged. */
    static int const _grab_event_mask;

    static bool _drag_initiated;

private:

    ControlPoint(ControlPoint const &other);

    void operator=(ControlPoint const &other);

    static int _event_handler(SPCanvasItem *item, GdkEvent *event, ControlPoint *point);

    static void _setMouseover(ControlPoint *, unsigned state);

    static void _clearMouseover();

    bool _updateTip(unsigned state);

    bool _updateDragTip(GdkEventMotion *event);

    void _setDefaultColors();

    void _commonInit();

    Geom::Point _position; ///< Current position in desktop coordinates

    gulong _event_handler_connection;

    bool _lurking;

    static ColorSet _default_color_set;

    /** Stores the window point over which the cursor was during the last mouse button press. */
    static Geom::Point _drag_event_origin;

    /** Stores the desktop point from which the last drag was initiated. */
    static Geom::Point _drag_origin;

    static bool _event_grab;

    bool _double_clicked;
};


} // namespace UI
} // namespace Inkscape

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

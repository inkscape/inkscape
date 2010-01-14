/** @file
 * Desktop-bound visual control object
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_CONTROL_POINT_H
#define SEEN_UI_TOOL_CONTROL_POINT_H

#include <boost/utility.hpp>
#include <sigc++/sigc++.h>
#include <gdkmm.h>
#include <gtkmm.h>
#include <2geom/point.h>

#include "display/display-forward.h"
#include "forward.h"
#include "util/accumulators.h"
#include "display/sodipodi-ctrl.h"

namespace Inkscape {
namespace UI {

// most of the documentation is in the .cpp file

class ControlPoint : boost::noncopyable, public sigc::trackable {
public:
    typedef Inkscape::Util::ReverseInterruptible RInt;
    typedef Inkscape::Util::Interruptible Int;
    // these have to be public, because GCC doesn't allow protected types in constructors,
    // even if the constructors are protected themselves.
    struct ColorEntry {
        guint32 fill;
        guint32 stroke;
    };
    struct ColorSet {
        ColorEntry normal;
        ColorEntry mouseover;
        ColorEntry clicked;
    };
    enum State {
        STATE_NORMAL,
        STATE_MOUSEOVER,
        STATE_CLICKED
    };

    virtual ~ControlPoint();
    
    /// @name Adjust the position of the control point
    /// @{
    /** Current position of the control point. */
    Geom::Point const &position() const { return _position; }
    operator Geom::Point const &() { return _position; }
    virtual void move(Geom::Point const &pos);
    virtual void setPosition(Geom::Point const &pos);
    virtual void transform(Geom::Matrix const &m);
    /// @}
    
    /// @name Toggle the point's visibility
    /// @{
    bool visible() const;
    virtual void setVisible(bool v);
    /// @}
    
    /// @name Transfer grab from another event handler
    /// @{
    void transferGrab(ControlPoint *from, GdkEventMotion *event);
    /// @}

    /// @name Receive notifications about control point events
    /// @{
    sigc::signal<void, Geom::Point const &, Geom::Point &, GdkEventMotion*> signal_dragged;
    sigc::signal<bool, GdkEventButton*>::accumulated<RInt> signal_clicked;
    sigc::signal<bool, GdkEventButton*>::accumulated<RInt> signal_doubleclicked;
    sigc::signal<bool, GdkEventMotion*>::accumulated<Int> signal_grabbed;
    sigc::signal<void, GdkEventButton*> signal_ungrabbed;
    /// @}

    /// @name Inspect the state of the control point
    /// @{
    State state() { return _state; }
    bool mouseovered() { return this == mouseovered_point; }
    /// @}

    static ControlPoint *mouseovered_point;
    static sigc::signal<void, ControlPoint*> signal_mouseover_change;
    static Glib::ustring format_tip(char const *format, ...) G_GNUC_PRINTF(1,2);

    // temporarily public, until snapping is refactored a little
    virtual bool _eventHandler(GdkEvent *event);

protected:
    ControlPoint(SPDesktop *d, Geom::Point const &initial_pos, Gtk::AnchorType anchor,
        SPCtrlShapeType shape, unsigned int size, ColorSet *cset = 0, SPCanvasGroup *group = 0);
    ControlPoint(SPDesktop *d, Geom::Point const &initial_pos, Gtk::AnchorType anchor,
        Glib::RefPtr<Gdk::Pixbuf> pixbuf, ColorSet *cset = 0, SPCanvasGroup *group = 0);

    /// @name Manipulate the control point's appearance in subclasses
    /// @{
    virtual void _setState(State state);
    void _setColors(ColorEntry c);

    unsigned int _size() const;
    SPCtrlShapeType _shape() const;
    GtkAnchorType _anchor() const;
    Glib::RefPtr<Gdk::Pixbuf> _pixbuf();

    void _setSize(unsigned int size);
    void _setShape(SPCtrlShapeType shape);
    void _setAnchor(GtkAnchorType anchor);
    void _setPixbuf(Glib::RefPtr<Gdk::Pixbuf>);
    /// @}

    virtual Glib::ustring _getTip(unsigned state) { return ""; }
    virtual Glib::ustring _getDragTip(GdkEventMotion *event) { return ""; }
    virtual bool _hasDragTips() { return false; }

    SPDesktop *const _desktop; ///< The desktop this control point resides on.
    SPCanvasItem * _canvas_item; ///< Visual representation of the control point.
    ColorSet *_cset; ///< Colors used to represent the point
    State _state;

    static int const _grab_event_mask;
    static Geom::Point const &_last_click_event_point() { return _drag_event_origin; }
    static Geom::Point const &_last_drag_origin() { return _drag_origin; }

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

    static Geom::Point _drag_event_origin;
    static Geom::Point _drag_origin;
    static bool _event_grab;
    static bool _drag_initiated;
};

extern ControlPoint::ColorSet invisible_cset;


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

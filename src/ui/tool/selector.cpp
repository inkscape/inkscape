/** @file
 * Selector component (click and rubberband)
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "desktop.h"
#include "desktop-handles.h"
#include "display/sodipodi-ctrlrect.h"
#include "event-context.h"
#include "preferences.h"
#include "ui/tool/control-point.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/selector.h"

namespace Inkscape {
namespace UI {

/** A hidden control point used for rubberbanding and selection.
 * It uses a clever hack: the canvas item is hidden and only receives events when they
 * are passed to it using Selector's event() function. When left mouse button
 * is pressed, it grabs events and handles drags and clicks in the usual way. */
class SelectorPoint : public ControlPoint {
public:
    SelectorPoint(SPDesktop *d, SPCanvasGroup *group, Selector *s)
        : ControlPoint(d, Geom::Point(0,0), Gtk::ANCHOR_CENTER, SP_CTRL_SHAPE_SQUARE,
            1, &invisible_cset, group)
        , _selector(s)
        , _cancel(false)
    {
        setVisible(false);
        _rubber = static_cast<CtrlRect*>(sp_canvas_item_new(sp_desktop_controls(_desktop),
        SP_TYPE_CTRLRECT, NULL));
        sp_canvas_item_hide(_rubber);
    }
    ~SelectorPoint() {
        gtk_object_destroy(_rubber);
    }
    SPDesktop *desktop() { return _desktop; }
    bool event(GdkEvent *e) {
        return _eventHandler(e);
    }

protected:
    virtual bool _eventHandler(GdkEvent *event) {
        if (event->type == GDK_KEY_PRESS && shortcut_key(event->key) == GDK_Escape &&
            sp_canvas_item_is_visible(_rubber))
        {
            _cancel = true;
            sp_canvas_item_hide(_rubber);
            return true;
        }
        return ControlPoint::_eventHandler(event);
    }

private:
    virtual bool grabbed(GdkEventMotion *) {
        _cancel = false;
        _start = position();
        sp_canvas_item_show(_rubber);
        return false;
    }
    virtual void dragged(Geom::Point &new_pos, GdkEventMotion *) {
        if (_cancel) return;
        Geom::Rect sel(_start, new_pos);
        _rubber->setRectangle(sel);
    }
    virtual void ungrabbed(GdkEventButton *event) {
        if (_cancel) return;
        sp_canvas_item_hide(_rubber);
        Geom::Rect sel(_start, position());
        _selector->signal_area.emit(sel, event);
    }
    virtual bool clicked(GdkEventButton *event) {
        if (event->button != 1) return false;
        _selector->signal_point.emit(position(), event);
        return true;
    }
    CtrlRect *_rubber;
    Selector *_selector;
    Geom::Point _start;
    bool _cancel;
};


Selector::Selector(SPDesktop *d)
    : Manipulator(d)
    , _dragger(new SelectorPoint(d, sp_desktop_controls(d), this))
{
    _dragger->setVisible(false);
}

Selector::~Selector()
{
    delete _dragger;
}

bool Selector::event(GdkEvent *event)
{
    // The hidden control point will capture all events after it obtains the grab,
    // but it relies on this function to initiate it. Here we can filter what events
    // it will receive.
    switch (event->type) {
    case GDK_BUTTON_PRESS:
        // Do not pass button presses other than left button to the control point.
        // This way middle click and right click can be handled in SPEventContext.
        if (event->button.button != 1) return false;
        _dragger->setPosition(_desktop->w2d(event_point(event->motion)));
        break;
    default: break;
    }
    return _dragger->event(event);
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

/** @file
 * Selector component (click and rubberband)
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "control-point.h"
#include "desktop.h"

#include "display/sodipodi-ctrlrect.h"
#include "ui/tools/tool-base.h"
#include "preferences.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/selector.h"

#include <gdk/gdkkeysyms.h>

namespace Inkscape {
namespace UI {

/** A hidden control point used for rubberbanding and selection.
 * It uses a clever hack: the canvas item is hidden and only receives events when they
 * are passed to it using Selector's event() function. When left mouse button
 * is pressed, it grabs events and handles drags and clicks in the usual way. */
class SelectorPoint : public ControlPoint {
public:
    SelectorPoint(SPDesktop *d, SPCanvasGroup *group, Selector *s) :
        ControlPoint(d, Geom::Point(0,0), SP_ANCHOR_CENTER,
                     CTRL_TYPE_INVISIPOINT,
                     invisible_cset, group),
        _selector(s),
        _cancel(false)
    {
        setVisible(false);
        _rubber = static_cast<CtrlRect*>(sp_canvas_item_new(_desktop->getControls(),
        SP_TYPE_CTRLRECT, NULL));
        _rubber->setShadow(1, 0xffffffff);
        sp_canvas_item_hide(_rubber);
    }

    ~SelectorPoint() {
        sp_canvas_item_destroy(_rubber);
    }

    SPDesktop *desktop() { return _desktop; }

    bool event(Inkscape::UI::Tools::ToolBase *ec, GdkEvent *e) {
        return _eventHandler(ec, e);
    }

protected:
    virtual bool _eventHandler(Inkscape::UI::Tools::ToolBase *event_context, GdkEvent *event) {
        if (event->type == GDK_KEY_PRESS && shortcut_key(event->key) == GDK_KEY_Escape &&
            sp_canvas_item_is_visible(_rubber))
        {
            _cancel = true;
            sp_canvas_item_hide(_rubber);
            return true;
        }
        return ControlPoint::_eventHandler(event_context, event);
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
    , _dragger(new SelectorPoint(d, d->getControls(), this))
{
    _dragger->setVisible(false);
}

Selector::~Selector()
{
    delete _dragger;
}

bool Selector::event(Inkscape::UI::Tools::ToolBase *event_context, GdkEvent *event)
{
    // The hidden control point will capture all events after it obtains the grab,
    // but it relies on this function to initiate it. If we pass only first button
    // press events here, it won't interfere with any other event handling.
    switch (event->type) {
    case GDK_BUTTON_PRESS:
        // Do not pass button presses other than left button to the control point.
        // This way middle click and right click can be handled in ToolBase.
        if (event->button.button == 1 && !event_context->space_panning) {
            _dragger->setPosition(_desktop->w2d(event_point(event->motion)));
            return _dragger->event(event_context, event);
        }
        break;
    default: break;
    }
    return false;
}

bool Selector::doubleClicked() {
    return _dragger->doubleClicked();
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

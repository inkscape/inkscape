/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/tool/selectable-control-point.h"
#include "ui/tool/control-point-selection.h"
#include "ui/tool/event-utils.h"

namespace Inkscape {
namespace UI {

ControlPoint::ColorSet SelectableControlPoint::_default_scp_color_set = {
    {0xffffff00, 0x01000000}, // normal fill, stroke
    {0xff0000ff, 0x01000000}, // mouseover fill, stroke
    {0x0000ffff, 0x01000000}, // clicked fill, stroke
    //
    {0x0000ffff, 0x000000ff}, // normal fill, stroke when selected
    {0xff000000, 0x000000ff}, // mouseover fill, stroke when selected
    {0xff000000, 0x000000ff}  // clicked fill, stroke when selected
};

SelectableControlPoint::SelectableControlPoint(SPDesktop *d, Geom::Point const &initial_pos, SPAnchorType anchor,
                                               Inkscape::ControlType type,
                                               ControlPointSelection &sel,
                                               ColorSet const &cset, SPCanvasGroup *group) :
    ControlPoint(d, initial_pos, anchor, type, cset, group),
    _selection(sel)
{
    _selection.allPoints().insert(this);
}

SelectableControlPoint::SelectableControlPoint(SPDesktop *d, Geom::Point const &initial_pos, SPAnchorType anchor,
                                               Glib::RefPtr<Gdk::Pixbuf> pixbuf,
                                               ControlPointSelection &sel,
                                               ColorSet const &cset, SPCanvasGroup *group) :
    ControlPoint(d, initial_pos, anchor, pixbuf, cset, group),
    _selection (sel)
{
    _selection.allPoints().insert(this);
}

SelectableControlPoint::~SelectableControlPoint()
{
    _selection.erase(this);
    _selection.allPoints().erase(this);
}

bool SelectableControlPoint::grabbed(GdkEventMotion *)
{
    // if a point is dragged while not selected, it should select itself
    if (!selected()) {
        _takeSelection();
    }
    _selection._pointGrabbed(this);
    return false;
}

void SelectableControlPoint::dragged(Geom::Point &new_pos, GdkEventMotion *event)
{
    _selection._pointDragged(new_pos, event);
}

void SelectableControlPoint::ungrabbed(GdkEventButton *)
{
    _selection._pointUngrabbed();
}

bool SelectableControlPoint::clicked(GdkEventButton *event)
{
    if (_selection._pointClicked(this, event))
        return true;

    if (event->button != 1) return false;
    if (held_shift(*event)) {
        if (selected()) {
            _selection.erase(this);
        } else {
            _selection.insert(this);
        }
    } else {
        _takeSelection();
    }
    return true;
}

void SelectableControlPoint::_takeSelection()
{
    _selection.clear();
    _selection.insert(this);
}

bool SelectableControlPoint::selected() const
{
    SelectableControlPoint *p = const_cast<SelectableControlPoint*>(this);
    return _selection.find(p) != _selection.end();
}

void SelectableControlPoint::_setState(State state)
{
    if (!selected()) {
        ControlPoint::_setState(state);
    } else {
        ColorEntry current = {0, 0};
        ColorSet const &activeCset = (_isLurking()) ? invisible_cset : _cset;
        switch (state) {
            case STATE_NORMAL:
                current = activeCset.selected_normal;
                break;
            case STATE_MOUSEOVER:
                current = activeCset.selected_mouseover;
                break;
            case STATE_CLICKED:
                current = activeCset.selected_clicked;
                break;
        }
        _setColors(current);
        _state = state;
    }
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

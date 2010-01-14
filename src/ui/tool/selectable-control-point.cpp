/** @file
 * Desktop-bound selectable control object - implementation
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/tool/control-point-selection.h"
#include "ui/tool/event-utils.h"
#include "ui/tool/selectable-control-point.h"

namespace Inkscape {
namespace UI {

static SelectableControlPoint::ColorSet default_scp_color_set = {
    {
        {0xffffff00, 0x01000000}, // normal fill, stroke
        {0xff0000ff, 0x01000000}, // mouseover fill, stroke
        {0x0000ffff, 0x01000000}  // clicked fill, stroke
    },
    {0x0000ffff, 0x000000ff}, // normal fill, stroke when selected
    {0xff000000, 0x000000ff}, // mouseover fill, stroke when selected
    {0xff000000, 0x000000ff}  // clicked fill, stroke when selected
};

SelectableControlPoint::SelectableControlPoint(SPDesktop *d, Geom::Point const &initial_pos,
        Gtk::AnchorType anchor, SPCtrlShapeType shape, unsigned int size,
        ControlPointSelection &sel, ColorSet *cset, SPCanvasGroup *group)
    : ControlPoint (d, initial_pos, anchor, shape, size,
        cset ? reinterpret_cast<ControlPoint::ColorSet*>(cset)
        : reinterpret_cast<ControlPoint::ColorSet*>(&default_scp_color_set), group)
    , _selection (sel)
{
    _connectHandlers();
}
SelectableControlPoint::SelectableControlPoint(SPDesktop *d, Geom::Point const &initial_pos,
        Gtk::AnchorType anchor, Glib::RefPtr<Gdk::Pixbuf> pixbuf,
        ControlPointSelection &sel, ColorSet *cset, SPCanvasGroup *group)
    : ControlPoint (d, initial_pos, anchor, pixbuf,
        cset ? reinterpret_cast<ControlPoint::ColorSet*>(cset)
        : reinterpret_cast<ControlPoint::ColorSet*>(&default_scp_color_set), group)
    , _selection (sel)
{
    _connectHandlers();
}

SelectableControlPoint::~SelectableControlPoint()
{
    _selection.erase(this);
    _selection.allPoints().erase(this);
}

void SelectableControlPoint::_connectHandlers()
{
    _selection.allPoints().insert(this);
    signal_grabbed.connect(
        sigc::bind_return(
            sigc::hide(
                sigc::mem_fun(*this, &SelectableControlPoint::_grabbedHandler)),
            false));
    signal_dragged.connect(
        sigc::mem_fun(*this, &SelectableControlPoint::_draggedHandler));
    signal_ungrabbed.connect(
        sigc::hide(
            sigc::mem_fun(*this, &SelectableControlPoint::_ungrabbedHandler)));
    signal_clicked.connect(
        sigc::mem_fun(*this, &SelectableControlPoint::_clickedHandler));
}

void SelectableControlPoint::_grabbedHandler()
{
    // if a point is dragged while not selected, it should select itself
    if (!selected()) {
        _takeSelection();
        _selection._pointGrabbed();
    }
}
void SelectableControlPoint::_draggedHandler(Geom::Point const &old_pos, Geom::Point &new_pos, GdkEventMotion *event)
{
    _selection._pointDragged(old_pos, new_pos, event);
}
void SelectableControlPoint::_ungrabbedHandler()
{
    _selection._pointUngrabbed();
}
bool SelectableControlPoint::_clickedHandler(GdkEventButton *event)
{
    if (selected() && _selection._pointClicked(this, event)) return true;
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
        return;
    }

    ColorSet *cset = reinterpret_cast<ColorSet*>(_cset);
    ColorEntry current = {0, 0};
    switch (state) {
    case STATE_NORMAL:
        current = cset->selected_normal; break;
    case STATE_MOUSEOVER:
        current = cset->selected_mouseover; break;
    case STATE_CLICKED:
        current = cset->selected_clicked; break;
    }
    _setColors(current);
    _state = state;
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

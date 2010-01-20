/** @file
 * Desktop-bound selectable control object
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_SELECTABLE_CONTROL_POINT_H
#define SEEN_UI_TOOL_SELECTABLE_CONTROL_POINT_H

#include <boost/enable_shared_from_this.hpp>
#include "ui/tool/control-point.h"

namespace Inkscape {
namespace UI {

class ControlPointSelection;

class SelectableControlPoint : public ControlPoint {
public:
    struct ColorSet {
        ControlPoint::ColorSet cpset;
        ColorEntry selected_normal;
        ColorEntry selected_mouseover;
        ColorEntry selected_clicked;
    };

    ~SelectableControlPoint();
    bool selected() const;
    void updateState() const { const_cast<SelectableControlPoint*>(this)->_setState(_state); }
    virtual Geom::Rect bounds() {
        return Geom::Rect(position(), position());
    }
protected:
    SelectableControlPoint(SPDesktop *d, Geom::Point const &initial_pos,
        Gtk::AnchorType anchor, SPCtrlShapeType shape,
        unsigned int size, ControlPointSelection &sel, ColorSet *cset = 0,
        SPCanvasGroup *group = 0);
    SelectableControlPoint(SPDesktop *d, Geom::Point const &initial_pos,
        Gtk::AnchorType anchor, Glib::RefPtr<Gdk::Pixbuf> pixbuf,
        ControlPointSelection &sel, ColorSet *cset = 0, SPCanvasGroup *group = 0);

    virtual void _setState(State state);

    ControlPointSelection &_selection;
private:
    void _connectHandlers();
    void _takeSelection();
    
    void _grabbedHandler();
    bool _clickedHandler(GdkEventButton *);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

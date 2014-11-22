/* Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2009 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_UI_TOOL_SELECTABLE_CONTROL_POINT_H
#define SEEN_UI_TOOL_SELECTABLE_CONTROL_POINT_H

#include "ui/tool/control-point.h"

namespace Inkscape {
namespace UI {

class ControlPointSelection;

/**
 * Desktop-bound selectable control object.
 */
class SelectableControlPoint : public ControlPoint {
public:

    ~SelectableControlPoint();
    bool selected() const;
    void updateState() { _setState(_state); }
    virtual Geom::Rect bounds() const {
        return Geom::Rect(position(), position());
    }
    friend class NodeList;

protected:

    SelectableControlPoint(SPDesktop *d, Geom::Point const &initial_pos, SPAnchorType anchor,
                           Inkscape::ControlType type,
                           ControlPointSelection &sel,
                           ColorSet const &cset = _default_scp_color_set, SPCanvasGroup *group = 0);

    SelectableControlPoint(SPDesktop *d, Geom::Point const &initial_pos, SPAnchorType anchor,
                           Glib::RefPtr<Gdk::Pixbuf> pixbuf,
                           ControlPointSelection &sel,
                           ColorSet const &cset = _default_scp_color_set, SPCanvasGroup *group = 0);

    virtual void _setState(State state);

    virtual void dragged(Geom::Point &new_pos, GdkEventMotion *event);
    virtual bool grabbed(GdkEventMotion *event);
    virtual void ungrabbed(GdkEventButton *event);
    virtual bool clicked(GdkEventButton *event);

    ControlPointSelection &_selection;

private:

    void _takeSelection();

    static ColorSet _default_scp_color_set;
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

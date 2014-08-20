#ifndef INKSCAPE_DISPLAY_SNAP_INDICATOR_H
#define INKSCAPE_DISPLAY_SNAP_INDICATOR_H

/**
 * @file
 * Provides a class that shows a temporary indicator on the canvas of where the snap was, and what kind of snap
 */
/*
 * Authors:
 *   Johan Engelen
 *   Diederik van Lierop
 *
 * Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Diederik van Lierop 2010 <mail@diedenrezi.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "snapped-point.h"

class SPDesktop;

namespace Inkscape {
namespace Display {

class TemporaryItem;

class SnapIndicator  {
public:
    SnapIndicator(SPDesktop *desktop);
    virtual ~SnapIndicator();

    void set_new_snaptarget(Inkscape::SnappedPoint const &p, bool pre_snap = false);
    void remove_snaptarget(bool only_if_presnap = false);

    void set_new_snapsource(Inkscape::SnapCandidatePoint const &p);
    void remove_snapsource();

    void set_new_debugging_point(Geom::Point const &p);
    void remove_debugging_points();

protected:
    TemporaryItem *_snaptarget;
    TemporaryItem *_snaptarget_tooltip;
    TemporaryItem *_snaptarget_bbox;
    TemporaryItem *_snapsource;
    std::list<TemporaryItem *> _debugging_points;
    bool _snaptarget_is_presnap;
    SPDesktop *_desktop;

private:
    SnapIndicator(const SnapIndicator&);
    SnapIndicator& operator=(const SnapIndicator&);
};

} //namespace Display
} //namespace Inkscape

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

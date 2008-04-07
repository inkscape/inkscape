#ifndef INKSCAPE_DISPLAY_SNAP_INDICATOR_H
#define INKSCAPE_DISPLAY_SNAP_INDICATOR_H

/** \file
 * Provides a class that shows a temporary indicator on the canvas of where the snap was, and what kind of snap
 *
 * Authors:
 *   Johan Engelen
 *
 * Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "forward.h"
#include "display/display-forward.h"
#include "snapped-point.h"

namespace Inkscape {
namespace Display {

class SnapIndicator  {
public:
    SnapIndicator(SPDesktop *desktop);
    virtual ~SnapIndicator();

    void set_new_snappoint(Inkscape::SnappedPoint const p);
    void remove_snappoint();

protected:
    TemporaryItem * tempitem;
    SPDesktop *desktop;

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

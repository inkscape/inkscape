#ifndef SEEN_SNAPPEDPOINT_H
#define SEEN_SNAPPEDPOINT_H

/**
 *    \file src/snapped-point.h
 *    \brief SnappedPoint class.
 *
 *    Authors:
 *      Mathieu Dimanche <mdimanche@free.fr>
 *
 *    Released under GNU GPL, read the file 'COPYING' for more information.
 */

#include <vector>
#include "libnr/nr-coord.h"
#include "libnr/nr-point.h"

namespace Inkscape
{

class HighlightGroup;

/// Class describing the result of an attempt to snap.
class SnappedPoint
{
public:
    SnappedPoint() {}
    SnappedPoint(::NR::Point p, ::NR::Coord d);
    ~SnappedPoint();

    void addHighlightGroup(HighlightGroup *group);
    void addHighlightGroups(std::vector<HighlightGroup*> *groups);

    ::NR::Coord getDistance() const;
    NR::Point getPoint() const;
    std::vector<HighlightGroup*> getHighlightGroups() const;

private:
    ::NR::Coord _distance;
    ::NR::Point _point;
    std::vector<HighlightGroup*> _hightlight_groups;
};

}
#endif /* !SEEN_SNAPPEDPOINT_H */

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

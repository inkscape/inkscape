#ifndef INKSCAPE_LPE_LINE_SEGMENT_H
#define INKSCAPE_LPE_LINE_SEGMENT_H

/** \file
 * LPE <line_segment> implementation
 */

/*
 * Authors:
 *   Maximilian Albert
 *
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/enum.h"
#include "live_effects/effect.h"

namespace Inkscape {
namespace LivePathEffect {

enum EndType {
    END_CLOSED,
    END_OPEN_INITIAL,
    END_OPEN_FINAL,
    END_OPEN_BOTH
};

class LPELineSegment : public Effect {
public:
    LPELineSegment(LivePathEffectObject *lpeobject);
    virtual ~LPELineSegment();

    virtual void doBeforeEffect (SPLPEItem const* lpeitem);

    virtual Geom::PathVector doEffect_path (Geom::PathVector const & path_in);

//private:
    EnumParam<EndType> end_type;

private:
    Geom::Point A, B; // intersections of the line segment with the limiting bounding box
    Geom::Point bboxA, bboxB; // upper left and lower right corner of limiting bounding box

    LPELineSegment(const LPELineSegment&);
    LPELineSegment& operator=(const LPELineSegment&);
};

} //namespace LivePathEffect
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

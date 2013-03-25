#ifndef INKSCAPE_LPE_RULER_H
#define INKSCAPE_LPE_RULER_H

/** \file
 * LPE <ruler> implementation, see lpe-ruler.cpp.
 */

/*
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/enum.h"
#include "live_effects/effect.h"
#include "live_effects/parameter/unit.h"

namespace Inkscape {
namespace LivePathEffect {

enum MarkType {
    MARK_MAJOR,
    MARK_MINOR
};

enum MarkDirType {
    MARKDIR_LEFT,
    MARKDIR_RIGHT,
    MARKDIR_BOTH,
};

enum BorderMarkType {
    BORDERMARK_NONE,
    BORDERMARK_START,
    BORDERMARK_END,
    BORDERMARK_BOTH,
};

class LPERuler : public Effect {
public:
    LPERuler(LivePathEffectObject *lpeobject);
    virtual ~LPERuler();

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

private:
    Geom::Piecewise<Geom::D2<Geom::SBasis> > ruler_mark(Geom::Point const &A, Geom::Point const &n, MarkType const &marktype);

    ScalarParam mark_distance;
    UnitParam unit;
    ScalarParam mark_length;
    ScalarParam minor_mark_length;
    ScalarParam major_mark_steps;
    ScalarParam shift;
    EnumParam<MarkDirType> mark_dir;
    ScalarParam offset;
    EnumParam<BorderMarkType> border_marks;

    static Geom::Point n_major, n_minor; // used for internal computations

    LPERuler(const LPERuler&);
    LPERuler& operator=(const LPERuler&);
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

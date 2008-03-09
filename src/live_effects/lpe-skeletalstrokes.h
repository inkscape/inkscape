#ifndef INKSCAPE_LPE_SKELETAL_STROKES_H
#define INKSCAPE_LPE_SKELETAL_STROKES_H

/*
 * Inkscape::LPESkeletalStrokes
 *
* Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/path.h"
#include "live_effects/parameter/enum.h"
#include "live_effects/parameter/bool.h"

namespace Inkscape {
namespace LivePathEffect {

enum SkelCopyType {
    SSCT_SINGLE = 0,
    SSCT_SINGLE_STRETCHED,
    SSCT_REPEATED,
    SSCT_REPEATED_STRETCHED,
    SSCT_END // This must be last
};

class LPESkeletalStrokes : public Effect {
public:
    LPESkeletalStrokes(LivePathEffectObject *lpeobject);
    virtual ~LPESkeletalStrokes();

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > & pwd2_in);

    virtual void transform_multiply(Geom::Matrix const& postmul, bool set);

private:
    PathParam  pattern;
    EnumParam<SkelCopyType> copytype;
    ScalarParam  prop_scale;
    BoolParam scale_y_rel;
    ScalarParam  spacing;
    ScalarParam  normal_offset;
    ScalarParam  tang_offset;
    BoolParam    prop_units;
    BoolParam    vertical_pattern;

    void on_pattern_pasted();

    LPESkeletalStrokes(const LPESkeletalStrokes&);
    LPESkeletalStrokes& operator=(const LPESkeletalStrokes&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif

#ifndef INKSCAPE_LPE_PATTERN_ALONG_PATH_H
#define INKSCAPE_LPE_PATTERN_ALONG_PATH_H

/*
 * Inkscape::LPEPatternAlongPath
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

enum PAPCopyType {
    PAPCT_SINGLE = 0,
    PAPCT_SINGLE_STRETCHED,
    PAPCT_REPEATED,
    PAPCT_REPEATED_STRETCHED,
    PAPCT_END // This must be last
};

class LPEPatternAlongPath : public Effect {
public:
    LPEPatternAlongPath(LivePathEffectObject *lpeobject);
    virtual ~LPEPatternAlongPath();

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    PathParam  pattern;
private:
    EnumParam<PAPCopyType> copytype;
    ScalarParam  prop_scale;
    BoolParam scale_y_rel;
    ScalarParam  spacing;
    ScalarParam  normal_offset;
    ScalarParam  tang_offset;
    BoolParam    prop_units;
    BoolParam    vertical_pattern;

    void on_pattern_pasted();

    LPEPatternAlongPath(const LPEPatternAlongPath&);
    LPEPatternAlongPath& operator=(const LPEPatternAlongPath&);
};

class LPEFreehandShape : public LPEPatternAlongPath {
public:
  LPEFreehandShape(LivePathEffectObject *lpeobject);
  virtual ~LPEFreehandShape() {}
};

}; //namespace LivePathEffect
}; //namespace Inkscape

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

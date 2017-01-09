#ifndef INKSCAPE_LPE_TRANSFORM_2PTS_H
#define INKSCAPE_LPE_TRANSFORM_2PTS_H

/** \file
 * LPE "Transform through 2 points" implementation
 */

/*
 * Authors:
 *
 *
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/lpegroupbbox.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/togglebutton.h"
#include "live_effects/parameter/point.h"

namespace Inkscape {
namespace LivePathEffect {

class LPETransform2Pts : public Effect, GroupBBoxEffect {
public:
    LPETransform2Pts(LivePathEffectObject *lpeobject);
    virtual ~LPETransform2Pts();

    virtual void doOnApply (SPLPEItem const* lpeitem);

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    virtual void doBeforeEffect (SPLPEItem const* lpeitem);

    virtual Gtk::Widget *newWidget();

    void updateIndex();

    size_t nodeCount(Geom::PathVector pathvector) const;

    Geom::Point pointAtNodeIndex(Geom::PathVector pathvector, size_t index) const;

    Geom::Path pathAtNodeIndex(Geom::PathVector pathvector, size_t index) const;

    void reset();

protected:
    virtual void addCanvasIndicators(SPLPEItem const *lpeitem, std::vector<Geom::PathVector> &hp_vec);

private:
    ToggleButtonParam elastic;
    ToggleButtonParam from_original_width;
    ToggleButtonParam lock_lenght;
    ToggleButtonParam lock_angle;
    ToggleButtonParam flip_horizontal;
    ToggleButtonParam flip_vertical;
    PointParam start;
    PointParam end;
    ScalarParam stretch;
    ScalarParam offset;
    ScalarParam first_knot;
    ScalarParam last_knot;
    ScalarParam helper_size;
    bool from_original_width_toggler;
    Geom::Point point_a;
    Geom::Point point_b;
    Geom::PathVector pathvector;
    bool append_path;
    Geom::Angle previous_angle;
    Geom::Point previous_start;
    double previous_lenght;
    LPETransform2Pts(const LPETransform2Pts&);
    LPETransform2Pts& operator=(const LPETransform2Pts&);
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

#ifndef INKSCAPE_LPE_SIMPLIFY_H
#define INKSCAPE_LPE_SIMPLIFY_H

/*
 * Inkscape::LPESimplify
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/togglebutton.h"
#include "live_effects/lpegroupbbox.h"

namespace Inkscape {
namespace LivePathEffect {

class LPESimplify : public Effect , GroupBBoxEffect {

public:
    LPESimplify(LivePathEffectObject *lpeobject);
    virtual ~LPESimplify();

    virtual void doEffect(SPCurve *curve);

    virtual void doBeforeEffect (SPLPEItem const* lpeitem);

    virtual void generateHelperPathAndSmooth(Geom::PathVector &result);

    virtual Gtk::Widget * newWidget();

    virtual void drawNode(Geom::Point p);

    virtual void drawHandle(Geom::Point p);

    virtual void drawHandleLine(Geom::Point p,Geom::Point p2);

protected:
    void addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec);

private:
    ScalarParam steps;
    ScalarParam threshold;
    ScalarParam smooth_angles;
    ScalarParam helper_size;
    ToggleButtonParam simplify_individual_paths;
    ToggleButtonParam simplify_just_coalesce;

    double radius_helper_nodes;
    Geom::PathVector hp;
    Geom::OptRect bbox;

    LPESimplify(const LPESimplify &);
    LPESimplify &operator=(const LPESimplify &);

};

}; //namespace LivePathEffect
}; //namespace Inkscape
#endif

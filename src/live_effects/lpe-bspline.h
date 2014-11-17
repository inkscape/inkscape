#ifndef INKSCAPE_LPE_BSPLINE_H
#define INKSCAPE_LPE_BSPLINE_H

/*
 * Inkscape::LPEBSpline
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/bool.h"
#include <vector>

namespace Inkscape {
namespace LivePathEffect {

class LPEBSpline : public Effect {
public:
    LPEBSpline(LivePathEffectObject *lpeobject);
    virtual ~LPEBSpline();

    virtual LPEPathFlashType pathFlashType() const {
        return SUPPRESS_FLASH;
    }
    virtual void doOnApply(SPLPEItem const* lpeitem);
    virtual void doEffect(SPCurve *curve);
    virtual void doBeforeEffect (SPLPEItem const* lpeitem);
    void drawHandle(Geom::Point p, double radiusHelperNodes);
    virtual void doBSplineFromWidget(SPCurve *curve, double value);
    void addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec);
    virtual Gtk::Widget *newWidget();
    virtual void changeWeight(double weightValue);
    virtual void toDefaultWeight();
    virtual void toMakeCusp();
    virtual void toWeight();

    // TODO make this private
    ScalarParam steps;

private:
    BoolParam ignoreCusp;
    BoolParam onlySelected;
    BoolParam showHelper;
    ScalarParam weight;
    Geom::PathVector hp;

    LPEBSpline(const LPEBSpline &);
    LPEBSpline &operator=(const LPEBSpline &);

};

} //namespace LivePathEffect
} //namespace Inkscape
#endif

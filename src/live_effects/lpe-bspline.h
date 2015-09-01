#ifndef INKSCAPE_LPE_BSPLINE_H
#define INKSCAPE_LPE_BSPLINE_H

/*
 * Inkscape::LPEBSpline
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include "live_effects/effect.h"

#include <vector>

namespace Inkscape {
namespace LivePathEffect {

class LPEBSpline : public Effect {
public:
    LPEBSpline(LivePathEffectObject *lpeobject);
    virtual ~LPEBSpline();

    virtual LPEPathFlashType pathFlashType() const
    {
        return SUPPRESS_FLASH;
    }
    virtual void doOnApply(SPLPEItem const* lpeitem);
    virtual void doEffect(SPCurve *curve);
    virtual void doBeforeEffect (SPLPEItem const* lpeitem);
    void doBSplineFromWidget(SPCurve *curve, double value);
    void addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec);
    virtual Gtk::Widget *newWidget();
    void changeWeight(double weightValue);
    void toDefaultWeight();
    void toMakeCusp();
    void toWeight();

    // TODO make this private
    ScalarParam steps;

private:
    ScalarParam helper_size;
    BoolParam apply_no_weight;
    BoolParam apply_with_weight;
    BoolParam only_selected;
    ScalarParam weight;

    LPEBSpline(const LPEBSpline &);
    LPEBSpline &operator=(const LPEBSpline &);

};
void sp_bspline_do_effect(SPCurve *curve, double helper_size);

} //namespace LivePathEffect
} //namespace Inkscape
#endif

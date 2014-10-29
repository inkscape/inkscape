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

    virtual void createAndApply(const char *name, SPDocument *doc, SPItem *item);
    virtual LPEPathFlashType pathFlashType() const {
        return SUPPRESS_FLASH;
    }
    virtual void doEffect(SPCurve *curve);
    virtual void doBeforeEffect (SPLPEItem const* lpeitem);
    void drawHandle(Geom::Point p, double radiusHelperNodes);
    void addCanvasIndicators(SPLPEItem const */*lpeitem*/, std::vector<Geom::PathVector> &hp_vec);
    virtual void doBSplineFromWidget(SPCurve *curve, double value);
    virtual bool nodeIsSelected(Geom::Point nodePoint);
    virtual Gtk::Widget *newWidget();
    virtual void changeWeight(double weightValue);
    virtual void toDefaultWeight(Gtk::Widget *widgWeight);
    virtual void toMakeCusp(Gtk::Widget *widgWeight);
    virtual void toWeight();

    // TODO make this private
    ScalarParam steps;

private:
    std::vector<Geom::Point> points;
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

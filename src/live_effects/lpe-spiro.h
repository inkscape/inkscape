#ifndef INKSCAPE_LPE_SPIRO_H
#define INKSCAPE_LPE_SPIRO_H

/*
 * Inkscape::LPESpiro
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/point.h"
#include "ui/widget/registered-widget.h"



namespace Inkscape {
namespace LivePathEffect {

class LPESpiro : public Effect {
public:
    LPESpiro(LivePathEffectObject *lpeobject);
    virtual ~LPESpiro();

    virtual void setup_nodepath(Inkscape::NodePath::Path *np);
    virtual void doEffect(SPCurve * curve);

private:
    LPESpiro(const LPESpiro&);
    LPESpiro& operator=(const LPESpiro&);
};

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif

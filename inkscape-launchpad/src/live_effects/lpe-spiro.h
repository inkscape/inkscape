#ifndef INKSCAPE_LPE_SPIRO_H
#define INKSCAPE_LPE_SPIRO_H

/*
 * Inkscape::LPESpiro
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"


namespace Inkscape {
namespace LivePathEffect {

class LPESpiro : public Effect {
public:
    LPESpiro(LivePathEffectObject *lpeobject);
    virtual ~LPESpiro();

    virtual LPEPathFlashType pathFlashType() const { return SUPPRESS_FLASH; }

    virtual void doEffect(SPCurve * curve);

private:
    LPESpiro(const LPESpiro&);
    LPESpiro& operator=(const LPESpiro&);
};

void sp_spiro_do_effect(SPCurve *curve);

}; //namespace LivePathEffect
}; //namespace Inkscape

#endif

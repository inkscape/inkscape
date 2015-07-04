#ifndef INKSCAPE_LPE_SHOW_HANDLES_H
#define INKSCAPE_LPE_SHOW_HANDLES_H

/*
 * Authors:
 *   Jabier Arraiza Cenoz
*
* Copyright (C) Jabier Arraiza Cenoz 2014 <jabier.arraiza@marker.es>
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/lpegroupbbox.h"
#include "live_effects/parameter/bool.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEShowHandles : public Effect , GroupBBoxEffect {

public:
    LPEShowHandles(LivePathEffectObject *lpeobject);
    virtual ~LPEShowHandles() {}

    virtual void doOnApply(SPLPEItem const* lpeitem);

    virtual void doBeforeEffect (SPLPEItem const* lpeitem);

    virtual void generateHelperPath(Geom::PathVector result);

    virtual void drawNode(Geom::Point p);

    virtual void drawHandle(Geom::Point p);

    virtual void drawHandleLine(Geom::Point p,Geom::Point p2);

protected:

    virtual Geom::PathVector doEffect_path (Geom::PathVector const & path_in);

private:

    BoolParam nodes;
    BoolParam handles;
    BoolParam original_path;
    ScalarParam scale_nodes_and_handles;
    ScalarParam rotate_nodes;
    double stroke_width;
    static bool alerts_off;

    Geom::PathVector outline_path;

    LPEShowHandles(const LPEShowHandles &);
    LPEShowHandles &operator=(const LPEShowHandles &);

};

}; //namespace LivePathEffect
}; //namespace Inkscape
#endif

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

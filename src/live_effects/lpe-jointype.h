/* Authors:
 * Liam P White
 *
 * Copyright (C) 2014 Authors
 *
 * Released under GNU GPL v2, read the file COPYING for more information
 */

#ifndef INKSCAPE_LPE_JOINTYPE_H
#define INKSCAPE_LPE_JOINTYPE_H

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/point.h"
#include "live_effects/parameter/enum.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEJoinType : public Effect {
public:
    LPEJoinType(LivePathEffectObject *lpeobject);
    virtual ~LPEJoinType();

    virtual void doOnApply(SPLPEItem const* lpeitem);
    virtual void doOnRemove(SPLPEItem const* lpeitem);
    virtual std::vector <Geom::Path> doEffect_path (std::vector<Geom::Path> const & path_in);

private:
    LPEJoinType(const LPEJoinType&);
    LPEJoinType& operator=(const LPEJoinType&);

    ScalarParam line_width;
    EnumParam<unsigned> linecap_type;
    EnumParam<unsigned> linejoin_type;
    ScalarParam start_lean;
    ScalarParam end_lean;
    ScalarParam miter_limit;
    BoolParam attempt_force_join;
    bool was_initialized;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :

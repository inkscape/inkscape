/** @file
 * @brief LPE perspective path effect implementation
 */
/* Authors:
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Johan Engelen <j.b.c.engelen@utwente.nl>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_LPE_PERSPECTIVE_PATH_H
#define INKSCAPE_LPE_PERSPECTIVE_PATH_H

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/bool.h"
#include "live_effects/lpegroupbbox.h"

#include <vector>
#include "2geom/point.h"

namespace Inkscape {
namespace LivePathEffect {

namespace PP {
  // we need a separate namespace to avoid clashes with other LPEs
  class KnotHolderEntityOffset;
}

class LPEPerspectivePath : public Effect, GroupBBoxEffect {
public:
    LPEPerspectivePath(LivePathEffectObject *lpeobject);
    virtual ~LPEPerspectivePath();
    virtual void doBeforeEffect (SPLPEItem const* lpeitem);
    virtual void doOnApply(SPLPEItem const* lpeitem);
    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

    virtual void refresh(Gtk::Entry* perspective);
    virtual Gtk::Widget * newWidget();
    /* the knotholder entity classes must be declared friends */
    friend class PP::KnotHolderEntityOffset;
    void addKnotHolderEntities(KnotHolder *knotholder, SPDesktop *desktop, SPItem *item);

private:
    // add the parameters for your effect here:
    ScalarParam scalex;
    ScalarParam scaley;
    // TODO: rewrite this using a PointParam instead of two ScalarParams
    ScalarParam offsetx;
    ScalarParam offsety;
    BoolParam uses_plane_xy;
    // there are all kinds of parameters. Check the /live_effects/parameter directory which types exist!
    Geom::Point orig;

    LPEPerspectivePath(const LPEPerspectivePath&);
    LPEPerspectivePath& operator=(const LPEPerspectivePath&);

    std::vector<Geom::Point> handles;
    double tmat[3][4];
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

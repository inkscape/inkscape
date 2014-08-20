#ifndef INKSCAPE_LPE_TEXT_LABEL_H
#define INKSCAPE_LPE_TEXT_LABEL_H

/** \file
 * LPE <text_label> implementation
 */
/*
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *
 * Copyright (C) Maximilian Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/text.h"

namespace Inkscape {
namespace LivePathEffect {

class LPETextLabel : public Effect {
public:
    LPETextLabel(LivePathEffectObject *lpeobject);
    virtual ~LPETextLabel();

    virtual Geom::Piecewise<Geom::D2<Geom::SBasis> > doEffect_pwd2 (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & pwd2_in);

private:
    TextParam label;

    LPETextLabel(const LPETextLabel&);
    LPETextLabel& operator=(const LPETextLabel&);
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

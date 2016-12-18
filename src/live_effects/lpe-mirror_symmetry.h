#ifndef INKSCAPE_LPE_MIRROR_SYMMETRY_H
#define INKSCAPE_LPE_MIRROR_SYMMETRY_H

/** \file
 * LPE <mirror_symmetry> implementation: mirrors a path with respect to a given line.
 */
/*
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
 *   Jabiertxof
 *
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 * Copyright (C) Maximilin Albert 2008 <maximilian.albert@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"
#include "live_effects/parameter/point.h"
#include "live_effects/parameter/path.h"
#include "live_effects/parameter/enum.h"
#include "live_effects/lpegroupbbox.h"

namespace Inkscape {
namespace LivePathEffect {

enum ModeType {
    MT_V,
    MT_H,
    MT_FREE,
    MT_X,
    MT_Y,
    MT_END
};

class LPEMirrorSymmetry : public Effect, GroupBBoxEffect {
public:
    LPEMirrorSymmetry(LivePathEffectObject *lpeobject);
    virtual ~LPEMirrorSymmetry();
    virtual void doOnApply (SPLPEItem const* lpeitem);
    virtual void doBeforeEffect (SPLPEItem const* lpeitem);
    virtual void transform_multiply(Geom::Affine const& postmul, bool set);
    virtual Geom::PathVector doEffect_path (Geom::PathVector const & path_in);
    virtual Gtk::Widget * newWidget();

protected:
    virtual void addCanvasIndicators(SPLPEItem const *lpeitem, std::vector<Geom::PathVector> &hp_vec);

private:
    EnumParam<ModeType> mode;
    BoolParam discard_orig_path;
    BoolParam fuse_paths;
    BoolParam oposite_fuse;
    PointParam start_point;
    PointParam end_point;
    PointParam center_point;
    Geom::Point previous_center;

    LPEMirrorSymmetry(const LPEMirrorSymmetry&);
    LPEMirrorSymmetry& operator=(const LPEMirrorSymmetry&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif

#ifndef INKSCAPE_LPE_MIRROR_SYMMETRY_H
#define INKSCAPE_LPE_MIRROR_SYMMETRY_H

/** \file
 * LPE <mirror_symmetry> implementation: mirrors a path with respect to a given line.
 */
/*
 * Authors:
 *   Maximilian Albert
 *   Johan Engelen
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

namespace Inkscape {
namespace LivePathEffect {

class LPEMirrorSymmetry : public Effect {
public:
    LPEMirrorSymmetry(LivePathEffectObject *lpeobject);
    virtual ~LPEMirrorSymmetry();

    virtual void doOnApply (SPLPEItem const* lpeitem);

    virtual void doBeforeEffect (SPLPEItem const* lpeitem);

    virtual Geom::PathVector doEffect_path (Geom::PathVector const & path_in);

private:
    BoolParam discard_orig_path;
    PathParam reflection_line;

    LPEMirrorSymmetry(const LPEMirrorSymmetry&);
    LPEMirrorSymmetry& operator=(const LPEMirrorSymmetry&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif

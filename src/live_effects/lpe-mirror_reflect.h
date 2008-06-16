#ifndef INKSCAPE_LPE_MIRROR_REFLECT_H
#define INKSCAPE_LPE_MIRROR_REFLECT_H

/** \file
 * LPE <mirror_reflection> implementation: mirrors a path with respect to a given line.
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

class LPEMirrorReflect : public Effect {
public:
    LPEMirrorReflect(LivePathEffectObject *lpeobject);
    virtual ~LPEMirrorReflect();

    virtual void acceptParamPath (SPPath *param_path);
    virtual int acceptsNumParams() { return 2; }

    virtual std::vector<Geom::Path> doEffect_path (std::vector<Geom::Path> const & path_in);

private:
    PathParam reflection_line;

    LPEMirrorReflect(const LPEMirrorReflect&);
    LPEMirrorReflect& operator=(const LPEMirrorReflect&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif

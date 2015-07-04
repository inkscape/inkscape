#ifndef INKSCAPE_LPE_CONSTRUCTGRID_H
#define INKSCAPE_LPE_CONSTRUCTGRID_H

/** \file
 * Implementation of the construct grid LPE, see lpe-constructgrid.cpp
 */

/*
 * Authors:
 *   Johan Engelen
*
* Copyright (C) Johan Engelen 2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/effect.h"
#include "live_effects/parameter/parameter.h"

namespace Inkscape {
namespace LivePathEffect {

class LPEConstructGrid : public Effect {
public:
    LPEConstructGrid(LivePathEffectObject *lpeobject);
    virtual ~LPEConstructGrid();

    virtual Geom::PathVector doEffect_path (Geom::PathVector const & path_in);

private:
    ScalarParam nr_x;
    ScalarParam nr_y;

    LPEConstructGrid(const LPEConstructGrid&);
    LPEConstructGrid& operator=(const LPEConstructGrid&);
};

} //namespace LivePathEffect
} //namespace Inkscape

#endif // INKSCAPE_LPE_CONSTRUCTGRID_H

#ifndef SEEN_NR_FILTER_TURBULENCE_H
#define SEEN_NR_FILTER_TURBULENCE_H

/*
 * feTurbulence filter primitive renderer
 *
 * Authors:
 *   World Wide Web Consortium <http://www.w3.org/>
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * This file has a considerable amount of code adapted from
 *  the W3C SVG filter specs, available at:
 *  http://www.w3.org/TR/SVG11/filters.html#feTurbulence
 *
 * W3C original code is licensed under the terms of
 *  the (GPL compatible) W3C® SOFTWARE NOTICE AND LICENSE:
 *  http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231
 *
 * Copyright (C) 2007 authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <2geom/point.h>

#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"

namespace Inkscape {
namespace Filters {

enum FilterTurbulenceType {
    TURBULENCE_FRACTALNOISE,
    TURBULENCE_TURBULENCE,
    TURBULENCE_ENDTYPE
};

class TurbulenceGenerator;

class FilterTurbulence : public FilterPrimitive {
public:
    FilterTurbulence();
    static FilterPrimitive *create();
    virtual ~FilterTurbulence();

    virtual void render_cairo(FilterSlot &slot);
    virtual double complexity(Geom::Affine const &ctm);
    virtual bool uses_background() { return false; }

    void set_baseFrequency(int axis, double freq);
    void set_numOctaves(int num);
    void set_seed(double s);
    void set_stitchTiles(bool st);
    void set_type(FilterTurbulenceType t);
    void set_updated(bool u);
private:

    TurbulenceGenerator *gen;

    void turbulenceInit(long seed);

    double XbaseFrequency, YbaseFrequency;
    int numOctaves;
    double seed;
    bool stitchTiles;
    FilterTurbulenceType type;
    bool updated;
    unsigned char *pix_data;

    double fTileWidth;
    double fTileHeight;

    double fTileX;
    double fTileY;

};

} /* namespace Filters */
} /* namespace Inkscape */

#endif /* __NR_FILTER_TURBULENCE_H__ */
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

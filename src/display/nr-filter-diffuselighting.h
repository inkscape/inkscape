#ifndef SEEN_NR_FILTER_DIFFUSELIGHTING_H
#define SEEN_NR_FILTER_DIFFUSELIGHTING_H

/*
 * feDiffuseLighting renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *   Jean-Rene Reinhard <jr@komite.net>
 * 
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-light-types.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-units.h"

class SPFeDistantLight;
class SPFePointLight;
class SPFeSpotLight;
struct SVGICCColor;
typedef unsigned int guint32;

namespace Inkscape {
namespace Filters {
    
class FilterDiffuseLighting : public FilterPrimitive {
public:
    FilterDiffuseLighting();
    static FilterPrimitive *create();
    virtual ~FilterDiffuseLighting();
    virtual void render_cairo(FilterSlot &slot);
    virtual void set_icc(SVGICCColor *icc_color);
    virtual void area_enlarge(Geom::IntRect &area, Geom::Affine const &trans);
    virtual double complexity(Geom::Affine const &ctm);

    union {
        SPFeDistantLight *distant;
        SPFePointLight *point;
        SPFeSpotLight *spot;
    } light;
    LightType light_type;
    double diffuseConstant;
    double surfaceScale;
    guint32 lighting_color;

private:
    SVGICCColor *icc;
};

} /* namespace Filters */
} /* namespace Inkscape */

#endif /* __NR_FILTER_DIFFUSELIGHTING_H__ */
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

#ifndef __NR_FILTER_SPECULARLIGHTING_H__
#define __NR_FILTER_SPECULARLIGHTING_H__

/*
 * feSpecularLighting renderer
 *
 * Authors:
 *   Niko Kiirala <niko@kiirala.com>
 *   Jean-Rene Reinhard <jr@komite.net>
 * 
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gdk/gdk.h>
#include "display/nr-light-types.h"
#include "display/nr-filter-primitive.h"

class SPFeDistantLight;
class SPFePointLight;
class SPFeSpotLight;
struct SVGICCColor;

namespace Inkscape {
namespace Filters {

class FilterSlot;

class FilterSpecularLighting : public FilterPrimitive {
public:
    FilterSpecularLighting();
    static FilterPrimitive *create();
    virtual ~FilterSpecularLighting();

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
    gdouble surfaceScale;
    gdouble specularConstant;
    gdouble specularExponent;
    guint32 lighting_color;

private:
    SVGICCColor *icc;
};

} /* namespace Filters */
} /* namespace Inkscape */

#endif /* __NR_FILTER_SPECULARLIGHTING_H__ */
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

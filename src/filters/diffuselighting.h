/** @file
 * @brief SVG diffuse lighting filter effect
 *//*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Jean-Rene Reinhard <jr@komite.net>
 *
 * Copyright (C) 2006-2007 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SP_FEDIFFUSELIGHTING_H_SEEN
#define SP_FEDIFFUSELIGHTING_H_SEEN

#include "sp-filter-primitive.h"
#include "number-opt-number.h"

#define SP_TYPE_FEDIFFUSELIGHTING (sp_feDiffuseLighting_get_type())
#define SP_FEDIFFUSELIGHTING(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FEDIFFUSELIGHTING, SPFeDiffuseLighting))
#define SP_FEDIFFUSELIGHTING_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FEDIFFUSELIGHTING, SPFeDiffuseLightingClass))
#define SP_IS_FEDIFFUSELIGHTING(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEDIFFUSELIGHTING))
#define SP_IS_FEDIFFUSELIGHTING_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FEDIFFUSELIGHTING))

struct SVGICCColor;

namespace Inkscape {
namespace Filters {
class FilterDiffuseLighting;
} }

struct SPFeDiffuseLighting : public SPFilterPrimitive {
    gfloat surfaceScale;
    guint surfaceScale_set : 1;
    gfloat diffuseConstant;
    guint diffuseConstant_set : 1;
    NumberOptNumber kernelUnitLength;
    guint32 lighting_color;
    guint lighting_color_set : 1;
    Inkscape::Filters::FilterDiffuseLighting *renderer;
    SVGICCColor *icc;
};

struct SPFeDiffuseLightingClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feDiffuseLighting_get_type();

#endif /* !SP_FEDIFFUSELIGHTING_H_SEEN */

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

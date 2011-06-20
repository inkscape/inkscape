/** @file
 * @brief SVG specular lighting filter effect
 *//*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Jean-Rene Reinhard <jr@komite.net>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *               2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SP_FESPECULARLIGHTING_H_SEEN
#define SP_FESPECULARLIGHTING_H_SEEN

#include "sp-filter-primitive.h"
#include "number-opt-number.h"

#define SP_TYPE_FESPECULARLIGHTING (sp_feSpecularLighting_get_type())
#define SP_FESPECULARLIGHTING(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FESPECULARLIGHTING, SPFeSpecularLighting))
#define SP_FESPECULARLIGHTING_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FESPECULARLIGHTING, SPFeSpecularLightingClass))
#define SP_IS_FESPECULARLIGHTING(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FESPECULARLIGHTING))
#define SP_IS_FESPECULARLIGHTING_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FESPECULARLIGHTING))

namespace Inkscape {
namespace Filters {
class FilterSpecularLighting;
}
}

class SPFeSpecularLightingClass;

struct SPFeSpecularLighting : public SPFilterPrimitive {
    gfloat surfaceScale;
    guint surfaceScale_set : 1;
    gfloat specularConstant;
    guint specularConstant_set : 1;
    gfloat specularExponent;
    guint specularExponent_set : 1;
    NumberOptNumber kernelUnitLength;
    guint32 lighting_color;
    guint lighting_color_set : 1;

    Inkscape::Filters::FilterSpecularLighting *renderer;
};

struct SPFeSpecularLightingClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feSpecularLighting_get_type();


#endif /* !SP_FESPECULARLIGHTING_H_SEEN */

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

#ifndef SP_FESPECULARLIGHTING_H_SEEN
#define SP_FESPECULARLIGHTING_H_SEEN

/** \file
 * SVG <feSpecularLighting> implementation, see SpecularLighting.cpp.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Jean-Rene Reinhard <jr@komite.net>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *               2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-filter.h"
#include "specularlighting-fns.h"

namespace Inkscape {
namespace Filters {
class FilterSpecularLighting;
}
}

/* FeSpecularLighting base class */
class SPFeSpecularLightingClass;

struct SPFeSpecularLighting : public SPFilterPrimitive {
    /** SPECULARLIGHTING ATTRIBUTES HERE */
    /** surfaceScale attribute */
    gfloat surfaceScale;
    guint surfaceScale_set : 1;
    /** specularConstant attribute */
    gfloat specularConstant;
    guint specularConstant_set : 1;
    /** specularConstant attribute */
    gfloat specularExponent;
    guint specularExponent_set : 1;
    /** kernelUnitLenght attribute */
    NumberOptNumber kernelUnitLength;
    /** lighting-color property */
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

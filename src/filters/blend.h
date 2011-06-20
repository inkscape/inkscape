/** @file
 * @brief SVG blend filter effect
 *//*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006,2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SP_FEBLEND_H_SEEN
#define SP_FEBLEND_H_SEEN

#include "sp-filter-primitive.h"
#include "display/nr-filter-blend.h"

#define SP_TYPE_FEBLEND (sp_feBlend_get_type())
#define SP_FEBLEND(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FEBLEND, SPFeBlend))
#define SP_FEBLEND_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FEBLEND, SPFeBlendClass))
#define SP_IS_FEBLEND(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FEBLEND))
#define SP_IS_FEBLEND_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FEBLEND))

class SPFeBlendClass;

struct SPFeBlend : public SPFilterPrimitive {
    Inkscape::Filters::FilterBlendMode blend_mode;
    int in2;
};

struct SPFeBlendClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feBlend_get_type();


#endif /* !SP_FEBLEND_H_SEEN */

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

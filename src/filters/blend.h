#ifndef SP_FEBLEND_H_SEEN
#define SP_FEBLEND_H_SEEN

/** \file
 * SVG <feBlend> implementation, see Blend.cpp.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006,2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-filter.h"
#include "blend-fns.h"

#include "display/nr-filter-blend.h"

/* FeBlend base class */
class SPFeBlendClass;

struct SPFeBlend : public SPFilterPrimitive {
    /** BLEND ATTRIBUTES HERE */
    NR::FilterBlendMode blend_mode;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

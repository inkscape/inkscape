#ifndef SP_FECOLORMATRIX_H_SEEN
#define SP_FECOLORMATRIX_H_SEEN

/** \file
 * SVG <feColorMatrix> implementation, see ColorMatrix.cpp.
 */
/*
 * Authors:
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-filter.h"
#include "colormatrix-fns.h"
#include "display/nr-filter-colormatrix.h"
#include <vector>

/* FeColorMatrix base class */
class SPFeColorMatrixClass;

struct SPFeColorMatrix : public SPFilterPrimitive {
    /** COLORMATRIX ATTRIBUTES HERE */
    Inkscape::Filters::FilterColorMatrixType type;
    gdouble value;
    std::vector<gdouble> values;
};

struct SPFeColorMatrixClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feColorMatrix_get_type();


#endif /* !SP_FECOLORMATRIX_H_SEEN */

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

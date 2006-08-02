#ifndef SP_FECONVOLVEMATRIX_H_SEEN
#define SP_FECONVOLVEMATRIX_H_SEEN

/** \file
 * SVG <feConvolveMatrix> implementation, see sp-feConvolveMatrix.cpp.
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
#include "sp-feconvolvematrix-fns.h"

/* FeConvolveMatrix base class */
class SPFeConvolveMatrixClass;

struct SPFeConvolveMatrix : public SPFilter {
    /** CONVOLVEMATRIX ATTRIBUTES HERE */
    
};

struct SPFeConvolveMatrixClass {
    SPFilterClass parent_class;
};

GType sp_feConvolveMatrix_get_type();


#endif /* !SP_FECONVOLVEMATRIX_H_SEEN */

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

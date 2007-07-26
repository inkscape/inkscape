#ifndef SP_FEIMAGE_H_SEEN
#define SP_FEIMAGE_H_SEEN

/** \file
 * SVG <feImage> implementation, see sp-feImage.cpp.
 */
/*
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-filter.h"
#include "sp-feimage-fns.h"

/* FeImage base class */
class SPFeImageClass;

struct SPFeImage : public SPFilterPrimitive {
    /** IMAGE ATTRIBUTES HERE */
    /*
        Apparently there's no attribute to keep here 
        since 'in' and 'xlink:href' are common filter attributes.
        --Juca
    */
};

struct SPFeImageClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feImage_get_type();


#endif /* !SP_FEIMAGE_H_SEEN */

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

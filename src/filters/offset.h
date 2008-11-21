#ifndef SP_FEOFFSET_H_SEEN
#define SP_FEOFFSET_H_SEEN

/** \file
 * SVG <feOffset> implementation, see Offset.cpp.
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
#include "offset-fns.h"

/* FeOffset base class */
class SPFeOffsetClass;

struct SPFeOffset : public SPFilterPrimitive {
    /** OFFSET ATTRIBUTES HERE */
    double dx, dy;
};

struct SPFeOffsetClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feOffset_get_type();


#endif /* !SP_FEOFFSET_H_SEEN */

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

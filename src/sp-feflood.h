#ifndef SP_FEFLOOD_H_SEEN
#define SP_FEFLOOD_H_SEEN

/** \file
 * SVG <feFlood> implementation, see sp-feFlood.cpp.
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
#include "sp-feflood-fns.h"

/* FeFlood base class */
class SPFeFloodClass;

struct SPFeFlood : public SPFilter {
    /** FLOOD ATTRIBUTES HERE */
    
};

struct SPFeFloodClass {
    SPFilterClass parent_class;
};

GType sp_feFlood_get_type();


#endif /* !SP_FEFLOOD_H_SEEN */

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

#ifndef SP_FECOMPONENTTRANSFER_H_SEEN
#define SP_FECOMPONENTTRANSFER_H_SEEN

/** \file
 * SVG <feComponentTransfer> implementation, see ComponentTransfer.cpp.
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
#include "componenttransfer-fns.h"
#include "display/nr-filter-component-transfer.h"
#include <vector>

/* FeComponentTransfer base class */
class SPFeComponentTransferClass;

struct SPFeComponentTransfer : public SPFilterPrimitive {
    /** COMPONENTTRANSFER ATTRIBUTES HERE */

    NR::FilterComponentTransfer *renderer;
};

struct SPFeComponentTransferClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feComponentTransfer_get_type();


#endif /* !SP_FECOMPONENTTRANSFER_H_SEEN */

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

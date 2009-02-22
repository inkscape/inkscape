#ifndef SP_FETILE_H_SEEN
#define SP_FETILE_H_SEEN

/** \file
 * SVG <feTile> implementation, see Tile.cpp.
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
#include "tile-fns.h"

#include "display/nr-filter.h"
#include "display/nr-filter-tile.h"

/* FeTile base class */
class SPFeTileClass;

struct SPFeTile : public SPFilterPrimitive {
    /** TILE ATTRIBUTES HERE */
    
};

struct SPFeTileClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feTile_get_type();


#endif /* !SP_FETILE_H_SEEN */

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

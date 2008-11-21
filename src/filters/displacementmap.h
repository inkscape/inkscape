#ifndef SP_FEDISPLACEMENTMAP_H_SEEN
#define SP_FEDISPLACEMENTMAP_H_SEEN

/** \file
 * SVG <feDisplacementMap> implementation, see DisplacementMap.cpp.
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
#include "displacementmap-fns.h"

enum FilterDisplacementMapChannelSelector {
    DISPLACEMENTMAP_CHANNEL_RED,
    DISPLACEMENTMAP_CHANNEL_GREEN,
    DISPLACEMENTMAP_CHANNEL_BLUE,
    DISPLACEMENTMAP_CHANNEL_ALPHA,
    DISPLACEMENTMAP_CHANNEL_ENDTYPE
};

/* FeDisplacementMap base class */
class SPFeDisplacementMapClass;

struct SPFeDisplacementMap : public SPFilterPrimitive {
    /** DISPLACEMENTMAP ATTRIBUTES HERE */
    int in2; 
    double scale;
    FilterDisplacementMapChannelSelector xChannelSelector;
    FilterDisplacementMapChannelSelector yChannelSelector;
};

struct SPFeDisplacementMapClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feDisplacementMap_get_type();


#endif /* !SP_FEDISPLACEMENTMAP_H_SEEN */

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

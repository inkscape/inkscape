#ifndef SP_FESPECULARLIGHTING_H_SEEN
#define SP_FESPECULARLIGHTING_H_SEEN

/** \file
 * SVG <feSpecularLighting> implementation, see sp-feSpecularLighting.cpp.
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
#include "sp-fespecularlighting-fns.h"

/* FeSpecularLighting base class */
class SPFeSpecularLightingClass;

struct SPFeSpecularLighting : public SPFilter {
    /** SPECULARLIGHTING ATTRIBUTES HERE */
    
};

struct SPFeSpecularLightingClass {
    SPFilterClass parent_class;
};

GType sp_feSpecularLighting_get_type();


#endif /* !SP_FESPECULARLIGHTING_H_SEEN */

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

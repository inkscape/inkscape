/** @file
 * @brief SVG turbulence filter effect
 *//*
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   Hugo Rodrigues <haa.rodrigues@gmail.com>
 *
 * Copyright (C) 2006 Hugo Rodrigues
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SP_FETURBULENCE_H_SEEN
#define SP_FETURBULENCE_H_SEEN

#include "sp-filter-primitive.h"
#include "number-opt-number.h"
#include "display/nr-filter-turbulence.h"

#define SP_TYPE_FETURBULENCE (sp_feTurbulence_get_type())
#define SP_FETURBULENCE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_FETURBULENCE, SPFeTurbulence))
#define SP_FETURBULENCE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_FETURBULENCE, SPFeTurbulenceClass))
#define SP_IS_FETURBULENCE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_FETURBULENCE))
#define SP_IS_FETURBULENCE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_FETURBULENCE))

/* FeTurbulence base class */
class SPFeTurbulenceClass;

struct SPFeTurbulence : public SPFilterPrimitive {
    /** TURBULENCE ATTRIBUTES HERE */
    NumberOptNumber baseFrequency;
    int numOctaves;
    double seed;
    bool stitchTiles;
    Inkscape::Filters::FilterTurbulenceType type;
    SVGLength x, y, height, width;
    bool updated;
};

struct SPFeTurbulenceClass {
    SPFilterPrimitiveClass parent_class;
};

GType sp_feTurbulence_get_type();


#endif /* !SP_FETURBULENCE_H_SEEN */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

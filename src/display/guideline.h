#ifndef __SP_GUIDELINE_H__
#define __SP_GUIDELINE_H__

/*
 * The visual representation of SPGuide.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Johan Engelen
 *
 * Copyright (C) 2000-2002 Lauris Kaplinski
 * Copyright (C) 2007 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-canvas.h"
#include <2geom/point.h>

#define SP_TYPE_GUIDELINE (sp_guideline_get_type())
#define SP_GUIDELINE(o) (GTK_CHECK_CAST((o), SP_TYPE_GUIDELINE, SPGuideLine))
#define SP_IS_GUIDELINE(o) (GTK_CHECK_TYPE((o), SP_TYPE_GUIDELINE))

class SPCtrlPoint;

struct SPGuideLine {
    SPCanvasItem item;
    SPCtrlPoint *origin; // unlike 'item', this is only held locally

    guint32 rgba;

    Geom::Point normal_to_line;
    Geom::Point point_on_line;
    double angle;

    unsigned int sensitive : 1;

    inline bool is_horizontal() const { return (normal_to_line[Geom::X] == 0.); };
    inline bool is_vertical() const { return (normal_to_line[Geom::Y] == 0.); };
};

struct SPGuideLineClass {
    SPCanvasItemClass parent_class;
};

GType sp_guideline_get_type();

SPCanvasItem *sp_guideline_new(SPCanvasGroup *parent, Geom::Point point_on_line, Geom::Point normal);

void sp_guideline_set_position(SPGuideLine *gl, Geom::Point point_on_line);
void sp_guideline_set_normal(SPGuideLine *gl, Geom::Point normal_to_line);
void sp_guideline_set_color(SPGuideLine *gl, unsigned int rgba);
void sp_guideline_set_sensitive(SPGuideLine *gl, int sensitive);
void sp_guideline_delete(SPGuideLine *gl);

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

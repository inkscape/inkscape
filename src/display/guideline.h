#ifndef SEEN_SP_GUIDELINE_H
#define SEEN_SP_GUIDELINE_H

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

#include <2geom/point.h>
#include "sp-canvas-item.h"

#define SP_TYPE_GUIDELINE (sp_guideline_get_type())
#define SP_GUIDELINE(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_GUIDELINE, SPGuideLine))
#define SP_IS_GUIDELINE(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_GUIDELINE))

struct SPCtrl;

struct SPGuideLine {
    SPCanvasItem item;
    Geom::Affine affine;

    SPCtrl *origin; // unlike 'item', this is only held locally

    guint32 rgba;

    char* label;
    bool locked;
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

SPCanvasItem *sp_guideline_new(SPCanvasGroup *parent, char* label, Geom::Point point_on_line, Geom::Point normal);

void sp_guideline_set_label(SPGuideLine *gl, const char* label);
void sp_guideline_set_locked(SPGuideLine *gl, const bool locked);
void sp_guideline_set_position(SPGuideLine *gl, Geom::Point point_on_line);
void sp_guideline_set_normal(SPGuideLine *gl, Geom::Point normal_to_line);
void sp_guideline_set_color(SPGuideLine *gl, unsigned int rgba);
void sp_guideline_set_sensitive(SPGuideLine *gl, int sensitive);
void sp_guideline_delete(SPGuideLine *gl);

#endif // SEEN_SP_GUIDELINE_H

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

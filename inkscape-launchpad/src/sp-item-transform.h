#ifndef SEEN_SP_ITEM_TRANSFORM_H
#define SEEN_SP_ITEM_TRANSFORM_H

#include <2geom/forward.h>

class SPItem;

void sp_item_rotate_rel(SPItem *item, Geom::Rotate const &rotation);
void sp_item_scale_rel (SPItem *item, Geom::Scale const &scale);
void sp_item_skew_rel (SPItem *item, double skewX, double skewY);
void sp_item_move_rel(SPItem *item, Geom::Translate const &tr);

Geom::Affine get_scale_transform_for_uniform_stroke (Geom::Rect const &bbox_visual, double stroke_x, double stroke_y, bool transform_stroke, bool preserve, double x0, double y0, double x1, double y1);
Geom::Affine get_scale_transform_for_variable_stroke (Geom::Rect const &bbox_visual, Geom::Rect const &bbox_geom, bool transform_stroke, bool preserve, double x0, double y0, double x1, double y1);
Geom::Rect get_visual_bbox (Geom::OptRect const &initial_geom_bbox, Geom::Affine const &abs_affine, double const initial_strokewidth, bool const transform_stroke);


#endif // SEEN_SP_ITEM_TRANSFORM_H

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

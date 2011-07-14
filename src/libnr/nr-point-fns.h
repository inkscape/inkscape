#ifndef __NR_POINT_OPS_H__
#define __NR_POINT_OPS_H__

#include <2geom/point.h>

Geom::Point snap_vector_midpoint (Geom::Point const &p, Geom::Point const &begin, Geom::Point const &end, double snap);

double get_offset_between_points (Geom::Point const &p, Geom::Point const &begin, Geom::Point const &end);

Geom::Point project_on_linesegment(Geom::Point const &p, Geom::Point const &p1, Geom::Point const &p2); 

#endif /* !__NR_POINT_OPS_H__ */

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

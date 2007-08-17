#define __LINE_GEOMETRY_C__

/*
 * Routines for dealing with lines (intersections, etc.)
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "line-geometry.h"
#include "inkscape.h"
#include "desktop-style.h"
#include "desktop-handles.h"
#include "display/sp-canvas.h"
#include "display/sodipodi-ctrl.h"
//#include "display/curve.cpp"

namespace Box3D {

/** 
 * Draw a line beginning at 'start'. If is_endpoint is true, use 'vec' as the endpoint
 * of the segment. Otherwise interpret it as the direction of the line.
 * FIXME: Think of a better way to distinguish between the two constructors of lines.
 */
Line::Line(NR::Point const &start, NR::Point const &vec, bool is_endpoint) {
    pt = start;
    if (is_endpoint)
        v_dir = vec - start;
    else
    	v_dir = vec;
    normal = v_dir.ccw();
    d0 = NR::dot(normal, pt);
}

Line::Line(Line const &line) {
    pt = line.pt;
    v_dir = line.v_dir;
    normal = line.normal;
    d0 = line.d0;
}

Line &Line::operator=(Line const &line) {
    pt = line.pt;
    v_dir = line.v_dir;
    normal = line.normal;
    d0 = line.d0;

    return *this;
}

NR::Maybe<NR::Point> Line::intersect(Line const &line) {
    NR::Coord denom = NR::dot(v_dir, line.normal);
    g_return_val_if_fail(fabs(denom) > 1e-6, NR::Nothing());

    NR::Coord lambda = (line.d0 - NR::dot(pt, line.normal)) / denom;
    return pt + lambda * v_dir;
}

void Line::set_direction(NR::Point const &dir)
{
    v_dir = dir;
    normal = v_dir.ccw();
    d0 = NR::dot(normal, pt);
}

NR::Point Line::closest_to(NR::Point const &pt)
{
	/* return the intersection of this line with a perpendicular line passing through pt */ 
    NR::Maybe<NR::Point> result = this->intersect(Line(pt, (this->v_dir).ccw(), false));
    g_return_val_if_fail (result, NR::Point (0.0, 0.0));
    return *result;
}

inline static double determinant (NR::Point const &a, NR::Point const &b)
{
    return (a[NR::X] * b[NR::Y] - a[NR::Y] * b[NR::X]);
}

/* The coordinates of w with respect to the basis {v1, v2} */
std::pair<double, double> coordinates (NR::Point const &v1, NR::Point const &v2, NR::Point const &w)
{
    double det = determinant (v1, v2);;
    if (fabs (det) < epsilon) {
        g_warning ("Vectors do not form a basis.\n");
        return std::make_pair (0.0, 0.0);
    }

    double lambda1 = determinant (w, v2) / det;
    double lambda2 = determinant (v1, w) / det;
    return std::make_pair (lambda1, lambda2);
}

/* whether w lies inside the sector spanned by v1 and v2 */
bool lies_in_sector (NR::Point const &v1, NR::Point const &v2, NR::Point const &w)
{
    std::pair<double, double> coords = coordinates (v1, v2, w);
    return (coords.first >= 0 and coords.second >= 0);
}

static double pos_angle (NR::Point A, NR::Point B)
{
    return fabs (NR::atan2 (A) - NR::atan2 (B));
}

/*
 * Returns the two corners of the quadrangle A, B, C, D spanning the edge that is hit by a semiline
 * starting at pt and going into direction dir.
 * If none of the sides is hit, it returns a pair containing two identical points.
 */
std::pair<NR::Point, NR::Point>
side_of_intersection (NR::Point const &A, NR::Point const &B, NR::Point const &C, NR::Point const &D,
                      NR::Point const &pt, NR::Point const &dir)
{
    NR::Point dir_A (A - pt);
    NR::Point dir_B (B - pt);
    NR::Point dir_C (C - pt);
    NR::Point dir_D (D - pt);

    std::pair<NR::Point, NR::Point> result;
    double angle = -1;
    double tmp_angle;

    if (lies_in_sector (dir_A, dir_B, dir)) {
        result = std::make_pair (A, B);
        angle = pos_angle (dir_A, dir_B);
    }
    if (lies_in_sector (dir_B, dir_C, dir)) {
        tmp_angle = pos_angle (dir_B, dir_C);
        if (tmp_angle > angle) {
            angle = tmp_angle;
            result = std::make_pair (B, C);
        }
    }
    if (lies_in_sector (dir_C, dir_D, dir)) {
        tmp_angle = pos_angle (dir_C, dir_D);
        if (tmp_angle > angle) {
            angle = tmp_angle;
            result = std::make_pair (C, D);
        }
    }
    if (lies_in_sector (dir_D, dir_A, dir)) {
        tmp_angle = pos_angle (dir_D, dir_A);
        if (tmp_angle > angle) {
            angle = tmp_angle;
            result = std::make_pair (D, A);
        }
    }
    if (angle == -1) {
        // no intersection found; return a pair containing two identical points
        return std::make_pair (A, A);
    } else {
        return result;
    }
}

void create_canvas_point(NR::Point const &pos, double size, guint32 rgba)
{
    SPDesktop *desktop = inkscape_active_desktop();
    SPCanvasItem * canvas_pt = sp_canvas_item_new(sp_desktop_controls(desktop), SP_TYPE_CTRL,
                          "size", size,
                          "filled", 1,
                          "fill_color", rgba,
                          "stroked", 1,
                          "stroke_color", 0x000000ff,
                          NULL);
    SP_CTRL(canvas_pt)->moveto(pos);
}

void create_canvas_line(NR::Point const &p1, NR::Point const &p2, guint32 rgba)
{
    SPDesktop *desktop = inkscape_active_desktop();
    SPCanvasItem *line = sp_canvas_item_new(sp_desktop_controls(desktop),
                                                            SP_TYPE_CTRLLINE, NULL);
    sp_ctrlline_set_coords(SP_CTRLLINE(line), p1, p2);
    sp_ctrlline_set_rgba32 (SP_CTRLLINE(line), rgba);
    sp_canvas_item_show (line);
}

} // namespace Box3D 
 
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

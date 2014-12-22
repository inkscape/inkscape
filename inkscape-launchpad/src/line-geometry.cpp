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
#include "desktop.h"
#include "desktop-style.h"

#include "display/sp-canvas.h"
#include "display/sp-ctrlline.h"
#include "display/sodipodi-ctrl.h"
#include "ui/control-manager.h"

using Inkscape::ControlManager;

namespace Box3D {

/** 
 * Draw a line beginning at 'start'. If is_endpoint is true, use 'vec' as the endpoint
 * of the segment. Otherwise interpret it as the direction of the line.
 * FIXME: Think of a better way to distinguish between the two constructors of lines.
 */
Line::Line(Geom::Point const &start, Geom::Point const &vec, bool is_endpoint):
    pt(start)
{
    if (is_endpoint)
        v_dir = vec - start;
    else
    	v_dir = vec;
    normal = v_dir.ccw();
    d0 = Geom::dot(normal, pt);
}

Line::Line(Line const &line):
    pt(line.pt),
    v_dir(line.v_dir),
    normal(line.normal),
    d0(line.d0)
{
}

Line &Line::operator=(Line const &line) {
    pt = line.pt;
    v_dir = line.v_dir;
    normal = line.normal;
    d0 = line.d0;

    return *this;
}

boost::optional<Geom::Point> Line::intersect(Line const &line) {
    Geom::Coord denom = Geom::dot(v_dir, line.normal);
    boost::optional<Geom::Point> no_point;
    if (fabs(denom) < 1e-6)
        return no_point;

    Geom::Coord lambda = (line.d0 - Geom::dot(pt, line.normal)) / denom;
    return pt + lambda * v_dir;
}

void Line::set_direction(Geom::Point const &dir)
{
    v_dir = dir;
    normal = v_dir.ccw();
    d0 = Geom::dot(normal, pt);
}

Geom::Point Line::closest_to(Geom::Point const &pt)
{
	/* return the intersection of this line with a perpendicular line passing through pt */ 
    boost::optional<Geom::Point> result = this->intersect(Line(pt, (this->v_dir).ccw(), false));
    g_return_val_if_fail (result, Geom::Point (0.0, 0.0));
    return *result;
}

double Line::lambda (Geom::Point const pt)
{
    double sign = (Geom::dot (pt - this->pt, this->v_dir) > 0) ? 1.0 : -1.0;
    double lambda = sign * Geom::L2 (pt - this->pt);
    // FIXME: It may speed things up (but how much?) if we assume that
    //        pt lies on the line and thus skip the following test
    Geom::Point test = point_from_lambda (lambda);
    if (!pts_coincide (pt, test)) {
        g_warning ("Point does not lie on line.\n");
        return 0;
    }
    return lambda;
}

/* The coordinates of w with respect to the basis {v1, v2} */
std::pair<double, double> coordinates (Geom::Point const &v1, Geom::Point const &v2, Geom::Point const &w)
{
    double det = determinant (v1, v2);;
    if (fabs (det) < epsilon) {
        // vectors are not linearly independent; we indicate this in the return value(s)
        return std::make_pair (HUGE_VAL, HUGE_VAL);
    }

    double lambda1 = determinant (w, v2) / det;
    double lambda2 = determinant (v1, w) / det;
    return std::make_pair (lambda1, lambda2);
}

/* whether w lies inside the sector spanned by v1 and v2 */
bool lies_in_sector (Geom::Point const &v1, Geom::Point const &v2, Geom::Point const &w)
{
    std::pair<double, double> coords = coordinates (v1, v2, w);
    if (coords.first == HUGE_VAL) {
        // catch the case that the vectors are not linearly independent
        // FIXME: Can we assume that it's safe to return true if the vectors point in different directions?
        return (Geom::dot (v1, v2) < 0);
    }
    return (coords.first >= 0 && coords.second >= 0);
}

bool lies_in_quadrangle (Geom::Point const &A, Geom::Point const &B, Geom::Point const &C, Geom::Point const &D, Geom::Point const &pt)
{
    return (lies_in_sector (D - A, B - A, pt - A) && lies_in_sector (D - C, B - C, pt - C));
}

static double pos_angle (Geom::Point v, Geom::Point w)
{
    return fabs (Geom::atan2 (v) - Geom::atan2 (w));
}

/*
 * Returns the two corners of the quadrangle A, B, C, D spanning the edge that is hit by a semiline
 * starting at pt and going into direction dir.
 * If none of the sides is hit, it returns a pair containing two identical points.
 */
std::pair<Geom::Point, Geom::Point>
side_of_intersection (Geom::Point const &A, Geom::Point const &B, Geom::Point const &C, Geom::Point const &D,
                      Geom::Point const &pt, Geom::Point const &dir)
{
    Geom::Point dir_A (A - pt);
    Geom::Point dir_B (B - pt);
    Geom::Point dir_C (C - pt);
    Geom::Point dir_D (D - pt);

    std::pair<Geom::Point, Geom::Point> result;
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

boost::optional<Geom::Point> Line::intersection_with_viewbox (SPDesktop *desktop)
{
    Geom::Rect vb = desktop->get_display_area();
    /* remaining viewbox corners */
    Geom::Point ul (vb.min()[Geom::X], vb.max()[Geom::Y]);
    Geom::Point lr (vb.max()[Geom::X], vb.min()[Geom::Y]);

    std::pair <Geom::Point, Geom::Point> e = side_of_intersection (vb.min(), lr, vb.max(), ul, this->pt, this->v_dir);
    if (e.first == e.second) {
        // perspective line lies outside the canvas
        return boost::optional<Geom::Point>();
    }

    Line line (e.first, e.second);
    return this->intersect (line);
}

void create_canvas_point(Geom::Point const &pos, double size, guint32 rgba)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    SPCanvasItem * canvas_pt = sp_canvas_item_new(desktop->getControls(), SP_TYPE_CTRL,
                          "size", size,
                          "filled", 1,
                          "fill_color", rgba,
                          "stroked", 1,
                          "stroke_color", 0x000000ff,
                          NULL);
    SP_CTRL(canvas_pt)->moveto(pos);
}

void create_canvas_line(Geom::Point const &p1, Geom::Point const &p2, guint32 rgba)
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    SPCtrlLine *line = ControlManager::getManager().createControlLine(desktop->getControls(), p1, p2);
    line->setRgba32(rgba);
    sp_canvas_item_show(line);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

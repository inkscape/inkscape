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

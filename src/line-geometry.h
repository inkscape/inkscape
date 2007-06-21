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

#ifndef SEEN_LINE_GEOMETRY_H
#define SEEN_LINE_GEOMETRY_H

#include "libnr/nr-point.h"
#include "libnr/nr-point-fns.h"
#include "libnr/nr-maybe.h"
#include "glib.h"
#include "display/sp-ctrlline.h"
#include "vanishing-point.h"

#include "document.h"
#include "ui/view/view.h"

namespace Box3D {

class Line {
public:
    Line(NR::Point const &start, NR::Point const &vec, bool is_endpoint = true);
    Line(Line const &line);
    Line &operator=(Line const &line);
    virtual NR::Maybe<NR::Point> intersect(Line const &line);
    void set_direction(NR::Point const &dir); // FIXME: Can we avoid this explicit assignment?
    
    NR::Point closest_to(NR::Point const &pt); // returns the point on the line closest to pt 

    friend inline std::ostream &operator<< (std::ostream &out_file, const Line &in_line);
	
private:
    NR::Point pt;
    NR::Point v_dir;
    NR::Point normal;
    NR::Coord d0;
};

/*** For testing purposes: Draw a knot/node of specified size and color at the given position ***/
void create_canvas_point(NR::Point const &pos, double size = 4.0, guint32 rgba = 0xff00007f);

/*** For testing purposes: Draw a line between the specified points ***/
void create_canvas_line(NR::Point const &p1, NR::Point const &p2, guint32 rgba = 0xff00007f);


/** A function to print out the Line.  It just prints out the coordinates of start end end point
    on the given output stream */
inline std::ostream &operator<< (std::ostream &out_file, const Line &in_line) {
    out_file << "Start: " << in_line.pt << "  Direction: " << in_line.v_dir;
    return out_file;
}

} // namespace Box3D


#endif /* !SEEN_LINE_GEOMETRY_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

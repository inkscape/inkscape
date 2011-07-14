#include "libnr/nr-point-fns.h"

Geom::Point
snap_vector_midpoint (Geom::Point const &p, Geom::Point const &begin, Geom::Point const &end, double snap)
{
    double length = Geom::distance(begin, end);
    Geom::Point be = (end - begin) / length;
    double r = Geom::dot(p - begin, be);

    if (r < 0.0) return begin;
    if (r > length) return end;

    double snapdist = length * snap;
    double r_snapped = (snap==0) ? r : floor(r/(snapdist + 0.5)) * snapdist;

    return (begin + r_snapped * be);
}

double
get_offset_between_points (Geom::Point const &p, Geom::Point const &begin, Geom::Point const &end)
{
    double length = Geom::distance(begin, end);
    Geom::Point be = (end - begin) / length;
    double r = Geom::dot(p - begin, be);

    if (r < 0.0) return 0.0;
    if (r > length) return 1.0;

    return (r / length);
}

Geom::Point
project_on_linesegment(Geom::Point const &p, Geom::Point const &p1, Geom::Point const &p2) 
{
    // p_proj = projection of p on the linesegment running from p1 to p2
    // p_proj = p1 + u (p2 - p1)
    // calculate u according to "Minimum Distance between a Point and a Line"
    // see http://local.wasp.uwa.edu.au/~pbourke/geometry/pointline/        
    
    // Warning: projected points will not necessarily be in between the endpoints of the linesegments!  
    
    if (p1 == p2) { // to avoid div. by zero below
        return p;
    }
    
    Geom::Point d1(p-p1); // delta 1
    Geom::Point d2(p2-p1); // delta 2
    double u = Geom::dot(d1, d2) / Geom::L2sq(d2);
    
    return (p1 + u*(p2-p1));
}

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

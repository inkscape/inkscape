/*
 * Class modelling a 3D perspective
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_PERSPECTIVE3D_H
#define SEEN_PERSPECTIVE3D_H

#include "vanishing-point.h"
#include "svg/stringstream.h"

class SP3DBox;

namespace Box3D {

class PerspectiveLine;

class Perspective3D {
public:
    Perspective3D(VanishingPoint const &pt_x, VanishingPoint const &pt_y, VanishingPoint const &pt_z);
    Perspective3D(Perspective3D &other);
    ~Perspective3D();

    bool operator== (Perspective3D const &other);

    VanishingPoint *get_vanishing_point (Box3D::Axis const dir);
    Axis get_axis_of_VP (VanishingPoint *vp);
    void set_vanishing_point (Box3D::Axis const dir, VanishingPoint const &pt);
    void set_vanishing_point (Box3D::Axis const dir, gdouble pt_x, gdouble pt_y, gdouble dir_x, gdouble dir_y, VPState st);
    void add_box (SP3DBox *box);
    void remove_box (const SP3DBox *box);
    bool has_box (const SP3DBox *box);
    void reshape_boxes (Box3D::Axis axes);
    void update_box_reprs ();

    static gint counter; // for testing only
    gint my_counter; // for testing only

    static GSList * perspectives; // All existing 3D perspectives
    static void add_perspective (Box3D::Perspective3D * const persp);
    static void remove_perspective (Box3D::Perspective3D * const persp);
    static Box3D::Perspective3D * find_perspective (Box3D::Perspective3D * const persp); // find an existing perspective whose VPs are equal to those of persp

    static void print_debugging_info();
    static Perspective3D * current_perspective;

private:
    VanishingPoint *vp_x;
    VanishingPoint *vp_y;
    VanishingPoint *vp_z;
    GSList * boxes; // holds a list of boxes sharing this specific perspective
};

Perspective3D * get_persp_of_box (const SP3DBox *box);
Perspective3D * get_persp_of_VP (const VanishingPoint *vp);

NR::Point perspective_intersection (NR::Point pt1, Box3D::Axis dir1, NR::Point pt2, Box3D::Axis dir2, Perspective3D *persp);
NR::Point perspective_line_snap (NR::Point pt, Box3D::Axis dir, NR::Point ext_pt, Perspective3D *persp);

} // namespace Box3D


/** A function to print out the VanishingPoint (prints the coordinates) **/
/***
inline std::ostream &operator<< (std::ostream &out_file, const VanishingPoint &vp) {
    out_file << vp;
    return out_file;
}
***/


#endif /* !SEEN_PERSPECTIVE3D_H */

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

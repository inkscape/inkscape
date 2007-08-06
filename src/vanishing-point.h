/*
 * Vanishing point for 3D perspectives
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_VANISHING_POINT_H
#define SEEN_VANISHING_POINT_H

#include "libnr/nr-point.h"
#include "knot.h"
#include "selection.h"
#include "axis-manip.h"

#include "line-geometry.h" // TODO: Remove this include as soon as we don't need create_canvas_(point|line) any more.

class SP3DBox;

namespace Box3D {

enum VPState {
    VP_FINITE = 0, // perspective lines meet in the VP
    VP_INFINITE    // perspective lines are parallel
};

// FIXME: Store the Axis of the VP inside the class
class VanishingPoint : public NR::Point {
public:
    inline VanishingPoint() : NR::Point() {};
    /***
    inline VanishingPoint(NR::Point const &pt, NR::Point const &ref = NR::Point(0,0))
                         : NR::Point (pt),
                           ref_pt (ref),
                           v_dir (pt[NR::X] - ref[NR::X], pt[NR::Y] - ref[NR::Y]) {}
    inline VanishingPoint(NR::Coord x, NR::Coord y, NR::Point const &ref = NR::Point(0,0))
                         : NR::Point (x, y),
                           ref_pt (ref),
                           v_dir (x - ref[NR::X], y - ref[NR::Y]) {}
    ***/
    VanishingPoint(NR::Point const &pt, NR::Point const &inf_dir, VPState st);
    VanishingPoint(NR::Point const &pt);
    VanishingPoint(NR::Point const &dir, VPState const state);
    VanishingPoint(NR::Point const &pt, NR::Point const &direction);
    VanishingPoint(NR::Coord x, NR::Coord y);
    VanishingPoint(NR::Coord x, NR::Coord y, VPState const state);
    VanishingPoint(NR::Coord x, NR::Coord y, NR::Coord dir_x, NR::Coord dir_y);
    VanishingPoint(VanishingPoint const &rhs);
    ~VanishingPoint();

    bool operator== (VanishingPoint const &other);

    inline NR::Point get_pos() const { return NR::Point ((*this)[NR::X], (*this)[NR::Y]); }
    inline void set_pos(NR::Point const &pt) { (*this)[NR::X] = pt[NR::X];
                                               (*this)[NR::Y] = pt[NR::Y]; }
    inline void set_pos(const double pt_x, const double pt_y) { (*this)[NR::X] = pt_x;
                                                                (*this)[NR::Y] = pt_y; }

    bool is_finite() const;
    VPState toggle_parallel();
    void draw(Box3D::Axis const axis); // Draws a point on the canvas if state == VP_FINITE
    //inline VPState state() { return state; }
	
    VPState state;
    //NR::Point ref_pt; // point of reference to compute the direction of parallel lines
    NR::Point v_dir; // direction of perslective lines if the VP has state == VP_INFINITE

private:
};

class Perspective3D;
class VPDrag;

struct VPDragger {
public:
    VPDragger(VPDrag *parent, NR::Point p, VanishingPoint *vp);
    ~VPDragger();

    VPDrag *parent;
    SPKnot *knot;

    // position of the knot, desktop coords
    NR::Point point;
    // position of the knot before it began to drag; updated when released
    NR::Point point_original;

    GSList *vps; // the list of vanishing points

    void addVP(VanishingPoint *vp);
    void removeVP(VanishingPoint *vp);
    /* returns the VP of the dragger that belongs to the given perspective */
    VanishingPoint *getVPofPerspective (Perspective3D *persp);

    bool hasBox (const SP3DBox *box);
    guint numberOfBoxes(); // the number of boxes linked to all VPs of the dragger

    bool hasPerspective (const Perspective3D *perps);
    void mergePerspectives (); // remove duplicate perspectives

    void reshapeBoxes(NR::Point const &p, Box3D::Axis axes);
    void updateBoxReprs();
};

struct VPDrag {
public:
    VPDrag(SPDesktop *desktop);
    ~VPDrag();

    VPDragger *getDraggerFor (VanishingPoint const &vp);

    //void grabKnot (VanishingPoint const &vp, gint x, gint y, guint32 etime);

    bool local_change;

    SPDesktop *desktop;
    GList *draggers;
    GSList *lines;

    void updateDraggers ();
    void updateLines ();
    void updateBoxHandles ();
    void drawLinesForFace (const SP3DBox *box, Box3D::Axis axis); //, guint corner1, guint corner2, guint corner3, guint corner4);
    bool show_lines; /* whether perspective lines are drawn at all */
    guint front_or_rear_lines; /* whether we draw perspective lines from all corners or only the
                                  front/rear corners (indicated by the first/second bit, respectively  */


    inline bool hasEmptySelection() { return this->selection->isEmpty(); }
    bool allBoxesAreSelected (VPDragger *dragger);
    GSList * selectedBoxesWithVPinDragger (VPDragger *dragger);

    // FIXME: Should this be private? (It's the case with the corresponding function in gradient-drag.h)
    //        But vp_knot_grabbed_handler
    void addDragger (VanishingPoint *vp);

private:
    //void deselect_all();

    void addLine (NR::Point p1, NR::Point p2, guint32 rgba);

    Inkscape::Selection *selection;
    sigc::connection sel_changed_connection;
    sigc::connection sel_modified_connection;
};

} // namespace Box3D


/** A function to print out the VanishingPoint (prints the coordinates) **/
/***
inline std::ostream &operator<< (std::ostream &out_file, const VanishingPoint &vp) {
    out_file << vp;
    return out_file;
}
***/


#endif /* !SEEN_VANISHING_POINT_H */

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

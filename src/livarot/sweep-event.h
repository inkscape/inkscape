#ifndef INKSCAPE_LIVAROT_SWEEP_EVENT_H
#define INKSCAPE_LIVAROT_SWEEP_EVENT_H
/** \file 
 * Intersection events.
 */

#include <2geom/point.h>
class SweepTree;


/** One intersection event. */
class SweepEvent
{
public:
    SweepTree *sweep[2];   ///< Sweep element associated with the left and right edge of the intersection.

    Geom::Point posx;         ///< Coordinates of the intersection.
    double tl, tr;          ///< Coordinates of the intersection on the left edge (tl) and on the right edge (tr).

    int ind;                ///< Index in the binary heap.

    SweepEvent();   // not used.
    virtual ~SweepEvent();  // not used.

    /// Initialize a SweepEvent structure.
    void MakeNew (SweepTree * iLeft, SweepTree * iRight, Geom::Point const &iPt,
                  double itl, double itr);

    /// Void a SweepEvent structure.
    void MakeDelete (void);
};


#endif /* !INKSCAPE_LIVAROT_SWEEP_EVENT_H */

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

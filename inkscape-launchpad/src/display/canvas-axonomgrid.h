#ifndef CANVAS_AXONOMGRID_H
#define CANVAS_AXONOMGRID_H

/*
 * Authors:
 *    Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright (C) 2006-2012 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "line-snapper.h"
#include "canvas-grid.h"

struct SPCanvasBuf;
class  SPDesktop;
class SPNamedView;

namespace Inkscape {
namespace XML {
    class Node;
};

class CanvasAxonomGrid : public CanvasGrid {
public:
    CanvasAxonomGrid(SPNamedView * nv, Inkscape::XML::Node * in_repr, SPDocument * in_doc);
    virtual ~CanvasAxonomGrid();

    virtual void Update (Geom::Affine const &affine, unsigned int flags);
    virtual void Render (SPCanvasBuf *buf);

    virtual void readRepr();
    virtual void onReprAttrChanged (Inkscape::XML::Node * repr, char const *key, char const *oldval, char const *newval, bool is_interactive);

    double lengthy;       /**< The lengths of the primary y-axis */
    double angle_deg[3];  /**< Angle of each axis (note that angle[2] == 0) */
    double angle_rad[3];  /**< Angle of each axis (note that angle[2] == 0) */
    double tan_angle[3];  /**< tan(angle[.]) */

    bool scaled;          /**< Whether the grid is in scaled mode */

protected:
    friend class CanvasAxonomGridSnapper;

    Geom::Point ow;         /**< Transformed origin by the affine for the zoom */
    double lyw;           /**< Transformed length y by the affine for the zoom */
    double lxw_x;
    double lxw_z;
    double spacing_ylines;

    Geom::Point sw;          /**< the scaling factors of the affine transform */

    virtual Gtk::Widget * newSpecificWidget();

private:
    CanvasAxonomGrid(const CanvasAxonomGrid&);
    CanvasAxonomGrid& operator=(const CanvasAxonomGrid&);

    void updateWidgets();
};



class CanvasAxonomGridSnapper : public LineSnapper
{
public:
    CanvasAxonomGridSnapper(CanvasAxonomGrid *grid, SnapManager *sm, Geom::Coord const d);
    bool ThisSnapperMightSnap() const;

    Geom::Coord getSnapperTolerance() const; //returns the tolerance of the snapper in screen pixels (i.e. independent of zoom)
    bool getSnapperAlwaysSnap() const; //if true, then the snapper will always snap, regardless of its tolerance

private:
    LineList _getSnapLines(Geom::Point const &p) const;
    void _addSnappedLine(IntermSnapResults &isr, Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, Geom::Point const &normal_to_line, const Geom::Point &point_on_line) const;
    void _addSnappedPoint(IntermSnapResults &isr, Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const;
    void _addSnappedLinePerpendicularly(IntermSnapResults &isr, Geom::Point const &snapped_point, Geom::Coord const &snapped_distance, SnapSourceType const &source, long source_num, bool constrained_snap) const;

    CanvasAxonomGrid *grid;
};


}; //namespace Inkscape



#endif



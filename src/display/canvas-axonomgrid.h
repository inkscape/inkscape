#ifndef CANVAS_AXONOMGRID_H
#define CANVAS_AXONOMGRID_H

/*
 * Copyright (C) 2006-2007 Johan Engelen  <johan@shouraizou.nl>
 *
  */

#include <display/sp-canvas.h>
#include <libnr/nr-coord.h>
#include "xml/repr.h"
#include <gtkmm/box.h>

#include <gtkmm.h>
#include "ui/widget/color-picker.h"
#include "ui/widget/scalar-unit.h"

#include "ui/widget/registered-widget.h"
#include "ui/widget/registry.h"

#include "xml/node-event-vector.h"

#include "snapper.h"
#include "line-snapper.h"

#include "canvas-grid.h"

struct SPDesktop;
struct SPNamedView;

namespace Inkscape {

class CanvasAxonomGrid : public CanvasGrid {
public:
    CanvasAxonomGrid(SPNamedView * nv, Inkscape::XML::Node * in_repr, SPDocument * in_doc);
    virtual ~CanvasAxonomGrid();

    void Update (NR::Matrix const &affine, unsigned int flags);
    void Render (SPCanvasBuf *buf);
    
    void readRepr();
    void onReprAttrChanged (Inkscape::XML::Node * repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive);

    double lengthy;       /**< The lengths of the primary y-axis */
    double angle_deg[3];  /**< Angle of each axis (note that angle[2] == 0) */
    double angle_rad[3];  /**< Angle of each axis (note that angle[2] == 0) */
    double tan_angle[3];  /**< tan(angle[.]) */
    
    bool scaled;          /**< Whether the grid is in scaled mode */
    
    NR::Point ow;         /**< Transformed origin by the affine for the zoom */
    double lyw;           /**< Transformed length y by the affine for the zoom */
    double lxw_x;
    double lxw_z;
    double spacing_ylines;
                          
    NR::Point sw;          /**< the scaling factors of the affine transform */

protected:
    virtual Gtk::Widget * newSpecificWidget();

private:
    CanvasAxonomGrid(const CanvasAxonomGrid&);
    CanvasAxonomGrid& operator=(const CanvasAxonomGrid&);
    
    void updateWidgets();
};



class CanvasAxonomGridSnapper : public LineSnapper
{
public:
    CanvasAxonomGridSnapper(CanvasAxonomGrid *grid, SnapManager const *sm, Geom::Coord const d);

private:    
    LineList _getSnapLines(Geom::Point const &p) const;
    void _addSnappedLine(SnappedConstraints &sc, Geom::Point const snapped_point, Geom::Coord const snapped_distance, Geom::Point const normal_to_line, const Geom::Point point_on_line) const;

    CanvasAxonomGrid *grid;
}; 


}; //namespace Inkscape



#endif    



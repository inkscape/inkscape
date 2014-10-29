#ifndef SEEN_NR_FILTER_UNITS_H
#define SEEN_NR_FILTER_UNITS_H

/*
 * Utilities for handling coordinate system transformations in filters
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2007 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-filter-units.h"
#include <2geom/affine.h>
#include <2geom/rect.h>

namespace Inkscape {
namespace Filters {

/* Notes:
 * - "filter units" is a coordinate system where the filter region is contained
 *   between (0,0) and (1,1). Do not confuse this with the filterUnits property
 * - "primitive units" is the coordinate system in which all lengths and distances
 *   in the filter definition should be interpreted. They are affected by the value
 *   of the primitiveUnits attribute
 * - "pb" is the coordinate system in which filter rendering happens.
 *   It might be aligned with user or screen coordinates depending on
 *   the filter primitives used in the filter.
 * - "display" are world coordinates of the canvas - pixel grid coordinates
 *   of the drawing area translated so that (0,0) corresponds to the document origin
 */
class FilterUnits {
public:
    FilterUnits();
    FilterUnits(SPFilterUnits const filterUnits, SPFilterUnits const primitiveUnits);

    /**
     * Sets the current transformation matrix, i.e. transformation matrix
     * from object's user coordinates to screen coordinates
     */
    void set_ctm(Geom::Affine const &ctm);

    /**
     * Sets the resolution, the filter should be rendered with.
     */
    void set_resolution(double const x_res, double const y_res);

    /**
     * Sets the item bounding box in user coordinates
     */
    void set_item_bbox(Geom::OptRect const &bbox);

    /**
     * Sets the filter effects area in user coordinates
     */
    void set_filter_area(Geom::OptRect const &area);

    /**
     * Sets, if x and y axis in pixblock coordinates should be paraller
     * to x and y of user coordinates.
     */
    void set_paraller(bool const paraller);

    /**
     * Sets, if filter resolution is automatic.
     * NOTE: even if resolution is automatic, it must be set with
     * set_resolution. This only tells, if the set value is automatic.
     */
    void set_automatic_resolution(bool const automatic);

    /**
     * Gets the item bounding box in user coordinates
     */
    Geom::OptRect get_item_bbox() const { return item_bbox; };

    /**
     * Gets the filter effects area in user coordinates
     */
    Geom::OptRect get_filter_area() const { return filter_area; };

    /**
     * Gets Filter Units (userSpaceOnUse or objectBoundingBox)
     */
    SPFilterUnits get_filter_units() const { return filterUnits; };

    /**
     * Gets Primitive Units (userSpaceOnUse or objectBoundingBox)
     */
    SPFilterUnits get_primitive_units() const { return primitiveUnits; };

    /**
     * Gets the user coordinates to pixblock coordinates transformation matrix.
     */
    Geom::Affine get_matrix_user2pb() const;

    /**
     * Gets the filterUnits to pixblock coordinates transformation matrix.
     */
    Geom::Affine get_matrix_filterunits2pb() const;

    /**
     * Gets the primitiveUnits to pixblock coordinates transformation matrix.
     */
    Geom::Affine get_matrix_primitiveunits2pb() const;

    /**
     * Gets the display coordinates to pixblock coordinates transformation
     * matrix.
     */
    Geom::Affine get_matrix_display2pb() const;

    /**
     * Gets the pixblock coordinates to display coordinates transformation
     * matrix
     */
    Geom::Affine get_matrix_pb2display() const;

    /**
     * Gets the user coordinates to filterUnits transformation matrix.
     */
    Geom::Affine get_matrix_user2filterunits() const;

    /**
     * Gets the user coordinates to primitiveUnits transformation matrix.
     */
    Geom::Affine get_matrix_user2primitiveunits() const;

    /**
     * Returns the filter area in pixblock coordinates.
     * NOTE: use only in filters, that define TRAIT_PARALLER in
     * get_input_traits. The filter effects area may not be representable
     * by simple rectangle otherwise. */
    Geom::IntRect get_pixblock_filterarea_paraller() const;

    FilterUnits& operator=(FilterUnits const &other);

private:
    Geom::Affine get_matrix_units2pb(SPFilterUnits units) const;
    Geom::Affine get_matrix_user2units(SPFilterUnits units) const;

    SPFilterUnits filterUnits, primitiveUnits;
    double resolution_x, resolution_y;
    bool paraller_axis;
    bool automatic_resolution;
    Geom::Affine ctm;
    Geom::OptRect item_bbox;
    Geom::OptRect filter_area;

};


} /* namespace Filters */
} /* namespace Inkscape */


#endif /* __NR_FILTER_UNITS_H__ */
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

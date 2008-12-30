#ifndef __NR_FILTER_UNITS_H__
#define __NR_FILTER_UNITS_H__

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
#include "libnr/nr-matrix.h"
#include <2geom/matrix.h>
#include "libnr/nr-rect.h"
#include "libnr/nr-rect-l.h"
#include <2geom/rect.h>

namespace NR {

class FilterUnits {
public:
    FilterUnits();
    FilterUnits(SPFilterUnits const filterUnits, SPFilterUnits const primitiveUnits);

    /**
     * Sets the current transformation matrix, i.e. transformation matrix
     * from object's user coordinates to screen coordinates
     */
    void set_ctm(Matrix const &ctm);

    /**
     * Sets the resolution, the filter should be rendered with.
     */
    void set_resolution(double const x_res, double const y_res);

    /**
     * Sets the item bounding box in user coordinates
     */
    void set_item_bbox(Rect const &bbox);

    /**
     * Sets the filter effects area in user coordinates
     */
    void set_filter_area(Rect const &area);

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
     * Gets the user coordinates to pixblock coordinates transformation matrix.
     */
    Matrix get_matrix_user2pb() const;

    /**
     * Gets the filterUnits to pixblock coordinates transformation matrix.
     */
    Matrix get_matrix_filterunits2pb() const;

    /**
     * Gets the primitiveUnits to pixblock coordinates transformation matrix.
     */
    Matrix get_matrix_primitiveunits2pb() const;

    /**
     * Gets the display coordinates to pixblock coordinates transformation
     * matrix.
     */
    Matrix get_matrix_display2pb() const;

    /**
     * Gets the pixblock coordinates to display coordinates transformation
     * matrix
     */
    Matrix get_matrix_pb2display() const;

    /**
     * Gets the user coordinates to filterUnits transformation matrix.
     */
    Matrix get_matrix_user2filterunits() const;

    /**
     * Gets the user coordinates to primitiveUnits transformation matrix.
     */
    Matrix get_matrix_user2primitiveunits() const;

    /**
     * Returns the filter area in pixblock coordinates.
     * NOTE: use only in filters, that define TRAIT_PARALLER in
     * get_input_traits. The filter effects area may not be representable
     * by simple rectangle otherwise. */
    IRect get_pixblock_filterarea_paraller() const;

    FilterUnits& operator=(FilterUnits const &other);

private:
    Matrix get_matrix_units2pb(SPFilterUnits units) const;
    Matrix get_matrix_user2units(SPFilterUnits units) const;

    SPFilterUnits filterUnits, primitiveUnits;
    double resolution_x, resolution_y;
    bool paraller_axis;
    bool automatic_resolution;
    Matrix ctm;
    Rect item_bbox;
    Rect filter_area;

};


} // namespace NR


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :

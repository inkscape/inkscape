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

#include <glib.h>

#include "display/nr-filter-units.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-rect.h"
#include "libnr/nr-scale.h"
#include "sp-filter-units.h"

namespace NR {

FilterUnits::FilterUnits() :
    filterUnits(SP_FILTER_UNITS_OBJECTBOUNDINGBOX),
    primitiveUnits(SP_FILTER_UNITS_USERSPACEONUSE),
    resolution_x(-1), resolution_y(-1),
    paraller_axis(false), automatic_resolution(true)
{}

FilterUnits::FilterUnits(SPFilterUnits const filterUnits, SPFilterUnits const primitiveUnits) :
    filterUnits(filterUnits), primitiveUnits(primitiveUnits),
    resolution_x(-1), resolution_y(-1),
    paraller_axis(false), automatic_resolution(true)
{}

void FilterUnits::set_ctm(Matrix const &ctm) {
    this->ctm = ctm;
}

void FilterUnits::set_resolution(double const x_res, double const y_res) {
    g_assert(x_res > 0);
    g_assert(y_res > 0);

    resolution_x = x_res;
    resolution_y = y_res;
}

void FilterUnits::set_item_bbox(Rect const &bbox) {
    item_bbox = bbox;
}

void FilterUnits::set_filter_area(Rect const &area) {
    filter_area = area;
}

void FilterUnits::set_paraller(bool const paraller) {
    paraller_axis = paraller;
}

void FilterUnits::set_automatic_resolution(bool const automatic) {
    automatic_resolution = automatic;
}

Matrix FilterUnits::get_matrix_user2pb() const {
    g_assert(resolution_x > 0);
    g_assert(resolution_y > 0);

    Matrix u2pb = ctm;

    if (paraller_axis || !automatic_resolution) {
        u2pb[0] = resolution_x / (filter_area.max()[X] - filter_area.min()[X]);
        u2pb[1] = 0;
        u2pb[2] = 0;
        u2pb[3] = resolution_y / (filter_area.max()[Y] - filter_area.min()[Y]);
        u2pb[4] = 0;
        u2pb[5] = 0;
    }

    return u2pb;
}

Matrix FilterUnits::get_matrix_units2pb(SPFilterUnits units) const {
    if (units == SP_FILTER_UNITS_OBJECTBOUNDINGBOX) {
        Matrix u2pb = get_matrix_user2pb();
        Point origo(item_bbox.min());
        origo *= u2pb;
        Point i_end(item_bbox.max()[X], item_bbox.min()[Y]);
        i_end *= u2pb;
        Point j_end(item_bbox.min()[X], item_bbox.max()[Y]);
        j_end *= u2pb;

        double len_i = sqrt((origo[X] - i_end[X]) * (origo[X] - i_end[X])
                            + (origo[Y] - i_end[Y]) * (origo[Y] - i_end[Y]));
        double len_j = sqrt((origo[X] - j_end[X]) * (origo[X] - j_end[X])
                            + (origo[Y] - j_end[Y]) * (origo[Y] - j_end[Y]));

        /* TODO: make sure that user coordinate system (0,0) is in correct
         * place in pixblock coordinates */
        scale scaling(1.0 / len_i, 1.0 / len_j);
        u2pb *= scaling;
        return u2pb;
    } else if (units == SP_FILTER_UNITS_USERSPACEONUSE) {
        return get_matrix_user2pb();
    } else {
        g_warning("Error in NR::FilterUnits::get_matrix_units2pb: unrecognized value of filterUnits");
        return Matrix();
    }
}

Matrix FilterUnits::get_matrix_filterunits2pb() const {
    return get_matrix_units2pb(filterUnits);
}

Matrix FilterUnits::get_matrix_primitiveunits2pb() const {
    return get_matrix_units2pb(primitiveUnits);
}

Matrix FilterUnits::get_matrix_display2pb() const {
    Matrix d2pb = ctm.inverse();
    d2pb *= get_matrix_user2pb();
    return d2pb;
}

Matrix FilterUnits::get_matrix_pb2display() const {
    Matrix pb2d = get_matrix_user2pb().inverse();
    pb2d *= ctm;
    return pb2d;
}

FilterUnits& FilterUnits::operator=(FilterUnits const &other) {
    filterUnits = other.filterUnits;
    primitiveUnits = other.primitiveUnits;
    resolution_x = other.resolution_x;
    resolution_y = other.resolution_y;
    paraller_axis = other.paraller_axis;
    automatic_resolution = other.automatic_resolution;
    ctm = other.ctm;
    item_bbox = other.item_bbox;
    filter_area = other.filter_area;
    return *this;
}

} // namespace NR


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

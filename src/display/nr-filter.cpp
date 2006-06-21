#define __NR_FILTER_CPP__

/*
 * SVG filters rendering
 *
 * Author:
 *   Niko Kiirala <niko@kiirala.com>
 *
 * Copyright (C) 2006 Niko Kiirala
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-gaussian.h"

#include "display/nr-arena-item.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-blit.h"
#include "svg/svg-length.h"
#include "sp-filter-units.h"

//#include "display/nr-arena-shape.h"

namespace NR {

Filter::Filter()
{
    _primitive_count = 1;
    _primitive_table_size = 1;
    _primitive = new FilterPrimitive*[1];
    _primitive[0] = new FilterGaussian;
    _common_init();
}

Filter::Filter(int n)
{
    _primitive_count = 0;
    _primitive_table_size = n;
    _primitive = new FilterPrimitive*[n];
    for ( int i = 0 ; i < n ; i++ ) {
        _primitive[i] = NULL;
    }
    _common_init();
}

void Filter::_common_init() {
    _slot_count = 1;
    _output_slot = -1;

    _region_x.set(SVGLength::PERCENT, -10, 0);
    _region_y.set(SVGLength::PERCENT, -10, 0);
    _region_width.set(SVGLength::PERCENT, 120, 0);
    _region_height.set(SVGLength::PERCENT, 120, 0);

    _x_pixels = -1.0;
    _y_pixels = -1.0;

    _filter_units = SP_FILTER_UNITS_OBJECTBOUNDINGBOX;
    _primitive_units = SP_FILTER_UNITS_USERSPACEONUSE;
}

Filter::~Filter()
{
    clear_primitives();
    delete[] _primitive;
}


int Filter::render(NRArenaItem const *item, NRPixBlock *pb)
{
    NRPixBlock *slot[2];
    slot[0] = pb;
    slot[1] = NULL;

    _primitive[0]->render(slot, *item->ctm);


    int size = (slot[0]->area.x1 - slot[0]->area.x0)
        * (slot[0]->area.y1 - slot[0]->area.y0)
        * NR_PIXBLOCK_BPP(slot[0]);
    memset(NR_PIXBLOCK_PX(slot[0]), 0, size);

    nr_blit_pixblock_pixblock(slot[0], slot[1]);

    slot[0]->visible_area = slot[0]->area;

    nr_pixblock_release(slot[1]);

    return 0;
}

int Filter::get_enlarge(Matrix const &m)
{
    int enlarge = 0;
    for ( int i = 0 ; i < _primitive_count ; i++ ) {
        if(_primitive[i]) enlarge += _primitive[i]->get_enlarge(m);
    }
    return enlarge;
}

void Filter::bbox_enlarge(NRRectL &bbox)
{
    int len_x = bbox.x1 - bbox.x0;
    int len_y = bbox.y1 - bbox.y0;
    int enlarge_x = (int)std::ceil(len_x / 10.0);
    int enlarge_y = (int)std::ceil(len_y / 10.0);
    bbox.x0 -= enlarge_x;
    bbox.x1 += enlarge_x;
    bbox.y0 -= enlarge_y;
    bbox.y1 += enlarge_y;
}

typedef FilterPrimitive*(*FilterConstructor)();
static FilterConstructor _constructor[NR_FILTER_ENDPRIMITIVETYPE];

void Filter::_create_constructor_table()
{
    static bool created = false;
    if(created) return;

    /* Filter effects not yet implemented are set to NULL */
    _constructor[NR_FILTER_BLEND] = NULL;
    _constructor[NR_FILTER_COLORMATRIX] = NULL;
    _constructor[NR_FILTER_COMPONENTTRANSFER] = NULL;
    _constructor[NR_FILTER_COMPOSITE] = NULL;
    _constructor[NR_FILTER_CONVOLVEMATRIX] = NULL;
    _constructor[NR_FILTER_DIFFUSELIGHTING] = NULL;
    _constructor[NR_FILTER_DISPLACEMENTMAP] = NULL;
    _constructor[NR_FILTER_FLOOD] = NULL;
    _constructor[NR_FILTER_GAUSSIANBLUR] = &FilterGaussian::create;
    _constructor[NR_FILTER_IMAGE] = NULL;
    _constructor[NR_FILTER_MERGE] = NULL;
    _constructor[NR_FILTER_MORPHOLOGY] = NULL;
    _constructor[NR_FILTER_OFFSET] = NULL;
    _constructor[NR_FILTER_SPECULARLIGHTING] = NULL;
    _constructor[NR_FILTER_TILE] = NULL;
    _constructor[NR_FILTER_TURBULENCE] = NULL;
}

void Filter::_enlarge_primitive_table() {
    FilterPrimitive **new_tbl = new FilterPrimitive*[_primitive_table_size * 2];
    for (int i = 0 ; i < _primitive_count ; i++) {
        new_tbl[i] = _primitive[i];
    }
    _primitive_table_size *= 2;
    for (int i = _primitive_count ; i < _primitive_table_size ; i++) {
        new_tbl[i] = NULL;
    }
    delete[] _primitive;
    _primitive = new_tbl;
}

FilterPrimitive *Filter::add_primitive(FilterPrimitiveType type)
{
    _create_constructor_table();

    // Check that we can create a new filter of specified type
    if (type < 0 || type >= NR_FILTER_ENDPRIMITIVETYPE)
        return NULL;
    if (!_constructor[type]) return NULL;
    FilterPrimitive *created = _constructor[type]();

    // If there is no space for new filter primitive, enlarge the table
    if (_primitive_count >= _primitive_table_size) {
        _enlarge_primitive_table();
    }

    _primitive[_primitive_count] = created;
    return created;
}

FilterPrimitive *Filter::replace_primitive(FilterPrimitive *target, FilterPrimitiveType type)
{
    _create_constructor_table();

    // Check that target is valid primitive inside this filter
    int place = -1;
    for (int i = 0 ; i < _primitive_count ; i++) {
        if (target == _primitive[i]) {
            place = i;
            break;
        }
    }
    if (place < 0) return NULL;

    // Check that we can create a new filter of specified type
    if (type < 0 || type >= NR_FILTER_ENDPRIMITIVETYPE)
        return NULL;
    if (!_constructor[type]) return NULL;
    FilterPrimitive *created = _constructor[type]();

    // If there is no space for new filter primitive, enlarge the table
    if (_primitive_count >= _primitive_table_size) {
        _enlarge_primitive_table();
    }

    delete target;
    _primitive[place] = created;
    return created;
}

void Filter::clear_primitives()
{
    for (int i = 0 ; i < _primitive_count ; i++) {
        if (_primitive[i]) delete _primitive[i];
    }
    _primitive_count = 0;
}

} /* namespace NR */

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

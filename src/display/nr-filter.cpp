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

#include <glib.h>
#include <cmath>

#include "display/nr-filter.h"
#include "display/nr-filter-primitive.h"
#include "display/nr-filter-slot.h"
#include "display/nr-filter-types.h"
#include "display/pixblock-scaler.h"
#include "display/pixblock-transform.h"
#include "display/nr-filter-gaussian.h"
#include "display/nr-filter-blend.h"

#include "display/nr-arena-item.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-blit.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-scale.h"
#include "svg/svg-length.h"
#include "sp-filter-units.h"
#if defined (SOLARIS_2_8)
#include "round.h"
using Inkscape::round;
#endif 

//#include "display/nr-arena-shape.h"

__attribute__ ((const))
inline static int _max4(const double a, const double b,
                        const double c, const double d) {
    double ret = a;
    if (b > ret) ret = b;
    if (c > ret) ret = c;
    if (d > ret) ret = d;
    return (int)round(ret);
}

__attribute__ ((const))
inline static int _min4(const double a, const double b,
                        const double c, const double d) {
    double ret = a;
    if (b < ret) ret = b;
    if (c < ret) ret = c;
    if (d < ret) ret = d;
    return (int)round(ret);
}

namespace NR {

Filter::Filter()
{
    _primitive_count = 0;
    _primitive_table_size = 1;
    _primitive = new FilterPrimitive*[1];
    _primitive[0] = NULL;
    //_primitive_count = 1;
    //_primitive[0] = new FilterGaussian;
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
    // Having "not set" here as value means the output of last filter
    // primitive will be used as output of this filter
    _output_slot = NR_FILTER_SLOT_NOT_SET;

    // These are the default values for filter region,
    // as specified in SVG standard
    // NB: SVGLength.set takes prescaled percent values: -.10 means -10%
    _region_x.set(SVGLength::PERCENT, -.10, 0);
    _region_y.set(SVGLength::PERCENT, -.10, 0);
    _region_width.set(SVGLength::PERCENT, 1.20, 0);
    _region_height.set(SVGLength::PERCENT, 1.20, 0);

    // Filter resolution, negative value here stands for "automatic"
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
    if(!_primitive[0]) { // if there are no primitives, do nothing
       return 0; 
    }

    Matrix trans = *item->ctm;
    Matrix paraller_trans = trans;
    bool notparaller = false;
    FilterSlot slot(_slot_count, item);
    NRPixBlock *in = new NRPixBlock;

    // If filter effects region is not paraller to viewport,
    // we must first undo the rotation / shear.
    // It will be redone after filtering.
    // If there is only rotation and uniform scaling (zoom), let's skip this,
    // as it will not make a difference with gaussian blur.
    if ((fabs(trans[1]) > 1e-6 || fabs(trans[2]) > 1e-6) &&
        !(fabs(trans[0] - trans[3]) < 1e-6 && fabs(trans[1] + trans[2]) < 1e-6)) {
        notparaller = true;

        // TODO: if filter resolution is specified, scaling should be set
        // according to that
        double scaling_factor = sqrt(trans.expansionX() * trans.expansionX() +
                                     trans.expansionY() * trans.expansionY());
        scale scaling(scaling_factor, scaling_factor);
        scale scaling_inv(1.0 / scaling_factor, 1.0 / scaling_factor);
        trans *= scaling_inv;
        paraller_trans.set_identity();
        paraller_trans *= scaling;

        Matrix itrans = trans.inverse();
        int x0 = pb->area.x0;
        int y0 = pb->area.y0;
        int x1 = pb->area.x1;
        int y1 = pb->area.y1;
        int min_x = _min4(itrans[0] * x0 + itrans[2] * y0 + itrans[4],
                          itrans[0] * x0 + itrans[2] * y1 + itrans[4],
                          itrans[0] * x1 + itrans[2] * y0 + itrans[4],
                          itrans[0] * x1 + itrans[2] * y1 + itrans[4]);
        int max_x = _max4(itrans[0] * x0 + itrans[2] * y0 + itrans[4],
                          itrans[0] * x0 + itrans[2] * y1 + itrans[4],
                          itrans[0] * x1 + itrans[2] * y0 + itrans[4],
                          itrans[0] * x1 + itrans[2] * y1 + itrans[4]);
        int min_y = _min4(itrans[1] * x0 + itrans[3] * y0 + itrans[5],
                          itrans[1] * x0 + itrans[3] * y1 + itrans[5],
                          itrans[1] * x1 + itrans[3] * y0 + itrans[5],
                          itrans[1] * x1 + itrans[3] * y1 + itrans[5]);
        int max_y = _max4(itrans[1] * x0 + itrans[3] * y0 + itrans[5],
                          itrans[1] * x0 + itrans[3] * y1 + itrans[5],
                          itrans[1] * x1 + itrans[3] * y0 + itrans[5],
                          itrans[1] * x1 + itrans[3] * y1 + itrans[5]);
        
        nr_pixblock_setup_fast(in, pb->mode,
                               min_x, min_y,
                               max_x, max_y, true);
        if (in->size != NR_PIXBLOCK_SIZE_TINY && in->data.px == NULL) // memory allocation failed
            return 0;
        transform_nearest(in, pb, itrans);
    } else if (_x_pixels >= 0) {
        // If filter resolution is not set to automatic, we should
        // scale the input image to correct resolution
        /* If filter resolution is zero, the object should not be rendered */
        if (_x_pixels == 0 || _y_pixels == 0) {
            int size = (pb->area.x1 - pb->area.x0)
                * (pb->area.y1 - pb->area.y0)
                * NR_PIXBLOCK_BPP(pb);
            memset(NR_PIXBLOCK_PX(pb), 0, size);
            return 0;
        }
        // Resolution is specified as pixel length of our internal buffer.
        // Though, we might not be rendering the whole object at time,
        // so we need to calculate the correct pixel size
        int x_len = (int)round(((pb->area.x1 - pb->area.x0) * _x_pixels) / (item->bbox.x1 - item->bbox.x0));
        if (x_len < 1) x_len = 1;
        // If y-resolution is also set, count y-area in the same way as x-area
        // Otherwise, make y-area so, that aspect ratio of input pixblock and
        // internal pixblock are the same.
        int y_len;
        if (_y_pixels > 0) {
            y_len = (int)round(((pb->area.y1 - pb->area.y0) * _y_pixels) / (item->bbox.y1 - item->bbox.y0));
        } else {
            y_len = (int)round((x_len * (pb->area.y1 - pb->area.y0)) / (double)(pb->area.x1 - pb->area.x0));
        }
        if (y_len < 1) y_len = 1;
        nr_pixblock_setup_fast(in, pb->mode, 0, 0, x_len, y_len, true);
        if (in->size != NR_PIXBLOCK_SIZE_TINY && in->data.px == NULL) // memory allocation failed
            return 0;
        scale_bicubic(in, pb);
        scale res_scaling(x_len / (double)(pb->area.x1 - pb->area.x0),
                          y_len / (double)(pb->area.y1 - pb->area.y0));
        paraller_trans *= res_scaling;
    } else {
        // If filter resolution is automatic, just make copy of input image
        nr_pixblock_setup_fast(in, pb->mode,
                               pb->area.x0, pb->area.y0,
                               pb->area.x1, pb->area.y1, true);
        if (in->size != NR_PIXBLOCK_SIZE_TINY && in->data.px == NULL) // memory allocation failed
            return 0;
        nr_blit_pixblock_pixblock(in, pb);
    }
    slot.set(NR_FILTER_SOURCEGRAPHIC, in);
    in = NULL; // in is now handled by FilterSlot, we should not touch it

    for (int i = 0 ; i < _primitive_count ; i++) {
        _primitive[i]->render(slot, paraller_trans);
    }
    NRPixBlock *out = slot.get(_output_slot);

    // Clear the pixblock, where the output will be put
    // -> the original image does not show through
    int size = (pb->area.x1 - pb->area.x0)
        * (pb->area.y1 - pb->area.y0)
        * NR_PIXBLOCK_BPP(pb);
    memset(NR_PIXBLOCK_PX(pb), 0, size);

    if (notparaller) {
        transform_nearest(pb, out, trans);
    } else if (_x_pixels < 0) {
        // If the filter resolution is automatic, just copy our final image
        // to output pixblock, otherwise use bicubic scaling
        nr_blit_pixblock_pixblock(pb, out);
    } else {
        scale_bicubic(pb, out);
    }

    // Take note of the amount of used image slots
    // -> next time this filter is rendered, we can reserve enough slots
    // immediately
    _slot_count = slot.get_slot_count();
    return 0;
}

int Filter::get_enlarge(Matrix const &m)
{
    // Just sum the enlargement factor of all filter elements.
    // TODO: this both sucks and blows for filters like feOffset
    // -> ditch this method and design a better one...
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
    /* TODO: fetch somehow the object ex and em lengths */
    _region_x.update(12, 6, len_x);
    _region_y.update(12, 6, len_y);
    _region_width.update(12, 6, len_x);
    _region_height.update(12, 6, len_y);
    if (_filter_units == SP_FILTER_UNITS_OBJECTBOUNDINGBOX) {
        if (_region_x.unit == SVGLength::PERCENT) {
            bbox.x0 += (ICoord)_region_x.computed;
        } else {
            bbox.x0 += (ICoord)(_region_x.computed * len_x);
        }
        if (_region_width.unit == SVGLength::PERCENT) {
            bbox.x1 = bbox.x0 + (ICoord)_region_width.computed;
        } else {
            bbox.x1 = bbox.x0 + (ICoord)(_region_width.computed * len_x);
        }

        if (_region_y.unit == SVGLength::PERCENT) {
            bbox.y0 += (ICoord)_region_y.computed;
        } else {
            bbox.y0 += (ICoord)(_region_y.computed * len_y);
        }
        if (_region_height.unit == SVGLength::PERCENT) {
            bbox.y1 = bbox.y0 + (ICoord)_region_height.computed;
        } else {
            bbox.y1 = bbox.y0 + (ICoord)(_region_height.computed * len_y);
        }
    } else if (_filter_units == SP_FILTER_UNITS_USERSPACEONUSE) {
        /* TODO: make sure bbox and fe region are in same coordinate system */
        bbox.x0 = (ICoord) _region_x.computed;
        bbox.x1 = bbox.x0 + (ICoord) _region_width.computed;
        bbox.y0 = (ICoord) _region_y.computed;
        bbox.y1 = bbox.y0 + (ICoord) _region_height.computed;
    } else {
        g_warning("Error in NR::Filter::bbox_enlarge: unrecognized value of _filter_units");
    }
}

/* Constructor table holds pointers to static methods returning filter
 * primitives. This table is indexed with FilterPrimitiveType, so that
 * for example method in _constructor[NR_FILTER_GAUSSIANBLUR]
 * returns a filter object of type NR::FilterGaussian.
 */
typedef FilterPrimitive*(*FilterConstructor)();
static FilterConstructor _constructor[NR_FILTER_ENDPRIMITIVETYPE];

void Filter::_create_constructor_table()
{
    // Constructor table won't change in run-time, so no need to recreate
    static bool created = false;
    if(created) return;

    /* Filter effects not yet implemented are set to NULL */
    _constructor[NR_FILTER_BLEND] = &FilterBlend::create;
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
    created = true;
}

/** Helper method for enlarging table of filter primitives. When new
 * primitives are added, but we have no space for them, this function
 * makes some more space.
 */
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

int Filter::add_primitive(FilterPrimitiveType type)
{
    _create_constructor_table();

    // Check that we can create a new filter of specified type
    if (type < 0 || type >= NR_FILTER_ENDPRIMITIVETYPE)
        return -1;
    if (!_constructor[type]) return -1;
    FilterPrimitive *created = _constructor[type]();

    // If there is no space for new filter primitive, enlarge the table
    if (_primitive_count >= _primitive_table_size) {
        _enlarge_primitive_table();
    }

    _primitive[_primitive_count] = created;
    int handle = _primitive_count;
    _primitive_count++;
    return handle;
}

int Filter::replace_primitive(int target, FilterPrimitiveType type)
{
    _create_constructor_table();

    // Check that target is valid primitive inside this filter
    if (target < 0) return -1;
    if (target >= _primitive_count) return -1;
    if (!_primitive[target]) return -1;

    // Check that we can create a new filter of specified type
    if (type < 0 || type >= NR_FILTER_ENDPRIMITIVETYPE)
        return -1;
    if (!_constructor[type]) return -1;
    FilterPrimitive *created = _constructor[type]();

    // If there is no space for new filter primitive, enlarge the table
    if (_primitive_count >= _primitive_table_size) {
        _enlarge_primitive_table();
    }

    delete _primitive[target];
    _primitive[target] = created;
    return target;
}

FilterPrimitive *Filter::get_primitive(int handle) {
    if (handle < 0 || handle >= _primitive_count) return NULL;
    return _primitive[handle];
}

void Filter::clear_primitives()
{
    for (int i = 0 ; i < _primitive_count ; i++) {
        if (_primitive[i]) delete _primitive[i];
    }
    _primitive_count = 0;
}

void Filter::set_x(SVGLength &length)
{ 
  if (length._set)
      _region_x = length;
}
void Filter::set_y(SVGLength &length)
{
  if (length._set)
      _region_y = length;
}
void Filter::set_width(SVGLength &length)
{
  if (length._set)
      _region_width = length;
}
void Filter::set_height(SVGLength &length)
{ 
  if (length._set)
      _region_height = length;
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

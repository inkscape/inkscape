/*
 * feComponentTransfer filter primitive renderer
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <felipe.sanches@gmail.com>
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-component-transfer.h"
#include "display/nr-filter-units.h"
#include "display/nr-filter-utils.h"
#include "libnr/nr-pixblock.h"
#include "libnr/nr-blit.h"
#include "libnr/nr-pixops.h"
#include <math.h>

namespace Inkscape {
namespace Filters {

FilterComponentTransfer::FilterComponentTransfer()
{
}

FilterPrimitive * FilterComponentTransfer::create() {
    return new FilterComponentTransfer();
}

FilterComponentTransfer::~FilterComponentTransfer()
{}

int FilterComponentTransfer::render(FilterSlot &slot, FilterUnits const &/*units*/) {
    NRPixBlock *in = slot.get(_input);

    if (!in) {
        g_warning("Missing source image for feComponentTransfer (in=%d)", _input);
        return 1;
    }

    int x0=in->area.x0;
    int x1=in->area.x1;
    int y0=in->area.y0;
    int y1=in->area.y1;

    NRPixBlock *out = new NRPixBlock;
    nr_pixblock_setup_fast(out, NR_PIXBLOCK_MODE_R8G8B8A8N, x0, y0, x1, y1, true);

    // this primitive is defined for non-premultiplied RGBA values,
    // thus convert them to that format before blending
    bool free_in_on_exit = false;
    if (in->mode != NR_PIXBLOCK_MODE_R8G8B8A8N) {
        NRPixBlock *original_in = in;
        in = new NRPixBlock;
        nr_pixblock_setup_fast(in, NR_PIXBLOCK_MODE_R8G8B8A8N,
                               original_in->area.x0, original_in->area.y0,
                               original_in->area.x1, original_in->area.y1,
                               false);
        nr_blit_pixblock_pixblock(in, original_in);
        free_in_on_exit = true;
    }

    unsigned char *in_data = NR_PIXBLOCK_PX(in);
    unsigned char *out_data = NR_PIXBLOCK_PX(out);

    (void)in_data;
    (void)out_data;

    int size = 4 * (y1-y0) * (x1-x0);
    int i;

    for (int color=0;color<4;color++){
        int _vsize = tableValues[color].size();
        double _intercept = intercept[color];
        double _slope = slope[color];
        double _amplitude = amplitude[color];
        double _exponent = exponent[color];
        double _offset = offset[color];
        switch(type[color]){
            case COMPONENTTRANSFER_TYPE_IDENTITY:
                for(i=color;i<size;i+=4){
                    out_data[i]=in_data[i];
                }
                break;
            case COMPONENTTRANSFER_TYPE_TABLE:
                if (_vsize<=1){
                    if (_vsize==1) {
                        g_warning("A component transfer table has to have at least two values.");
                    }
                    for(i=color;i<size;i+=4){
                        out_data[i]=in_data[i];
                    }
                } else {
                    std::vector<gdouble> _tableValues(tableValues[color]);
                    // Scale by 255 and add .5 to avoid having to add it later for rounding purposes
                    for(i=0;i<_vsize;i++) {
                        _tableValues[i] = std::max(0.,std::min(255.,255*_tableValues[i])) + .5;
                    }
                    for(i=color;i<size;i+=4){
                        int k = FAST_DIVIDE<255>((_vsize-1) * in_data[i]);
                        double dx = ((_vsize-1) * in_data[i])/255.0 - k;
                        out_data[i] = static_cast<unsigned char>(_tableValues[k] + dx * (_tableValues[k+1] - _tableValues[k]));
                    }
                }
                break;
            case COMPONENTTRANSFER_TYPE_DISCRETE:
                if (_vsize==0){
                    for(i=color;i<size;i+=4){
                        out_data[i] = in_data[i];
                    }
                } else {
                    std::vector<unsigned char> _tableValues(_vsize);
                    // Convert to unsigned char
                    for(i=0;i<_vsize;i++) {
                        _tableValues[i] = static_cast<unsigned char>(std::max(0.,std::min(255.,255*tableValues[color][i])) + .5);
                    }
                    for(i=color;i<size;i+=4){
                        int k = FAST_DIVIDE<255>((_vsize-1) * in_data[i]);
                        out_data[i] =  _tableValues[k];
                    }
                }
                break;
            case COMPONENTTRANSFER_TYPE_LINEAR:
                _intercept = 255*_intercept + .5;
                for(i=color;i<size;i+=4){
                    out_data[i] = CLAMP_D_TO_U8(_slope * in_data[i] + _intercept);
                }
                break;
            case COMPONENTTRANSFER_TYPE_GAMMA:
                _amplitude *= pow(255.0, -_exponent+1);
                _offset = 255*_offset + .5;
                for(i=color;i<size;i+=4){
                    out_data[i] = CLAMP_D_TO_U8(_amplitude * pow((double)in_data[i], _exponent) + _offset);
                }
                break;
            case COMPONENTTRANSFER_TYPE_ERROR:
                //TODO: report an error here
                g_warning("Component tranfer type \"error\".");
                break;
            default:
                g_warning("Invalid tranfer type %d.", type[color]);
        }
    }

    if (free_in_on_exit) {
        nr_pixblock_release(in);
        delete in;
    }

    out->empty = FALSE;
    slot.set(_output, out);
    return 0;
}

void FilterComponentTransfer::area_enlarge(NRRectL &/*area*/, Geom::Matrix const &/*trans*/)
{
}

} /* namespace Filters */
} /* namespace Inkscape */

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

#ifndef __SP_FILTER_ENUMS_H__
#define __SP_FILTER_ENUMS_H__

/*
 * Conversion data for filter and filter primitive enumerations
 *
 * Authors:
 *   Nicholas Bishop
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/nr-filter-blend.h"
#include "display/nr-filter-colormatrix.h"
#include "display/nr-filter-composite.h"
#include "display/nr-filter-convolve-matrix.h"
#include "display/nr-filter-types.h"
#include "util/enums.h"

// Filter primitives
extern const Inkscape::Util::EnumData<NR::FilterPrimitiveType> FPData[NR::NR_FILTER_ENDPRIMITIVETYPE];
extern const Inkscape::Util::EnumDataConverter<NR::FilterPrimitiveType> FPConverter;

enum FilterPrimitiveInput {
    FPINPUT_SOURCEGRAPHIC,
    FPINPUT_SOURCEALPHA,
    FPINPUT_BACKGROUNDIMAGE,
    FPINPUT_BACKGROUNDALPHA,
    FPINPUT_FILLPAINT,
    FPINPUT_STROKEPAINT,
    FPINPUT_END
};

extern const Inkscape::Util::EnumData<FilterPrimitiveInput> FPInputData[FPINPUT_END];
extern const Inkscape::Util::EnumDataConverter<FilterPrimitiveInput> FPInputConverter;

// Blend mode
extern const Inkscape::Util::EnumData<NR::FilterBlendMode> BlendModeData[NR::BLEND_ENDMODE];
extern const Inkscape::Util::EnumDataConverter<NR::FilterBlendMode> BlendModeConverter;
// ColorMatrix type
extern const Inkscape::Util::EnumData<NR::FilterColorMatrixType> ColorMatrixTypeData[NR::COLORMATRIX_ENDTYPE];
extern const Inkscape::Util::EnumDataConverter<NR::FilterColorMatrixType> ColorMatrixTypeConverter;
// Composite operator
extern const Inkscape::Util::EnumData<FeCompositeOperator> CompositeOperatorData[COMPOSITE_ENDOPERATOR];
extern const Inkscape::Util::EnumDataConverter<FeCompositeOperator> CompositeOperatorConverter;
// ConvolveMatrix edgeMode
extern const Inkscape::Util::EnumData<NR::FilterConvolveMatrixEdgeMode> ConvolveMatrixEdgeModeData[NR::CONVOLVEMATRIX_EDGEMODE_ENDTYPE];
extern const Inkscape::Util::EnumDataConverter<NR::FilterConvolveMatrixEdgeMode> ConvolveMatrixEdgeModeConverter;
// DisplacementMap channel
extern const Inkscape::Util::EnumData<int> DisplacementMapChannelData[4];
extern const Inkscape::Util::EnumDataConverter<int> DisplacementMapChannelConverter;
// Lighting
enum LightSource {
    LIGHT_DISTANT,
    LIGHT_POINT,
    LIGHT_SPOT,
    LIGHT_ENDSOURCE
};
extern const Inkscape::Util::EnumData<LightSource> LightSourceData[LIGHT_ENDSOURCE];
extern const Inkscape::Util::EnumDataConverter<LightSource> LightSourceConverter;

#endif

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

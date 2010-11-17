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
#include "display/nr-filter-component-transfer.h"
#include "display/nr-filter-composite.h"
#include "display/nr-filter-convolve-matrix.h"
#include "display/nr-filter-morphology.h"
#include "display/nr-filter-turbulence.h"
#include "display/nr-filter-types.h"
#include "filters/displacementmap.h"
#include "util/enums.h"

// Filter primitives
extern const Inkscape::Util::EnumData<Inkscape::Filters::FilterPrimitiveType> FPData[Inkscape::Filters::NR_FILTER_ENDPRIMITIVETYPE];
extern const Inkscape::Util::EnumDataConverter<Inkscape::Filters::FilterPrimitiveType> FPConverter;

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
extern const Inkscape::Util::EnumData<Inkscape::Filters::FilterBlendMode> BlendModeData[Inkscape::Filters::BLEND_ENDMODE];
extern const Inkscape::Util::EnumDataConverter<Inkscape::Filters::FilterBlendMode> BlendModeConverter;
// ColorMatrix type
extern const Inkscape::Util::EnumData<Inkscape::Filters::FilterColorMatrixType> ColorMatrixTypeData[Inkscape::Filters::COLORMATRIX_ENDTYPE];
extern const Inkscape::Util::EnumDataConverter<Inkscape::Filters::FilterColorMatrixType> ColorMatrixTypeConverter;
// ComponentTransfer type
extern const Inkscape::Util::EnumData<Inkscape::Filters::FilterComponentTransferType> ComponentTransferTypeData[Inkscape::Filters::COMPONENTTRANSFER_TYPE_ERROR];
extern const Inkscape::Util::EnumDataConverter<Inkscape::Filters::FilterComponentTransferType> ComponentTransferTypeConverter;
// Composite operator
extern const Inkscape::Util::EnumData<FeCompositeOperator> CompositeOperatorData[COMPOSITE_ENDOPERATOR];
extern const Inkscape::Util::EnumDataConverter<FeCompositeOperator> CompositeOperatorConverter;
// ConvolveMatrix edgeMode
extern const Inkscape::Util::EnumData<Inkscape::Filters::FilterConvolveMatrixEdgeMode> ConvolveMatrixEdgeModeData[Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_ENDTYPE];
extern const Inkscape::Util::EnumDataConverter<Inkscape::Filters::FilterConvolveMatrixEdgeMode> ConvolveMatrixEdgeModeConverter;
// DisplacementMap channel
extern const Inkscape::Util::EnumData<FilterDisplacementMapChannelSelector> DisplacementMapChannelData[4];
extern const Inkscape::Util::EnumDataConverter<FilterDisplacementMapChannelSelector> DisplacementMapChannelConverter;
// Morphology operator
extern const Inkscape::Util::EnumData<Inkscape::Filters::FilterMorphologyOperator> MorphologyOperatorData[Inkscape::Filters::MORPHOLOGY_OPERATOR_END];
extern const Inkscape::Util::EnumDataConverter<Inkscape::Filters::FilterMorphologyOperator> MorphologyOperatorConverter;
// Turbulence type
extern const Inkscape::Util::EnumData<Inkscape::Filters::FilterTurbulenceType> TurbulenceTypeData[Inkscape::Filters::TURBULENCE_ENDTYPE];
extern const Inkscape::Util::EnumDataConverter<Inkscape::Filters::FilterTurbulenceType> TurbulenceTypeConverter;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

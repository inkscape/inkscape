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

#include <glibmm/i18n.h>

#include "filter-enums.h"

using Inkscape::Util::EnumData;
using Inkscape::Util::EnumDataConverter;

const EnumData<NR::FilterPrimitiveType> FPData[NR::NR_FILTER_ENDPRIMITIVETYPE] = {
    {NR::NR_FILTER_BLEND,             _("Blend"),              "svg:feBlend"},
    {NR::NR_FILTER_COLORMATRIX,       _("Color Matrix"),       "svg:feColorMatrix"},
    {NR::NR_FILTER_COMPONENTTRANSFER, _("Component Transfer"), "svg:feComponentTransfer"},
    {NR::NR_FILTER_COMPOSITE,         _("Composite"),          "svg:feComposite"},
    {NR::NR_FILTER_CONVOLVEMATRIX,    _("Convolve Matrix"),    "svg:feConvolveMatrix"},
    {NR::NR_FILTER_DIFFUSELIGHTING,   _("Diffuse Lighting"),   "svg:feDiffuseLighting"},
    {NR::NR_FILTER_DISPLACEMENTMAP,   _("Displacement Map"),   "svg:feDisplacementMap"},
    {NR::NR_FILTER_FLOOD,             _("Flood"),              "svg:feFlood"},
    {NR::NR_FILTER_GAUSSIANBLUR,      _("Gaussian Blur"),      "svg:feGaussianBlur"},
    {NR::NR_FILTER_IMAGE,             _("Image"),              "svg:feImage"},
    {NR::NR_FILTER_MERGE,             _("Merge"),              "svg:feMerge"},
    {NR::NR_FILTER_MORPHOLOGY,        _("Morphology"),         "svg:feMorphology"},
    {NR::NR_FILTER_OFFSET,            _("Offset"),             "svg:feOffset"},
    {NR::NR_FILTER_SPECULARLIGHTING,  _("Specular Lighting"),  "svg:feSpecularLighting"},
    {NR::NR_FILTER_TILE,              _("Tile"),               "svg:feTile"},
    {NR::NR_FILTER_TURBULENCE,        _("Turbulence"),         "svg:feTurbulence"}
};
const EnumDataConverter<NR::FilterPrimitiveType> FPConverter(FPData, NR::NR_FILTER_ENDPRIMITIVETYPE);

const EnumData<FilterPrimitiveInput> FPInputData[FPINPUT_END] = {
    {FPINPUT_SOURCEGRAPHIC,     _("Source Graphic"),     "SourceGraphic"},
    {FPINPUT_SOURCEALPHA,       _("Source Alpha"),       "SourceAlpha"},
    {FPINPUT_BACKGROUNDIMAGE,   _("Background Image"),   "BackgroundImage"},
    {FPINPUT_BACKGROUNDALPHA,   _("Background Alpha"),   "BackgroundAlpha"},
    {FPINPUT_FILLPAINT,         _("Fill Paint"),         "FillPaint"},
    {FPINPUT_STROKEPAINT,       _("Stroke Paint"),       "StrokePaint"},
};
const EnumDataConverter<FilterPrimitiveInput> FPInputConverter(FPInputData, FPINPUT_END);

// feBlend
const EnumData<NR::FilterBlendMode> BlendModeData[NR::BLEND_ENDMODE] = {
    {NR::BLEND_NORMAL,   _("Normal"),   "normal"},
    {NR::BLEND_MULTIPLY, _("Multiply"), "multiply"},
    {NR::BLEND_SCREEN,   _("Screen"),   "screen"},
    {NR::BLEND_DARKEN,   _("Darken"),   "darken"},
    {NR::BLEND_LIGHTEN,  _("Lighten"),  "lighten"}
};
const EnumDataConverter<NR::FilterBlendMode> BlendModeConverter(BlendModeData, NR::BLEND_ENDMODE);


const EnumData<NR::FilterColorMatrixType> ColorMatrixTypeData[NR::COLORMATRIX_ENDTYPE] = {
    {NR::COLORMATRIX_MATRIX,           _("Matrix"),             "matrix"},
    {NR::COLORMATRIX_SATURATE,         _("Saturate"),           "saturate"},
    {NR::COLORMATRIX_HUEROTATE,        _("Hue Rotate"),         "hueRotate"},
    {NR::COLORMATRIX_LUMINANCETOALPHA, _("Luminance to Alpha"), "luminanceToAlpha"}
};
const EnumDataConverter<NR::FilterColorMatrixType> ColorMatrixTypeConverter(ColorMatrixTypeData, NR::COLORMATRIX_ENDTYPE);

// feComposite
const EnumData<FeCompositeOperator> CompositeOperatorData[COMPOSITE_ENDOPERATOR] = {
    {COMPOSITE_DEFAULT,    _("Default"),    ""},
    {COMPOSITE_OVER,       _("Over"),       "over"},
    {COMPOSITE_IN,         _("In"),         "in"},
    {COMPOSITE_OUT,        _("Out"),        "out"},
    {COMPOSITE_ATOP,       _("Atop"),       "atop"},
    {COMPOSITE_XOR,        _("XOR"),        "xor"},
    {COMPOSITE_ARITHMETIC, _("Arithmetic"), "arithmetic"}
};
const EnumDataConverter<FeCompositeOperator> CompositeOperatorConverter(CompositeOperatorData, COMPOSITE_ENDOPERATOR);

// feConvolveMatrix
const EnumData<NR::FilterConvolveMatrixEdgeMode> ConvolveMatrixEdgeModeData[NR::CONVOLVEMATRIX_EDGEMODE_ENDTYPE] = {
    {NR::CONVOLVEMATRIX_EDGEMODE_DUPLICATE, _("Duplicate"), "duplicate"},
    {NR::CONVOLVEMATRIX_EDGEMODE_WRAP, _("Wrap"), "wrap"},
    {NR::CONVOLVEMATRIX_EDGEMODE_NONE, _("None"), "none"}
};
const EnumDataConverter<NR::FilterConvolveMatrixEdgeMode> ConvolveMatrixEdgeModeConverter(ConvolveMatrixEdgeModeData, NR::CONVOLVEMATRIX_EDGEMODE_ENDTYPE);

// Light source
const EnumData<LightSource> LightSourceData[LIGHT_ENDSOURCE] = {
    {LIGHT_DISTANT, _("Distance Light"), "svg:feDistanceLight"},
    {LIGHT_POINT,   _("Point Light"),    "svg:fePointLight"},
    {LIGHT_SPOT,    _("Spot Light"),     "svg:feSpotLight"}
};
const EnumDataConverter<LightSource> LightSourceConverter(LightSourceData, LIGHT_ENDSOURCE);

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

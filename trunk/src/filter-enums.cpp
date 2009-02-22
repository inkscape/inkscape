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

const EnumData<Inkscape::Filters::FilterPrimitiveType> FPData[Inkscape::Filters::NR_FILTER_ENDPRIMITIVETYPE] = {
    {Inkscape::Filters::NR_FILTER_BLEND,             _("Blend"),              "svg:feBlend"},
    {Inkscape::Filters::NR_FILTER_COLORMATRIX,       _("Color Matrix"),       "svg:feColorMatrix"},
    {Inkscape::Filters::NR_FILTER_COMPONENTTRANSFER, _("Component Transfer"), "svg:feComponentTransfer"},
    {Inkscape::Filters::NR_FILTER_COMPOSITE,         _("Composite"),          "svg:feComposite"},
    {Inkscape::Filters::NR_FILTER_CONVOLVEMATRIX,    _("Convolve Matrix"),    "svg:feConvolveMatrix"},
    {Inkscape::Filters::NR_FILTER_DIFFUSELIGHTING,   _("Diffuse Lighting"),   "svg:feDiffuseLighting"},
    {Inkscape::Filters::NR_FILTER_DISPLACEMENTMAP,   _("Displacement Map"),   "svg:feDisplacementMap"},
    {Inkscape::Filters::NR_FILTER_FLOOD,             _("Flood"),              "svg:feFlood"},
    {Inkscape::Filters::NR_FILTER_GAUSSIANBLUR,      _("Gaussian Blur"),      "svg:feGaussianBlur"},
    {Inkscape::Filters::NR_FILTER_IMAGE,             _("Image"),              "svg:feImage"},
    {Inkscape::Filters::NR_FILTER_MERGE,             _("Merge"),              "svg:feMerge"},
    {Inkscape::Filters::NR_FILTER_MORPHOLOGY,        _("Morphology"),         "svg:feMorphology"},
    {Inkscape::Filters::NR_FILTER_OFFSET,            _("Offset"),             "svg:feOffset"},
    {Inkscape::Filters::NR_FILTER_SPECULARLIGHTING,  _("Specular Lighting"),  "svg:feSpecularLighting"},
    {Inkscape::Filters::NR_FILTER_TILE,              _("Tile"),               "svg:feTile"},
    {Inkscape::Filters::NR_FILTER_TURBULENCE,        _("Turbulence"),         "svg:feTurbulence"}
};
const EnumDataConverter<Inkscape::Filters::FilterPrimitiveType> FPConverter(FPData, Inkscape::Filters::NR_FILTER_ENDPRIMITIVETYPE);

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
const EnumData<Inkscape::Filters::FilterBlendMode> BlendModeData[Inkscape::Filters::BLEND_ENDMODE] = {
    //TRANSLATORS: This is a context string, only put the word "Normal" in your translation
    {Inkscape::Filters::BLEND_NORMAL,   Q_("filterBlendMode|Normal"),   "normal"},
    {Inkscape::Filters::BLEND_MULTIPLY, _("Multiply"), "multiply"},
    {Inkscape::Filters::BLEND_SCREEN,   _("Screen"),   "screen"},
    {Inkscape::Filters::BLEND_DARKEN,   _("Darken"),   "darken"},
    {Inkscape::Filters::BLEND_LIGHTEN,  _("Lighten"),  "lighten"}
};
const EnumDataConverter<Inkscape::Filters::FilterBlendMode> BlendModeConverter(BlendModeData, Inkscape::Filters::BLEND_ENDMODE);


const EnumData<Inkscape::Filters::FilterColorMatrixType> ColorMatrixTypeData[Inkscape::Filters::COLORMATRIX_ENDTYPE] = {
    {Inkscape::Filters::COLORMATRIX_MATRIX,           _("Matrix"),             "matrix"},
    {Inkscape::Filters::COLORMATRIX_SATURATE,         _("Saturate"),           "saturate"},
    {Inkscape::Filters::COLORMATRIX_HUEROTATE,        _("Hue Rotate"),         "hueRotate"},
    {Inkscape::Filters::COLORMATRIX_LUMINANCETOALPHA, _("Luminance to Alpha"), "luminanceToAlpha"}
};
const EnumDataConverter<Inkscape::Filters::FilterColorMatrixType> ColorMatrixTypeConverter(ColorMatrixTypeData, Inkscape::Filters::COLORMATRIX_ENDTYPE);

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

// feComponentTransfer
const EnumData<Inkscape::Filters::FilterComponentTransferType> ComponentTransferTypeData[Inkscape::Filters::COMPONENTTRANSFER_TYPE_ERROR] = {
    {Inkscape::Filters::COMPONENTTRANSFER_TYPE_IDENTITY, _("Identity"), "identity"},
    {Inkscape::Filters::COMPONENTTRANSFER_TYPE_TABLE,    _("Table"),    "table"},
    {Inkscape::Filters::COMPONENTTRANSFER_TYPE_DISCRETE, _("Discrete"), "discrete"},
    {Inkscape::Filters::COMPONENTTRANSFER_TYPE_LINEAR,   _("Linear"),   "linear"},
    {Inkscape::Filters::COMPONENTTRANSFER_TYPE_GAMMA,    _("Gamma"),    "gamma"},
};
const EnumDataConverter<Inkscape::Filters::FilterComponentTransferType> ComponentTransferTypeConverter(ComponentTransferTypeData, Inkscape::Filters::COMPONENTTRANSFER_TYPE_ERROR);

// feConvolveMatrix
const EnumData<Inkscape::Filters::FilterConvolveMatrixEdgeMode> ConvolveMatrixEdgeModeData[Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_ENDTYPE] = {
    {Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_DUPLICATE, _("Duplicate"), "duplicate"},
    {Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_WRAP,      _("Wrap"),      "wrap"},
    {Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_NONE,      _("None"),      "none"}
};
const EnumDataConverter<Inkscape::Filters::FilterConvolveMatrixEdgeMode> ConvolveMatrixEdgeModeConverter(ConvolveMatrixEdgeModeData, Inkscape::Filters::CONVOLVEMATRIX_EDGEMODE_ENDTYPE);

// feDisplacementMap
const EnumData<FilterDisplacementMapChannelSelector> DisplacementMapChannelData[DISPLACEMENTMAP_CHANNEL_ENDTYPE] = {
    {DISPLACEMENTMAP_CHANNEL_RED, _("Red"),   "R"},
    {DISPLACEMENTMAP_CHANNEL_GREEN, _("Green"), "G"},
    {DISPLACEMENTMAP_CHANNEL_BLUE, _("Blue"),  "B"},
    {DISPLACEMENTMAP_CHANNEL_ALPHA, _("Alpha"), "A"}
};
const EnumDataConverter<FilterDisplacementMapChannelSelector> DisplacementMapChannelConverter(DisplacementMapChannelData, DISPLACEMENTMAP_CHANNEL_ENDTYPE);

// feMorphology
const EnumData<Inkscape::Filters::FilterMorphologyOperator> MorphologyOperatorData[Inkscape::Filters::MORPHOLOGY_OPERATOR_END] = {
    {Inkscape::Filters::MORPHOLOGY_OPERATOR_ERODE,  _("Erode"),   "erode"},
    {Inkscape::Filters::MORPHOLOGY_OPERATOR_DILATE, _("Dilate"),  "dilate"}
};
const EnumDataConverter<Inkscape::Filters::FilterMorphologyOperator> MorphologyOperatorConverter(MorphologyOperatorData, Inkscape::Filters::MORPHOLOGY_OPERATOR_END);

// feTurbulence
const EnumData<Inkscape::Filters::FilterTurbulenceType> TurbulenceTypeData[Inkscape::Filters::TURBULENCE_ENDTYPE] = {
    {Inkscape::Filters::TURBULENCE_FRACTALNOISE, _("Fractal Noise"), "fractalNoise"},
    {Inkscape::Filters::TURBULENCE_TURBULENCE,   _("Turbulence"),    "turbulence"}
};
const EnumDataConverter<Inkscape::Filters::FilterTurbulenceType> TurbulenceTypeConverter(TurbulenceTypeData, Inkscape::Filters::TURBULENCE_ENDTYPE);

// Light source
const EnumData<LightSource> LightSourceData[LIGHT_ENDSOURCE] = {
    {LIGHT_DISTANT, _("Distant Light"), "svg:feDistantLight"},
    {LIGHT_POINT,   _("Point Light"),   "svg:fePointLight"},
    {LIGHT_SPOT,    _("Spot Light"),    "svg:feSpotLight"}
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

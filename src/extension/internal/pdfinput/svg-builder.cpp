 /** \file
 * Native PDF import using libpoppler.
 * 
 * Authors:
 *   miklos erdelyi
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_POPPLER

#include "svg-builder.h"
#include "pdf-parser.h"

#include <png.h>

#include "document-private.h"
#include "xml/document.h"
#include "xml/node.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "svg/path-string.h"
#include "svg/css-ostringstream.h"
#include "svg/svg-color.h"
#include "color.h"
#include "unit-constants.h"
#include "io/stringstream.h"
#include "io/base64stream.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-macros.h"
#include "libnrtype/font-instance.h"

#include "Function.h"
#include "GfxState.h"
#include "GfxFont.h"
#include "Stream.h"
#include "Page.h"
#include "UnicodeMap.h"
#include "GlobalParams.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

//#define IFTRACE(_code)  _code
#define IFTRACE(_code)

#define TRACE(_args) IFTRACE(g_print _args)


/**
 * \class SvgBuilder
 * 
 */

SvgBuilder::SvgBuilder() {
    _in_text_object = false;
    _need_font_update = true;
    _invalidated_style = true;
    _font_style = NULL;
    _current_font = NULL;
    _current_state = NULL;
}

SvgBuilder::SvgBuilder(SPDocument *document, XRef *xref) {
    _doc = document;
    _xref = xref;
    _xml_doc = sp_document_repr_doc(_doc);
    _container = _root = _doc->rroot;
    SvgBuilder();
}

SvgBuilder::SvgBuilder(SvgBuilder *parent, Inkscape::XML::Node *root) {
    _doc = parent->_doc;
    _xref = parent->_xref;
    _xml_doc = parent->_xml_doc;
    _container = this->_root = root;
    SvgBuilder();
}

SvgBuilder::~SvgBuilder() {
}

void SvgBuilder::setDocumentSize(double width, double height) {
    sp_repr_set_svg_double(_root, "width", width);
    sp_repr_set_svg_double(_root, "height", height);
    this->_width = width;
    this->_height = height;
}

void SvgBuilder::saveState() {
    _group_depth.push_back(0);
    pushGroup();
}

void SvgBuilder::restoreState() {
    while (_group_depth.back() > 0) {
        popGroup();
    }
    _group_depth.pop_back();
}

Inkscape::XML::Node *SvgBuilder::pushGroup() {
    Inkscape::XML::Node *node = _xml_doc->createElement("svg:g");
    _container->appendChild(node);
    _container = node;
    Inkscape::GC::release(node);
    _group_depth.back()++;

    return _container;
}

Inkscape::XML::Node *SvgBuilder::popGroup() {
    if (_container != _root) {  // Pop if the current container isn't root
        _container = _container->parent();
        _group_depth[_group_depth.size()-1] = --_group_depth.back();
    }

    return _container;
}

Inkscape::XML::Node *SvgBuilder::getContainer() {
    return _container;
}

static gchar *svgConvertRGBToText(double r, double g, double b) {
    static gchar tmp[1023] = {0};
    snprintf(tmp, 1023,
             "#%02x%02x%02x",
             CLAMP(SP_COLOR_F_TO_U(r), 0, 255),
             CLAMP(SP_COLOR_F_TO_U(g), 0, 255),
             CLAMP(SP_COLOR_F_TO_U(b), 0, 255));
    return (gchar *)&tmp;
}

static gchar *svgConvertGfxRGB(GfxRGB *color) {
    double r = color->r / 65535.0;
    double g = color->g / 65535.0;
    double b = color->b / 65535.0;
    return svgConvertRGBToText(r, g, b);
}

static void svgSetTransform(Inkscape::XML::Node *node, double c0, double c1,
                              double c2, double c3, double c4, double c5) {
    NR::Matrix matrix(c0, c1, c2, c3, c4, c5);
    gchar *transform_text = sp_svg_transform_write(matrix);
    node->setAttribute("transform", transform_text);
    g_free(transform_text);
}

static void svgSetTransform(Inkscape::XML::Node *node, double *transform) {
    svgSetTransform(node, transform[0], transform[1], transform[2], transform[3],
                    transform[4], transform[5]);
}

/**
 * \brief Generates a SVG path string from poppler's data structure
 */
static gchar *svgInterpretPath(GfxPath *path) {
    GfxSubpath *subpath;
    Inkscape::SVG::PathString pathString;
    int i, j;
    for ( i = 0 ; i < path->getNumSubpaths() ; ++i ) {
        subpath = path->getSubpath(i);
        if (subpath->getNumPoints() > 0) {
            pathString.moveTo(subpath->getX(0), subpath->getY(0));
            j = 1;
            while (j < subpath->getNumPoints()) {
                if (subpath->getCurve(j)) {
                    pathString.curveTo(subpath->getX(j), subpath->getY(j),
                                       subpath->getX(j+1), subpath->getY(j+1),
                                       subpath->getX(j+2), subpath->getY(j+2));

                    j += 3;
                } else {
                    pathString.lineTo(subpath->getX(j), subpath->getY(j));
                    ++j;
                }
            }
            if (subpath->isClosed()) {
                pathString.closePath();
            }
        }
    }

    return g_strdup(pathString.c_str());
}

/**
 * \brief Sets stroke style from poppler's GfxState data structure
 * Uses the given SPCSSAttr for storing the style properties
 */
void SvgBuilder::_setStrokeStyle(SPCSSAttr *css, GfxState *state) {

    // Check line width
    if ( state->getLineWidth() <= 0.0 ) {
        // Ignore stroke
        sp_repr_css_set_property(css, "stroke", "none");
        return;
    }

    // Stroke color/pattern
    if ( state->getStrokeColorSpace()->getMode() == csPattern ) {
        gchar *urltext = _createPattern(state->getStrokePattern(), state, true);
        sp_repr_css_set_property(css, "stroke", urltext);
        if (urltext) {
            g_free(urltext);
        }
    } else {
        GfxRGB stroke_color;
        state->getStrokeRGB(&stroke_color);
        sp_repr_css_set_property(css, "stroke", svgConvertGfxRGB(&stroke_color));
    }

    // Opacity
    Inkscape::CSSOStringStream os_opacity;
    os_opacity << state->getStrokeOpacity();
    sp_repr_css_set_property(css, "stroke-opacity", os_opacity.str().c_str());

    // Line width
    Inkscape::CSSOStringStream os_width;
    os_width << state->getLineWidth();
    sp_repr_css_set_property(css, "stroke-width", os_width.str().c_str());

    // Line cap
    switch (state->getLineCap()) {
        case 0:
            sp_repr_css_set_property(css, "stroke-linecap", "butt");
            break;
        case 1:
            sp_repr_css_set_property(css, "stroke-linecap", "round");
            break;
        case 2:
            sp_repr_css_set_property(css, "stroke-linecap", "square");
            break;
    }

    // Line join
    switch (state->getLineJoin()) {
        case 0:
            sp_repr_css_set_property(css, "stroke-linejoin", "miter");
            break;
        case 1:
            sp_repr_css_set_property(css, "stroke-linejoin", "round");
            break;
        case 2:
            sp_repr_css_set_property(css, "stroke-linejoin", "bevel");
            break;
    }

    // Miterlimit
    Inkscape::CSSOStringStream os_ml;
    os_ml << state->getMiterLimit();
    sp_repr_css_set_property(css, "stroke-miterlimit", os_ml.str().c_str());

    // Line dash
    double *dash_pattern;
    int dash_length;
    double dash_start;
    state->getLineDash(&dash_pattern, &dash_length, &dash_start);
    if ( dash_length > 0 ) {
        Inkscape::CSSOStringStream os_array;
        for ( int i = 0 ; i < dash_length ; i++ ) {
            os_array << dash_pattern[i];
            if (i < (dash_length - 1)) {
                os_array << ",";
            }
        }
        sp_repr_css_set_property(css, "stroke-dasharray", os_array.str().c_str());

        Inkscape::CSSOStringStream os_offset;
        os_offset << dash_start;
        sp_repr_css_set_property(css, "stroke-dashoffset", os_offset.str().c_str());
    } else {
        sp_repr_css_set_property(css, "stroke-dasharray", "none");
        sp_repr_css_set_property(css, "stroke-dashoffset", NULL);
    }
}

/**
 * \brief Sets fill style from poppler's GfxState data structure
 * Uses the given SPCSSAttr for storing the style properties.
 */
void SvgBuilder::_setFillStyle(SPCSSAttr *css, GfxState *state, bool even_odd) {

    // Fill color/pattern
    if ( state->getFillColorSpace()->getMode() == csPattern ) {
        gchar *urltext = _createPattern(state->getFillPattern(), state);
        sp_repr_css_set_property(css, "fill", urltext);
        if (urltext) {
            g_free(urltext);
        }
    } else {
        GfxRGB fill_color;
        state->getFillRGB(&fill_color);
        sp_repr_css_set_property(css, "fill", svgConvertGfxRGB(&fill_color));
    }

    // Opacity
    Inkscape::CSSOStringStream os_opacity;
    os_opacity << state->getFillOpacity();
    sp_repr_css_set_property(css, "fill-opacity", os_opacity.str().c_str());
    
    // Fill rule
    sp_repr_css_set_property(css, "fill-rule", even_odd ? "evenodd" : "nonzero");
}

/**
 * \brief Sets style properties from poppler's GfxState data structure
 * \return SPCSSAttr with all the relevant properties set
 */
SPCSSAttr *SvgBuilder::_setStyle(GfxState *state, bool fill, bool stroke, bool even_odd) {
    SPCSSAttr *css = sp_repr_css_attr_new();
    if (fill) {
        _setFillStyle(css, state, even_odd);
    } else {
        sp_repr_css_set_property(css, "fill", "none");
    }
    
    if (stroke) {
        _setStrokeStyle(css, state);
    } else {
        sp_repr_css_set_property(css, "stroke", "none");
    }

    return css;
}

/**
 * \brief Emits the current path in poppler's GfxState data structure
 * Can be used to do filling and stroking at once.
 *
 * \param fill whether the path should be filled
 * \param stroke whether the path should be stroked
 * \param even_odd whether the even-odd rule should be used when filling the path
 */
void SvgBuilder::addPath(GfxState *state, bool fill, bool stroke, bool even_odd) {
    Inkscape::XML::Node *path = _xml_doc->createElement("svg:path");
    gchar *pathtext = svgInterpretPath(state->getPath());
    path->setAttribute("d", pathtext);
    g_free(pathtext);

    // Set style
    SPCSSAttr *css = _setStyle(state, fill, stroke, even_odd);
    sp_repr_css_change(path, css, "style");
    sp_repr_css_attr_unref(css);

    _container->appendChild(path);
    Inkscape::GC::release(path);
}

/**
 * \brief Clips to the current path set in GfxState
 * \param state poppler's data structure
 * \param even_odd whether the even-odd rule should be applied
 */
void SvgBuilder::clip(GfxState *state, bool even_odd) {
    pushGroup();
    setClipPath(state, even_odd);
}

void SvgBuilder::setClipPath(GfxState *state, bool even_odd) {
    // Create the clipPath repr
    Inkscape::XML::Node *clip_path = _xml_doc->createElement("svg:clipPath");
    clip_path->setAttribute("clipPathUnits", "userSpaceOnUse");
    // Create the path
    Inkscape::XML::Node *path = _xml_doc->createElement("svg:path");
    gchar *pathtext = svgInterpretPath(state->getPath());
    path->setAttribute("d", pathtext);
    g_free(pathtext);
    clip_path->appendChild(path);
    Inkscape::GC::release(path);
    // Append clipPath to defs and get id
    SP_OBJECT_REPR (SP_DOCUMENT_DEFS (_doc))->appendChild(clip_path);
    gchar *urltext = g_strdup_printf ("url(#%s)", clip_path->attribute("id"));
    Inkscape::GC::release(clip_path);
    _container->setAttribute("clip-path", urltext);
    g_free(urltext);
}

/**
 * \brief Fills the given array with the current container's transform, if set
 * \param transform array of doubles to be filled
 * \return true on success; false on invalid transformation
 */
bool SvgBuilder::getTransform(double *transform) {
    NR::Matrix svd;
    gchar const *tr = _container->attribute("transform");
    bool valid = sp_svg_transform_read(tr, &svd);
    if (valid) {
        for ( int i = 0 ; i < 6 ; i++ ) {
            transform[i] = svd[i];
        }
        return true;
    } else {
        return false;
    }
}

/**
 * \brief Sets the transformation matrix of the current container
 */
void SvgBuilder::setTransform(double c0, double c1, double c2, double c3,
                              double c4, double c5) {

    TRACE(("setTransform: %f %f %f %f %f %f\n", c0, c1, c2, c3, c4, c5));
    svgSetTransform(_container, c0, c1, c2, c3, c4, c5);
}

void SvgBuilder::setTransform(double *transform) {
    setTransform(transform[0], transform[1], transform[2], transform[3],
                 transform[4], transform[5]);
}

/**
 * \brief Checks whether the given pattern type can be represented in SVG
 * Used by PdfParser to decide when to do fallback operations.
 */
bool SvgBuilder::isPatternTypeSupported(GfxPattern *pattern) {
    if ( pattern != NULL ) {
        if ( pattern->getType() == 2 ) {    // shading pattern
            GfxShading *shading = ((GfxShadingPattern *)pattern)->getShading();
            int shadingType = shading->getType();
            if ( shadingType == 2 || // axial shading
                 shadingType == 3 ) {   // radial shading
                return true;
            }
            return false;
        } else if ( pattern->getType() == 1 ) {   // tiling pattern
            return true;
        }
    }

    return false;
}

/**
 * \brief Creates a pattern from poppler's data structure
 * Handles linear and radial gradients. Creates a new PdfParser and uses it to
 * build a tiling pattern.
 * \return an url pointing to the created pattern
 */
gchar *SvgBuilder::_createPattern(GfxPattern *pattern, GfxState *state, bool is_stroke) {
    gchar *id = NULL;
    if ( pattern != NULL ) {
        if ( pattern->getType() == 2 ) {  // Shading pattern
            id = _createGradient((GfxShadingPattern*)pattern);
        } else if ( pattern->getType() == 1 ) {   // Tiling pattern
            id = _createTilingPattern((GfxTilingPattern*)pattern, state, is_stroke);
        }
    } else {
        return NULL;
    }
    gchar *urltext = g_strdup_printf ("url(#%s)", id);
    g_free(id);
    return urltext;
}

/**
 * \brief Creates a tiling pattern from poppler's data structure
 * Creates a sub-page PdfParser and uses it to parse the pattern's content stream.
 * \return id of the created pattern
 */
gchar *SvgBuilder::_createTilingPattern(GfxTilingPattern *tiling_pattern,
                                        GfxState *state, bool is_stroke) {

    Inkscape::XML::Node *pattern_node = _xml_doc->createElement("svg:pattern");
    // Set pattern transform matrix
    double *p2u = tiling_pattern->getMatrix();
    NR::Matrix pat_matrix(p2u[0], p2u[1], p2u[2], p2u[3], p2u[4], p2u[5]);
    gchar *transform_text = sp_svg_transform_write(pat_matrix);
    pattern_node->setAttribute("patternTransform", transform_text);
    g_free(transform_text);
    pattern_node->setAttribute("patternUnits", "userSpaceOnUse");
    // Set pattern tiling
    // FIXME: don't ignore XStep and YStep
    double *bbox = tiling_pattern->getBBox();
    sp_repr_set_svg_double(pattern_node, "x", 0.0);
    sp_repr_set_svg_double(pattern_node, "y", 0.0);
    sp_repr_set_svg_double(pattern_node, "width", bbox[2] - bbox[0]);
    sp_repr_set_svg_double(pattern_node, "height", bbox[3] - bbox[1]);

    // Convert BBox for PdfParser
    PDFRectangle box;
    box.x1 = bbox[0];
    box.y1 = bbox[1];
    box.x2 = bbox[2];
    box.y2 = bbox[3];
    // Create new SvgBuilder and sub-page PdfParser
    SvgBuilder *pattern_builder = new SvgBuilder(this, pattern_node);
    PdfParser *pdf_parser = new PdfParser(_xref, pattern_builder, tiling_pattern->getResDict(),
                                          &box);
    // Get pattern color space
    GfxPatternColorSpace *pat_cs = (GfxPatternColorSpace *)( is_stroke ? state->getStrokeColorSpace()
                                                            : state->getFillColorSpace() );
    // Set fill/stroke colors if this is an uncolored tiling pattern
    GfxColorSpace *cs = NULL;
    if ( tiling_pattern->getPaintType() == 2 && ( cs = pat_cs->getUnder() ) ) {
        GfxState *pattern_state = pdf_parser->getState();
        pattern_state->setFillColorSpace(cs->copy());
        pattern_state->setFillColor(state->getFillColor());
        pattern_state->setStrokeColorSpace(cs->copy());
        pattern_state->setStrokeColor(state->getFillColor());
    }

    // Generate the SVG pattern
    pdf_parser->parse(tiling_pattern->getContentStream());

    // Cleanup
    delete pdf_parser;
    delete pattern_builder;

    // Append the pattern to defs
    SP_OBJECT_REPR (SP_DOCUMENT_DEFS (_doc))->appendChild(pattern_node);
    gchar *id = g_strdup(pattern_node->attribute("id"));
    Inkscape::GC::release(pattern_node);

    return id;
}

/**
 * \brief Creates a linear or radial gradient from poppler's data structure
 * \return id of the created object
 */
gchar *SvgBuilder::_createGradient(GfxShadingPattern *shading_pattern) {
    GfxShading *shading = shading_pattern->getShading();
    Inkscape::XML::Node *gradient;
    Function *func;
    int num_funcs;
    bool extend0, extend1;

    if ( shading->getType() == 2 ) {  // Axial shading
        gradient = _xml_doc->createElement("svg:linearGradient");
        GfxAxialShading *axial_shading = (GfxAxialShading*)shading;
        double x1, y1, x2, y2;
        axial_shading->getCoords(&x1, &y1, &x2, &y2);
        sp_repr_set_svg_double(gradient, "x1", x1);
        sp_repr_set_svg_double(gradient, "y1", y1);
        sp_repr_set_svg_double(gradient, "x2", x2);
        sp_repr_set_svg_double(gradient, "y2", y2);
        extend0 = axial_shading->getExtend0();
        extend1 = axial_shading->getExtend1();
        num_funcs = axial_shading->getNFuncs();
        func = axial_shading->getFunc(0);
    } else if (shading->getType() == 3) {   // Radial shading
        gradient = _xml_doc->createElement("svg:radialGradient");
        GfxRadialShading *radial_shading = (GfxRadialShading*)shading;
        double x1, y1, r1, x2, y2, r2;
        radial_shading->getCoords(&x1, &y1, &r1, &x2, &y2, &r2);
        // FIXME: the inner circle's radius is ignored here
        sp_repr_set_svg_double(gradient, "fx", x1);
        sp_repr_set_svg_double(gradient, "fy", y1);
        sp_repr_set_svg_double(gradient, "cx", x2);
        sp_repr_set_svg_double(gradient, "cy", y2);
        sp_repr_set_svg_double(gradient, "r", r2);
        extend0 = radial_shading->getExtend0();
        extend1 = radial_shading->getExtend1();
        num_funcs = radial_shading->getNFuncs();
        func = radial_shading->getFunc(0);
    } else {    // Unsupported shading type
        return NULL;
    }
    gradient->setAttribute("gradientUnits", "userSpaceOnUse");
    // Flip the gradient transform around the y axis
    double *p2u = shading_pattern->getMatrix();
    NR::Matrix pat_matrix(p2u[0], p2u[1], p2u[2], p2u[3], p2u[4], p2u[5]);
    NR::Matrix flip(1.0, 0.0, 0.0, -1.0, 0.0, _height * PT_PER_PX);
    pat_matrix *= flip;
    gchar *transform_text = sp_svg_transform_write(pat_matrix);
    gradient->setAttribute("gradientTransform", transform_text);
    g_free(transform_text);

    if ( extend0 && extend1 ) {
        gradient->setAttribute("spreadMethod", "pad");
    }

    if ( num_funcs > 1 || !_addStopsToGradient(gradient, func, 1.0) ) {
        Inkscape::GC::release(gradient);
        return NULL;
    }

    Inkscape::XML::Node *defs = SP_OBJECT_REPR (SP_DOCUMENT_DEFS (_doc));
    defs->appendChild(gradient);
    gchar *id = g_strdup(gradient->attribute("id"));
    Inkscape::GC::release(gradient);

    return id;
}

#define EPSILON 0.0001
bool SvgBuilder::_addSamplesToGradient(Inkscape::XML::Node *gradient,
                                       SampledFunction *func, double offset0,
                                       double offset1, double opacity) {

    // Check whether this sampled function can be converted to color stops
    int sample_size = func->getSampleSize(0);
    if ( sample_size != 2 )
        return false;
    int num_comps = func->getOutputSize();
    if ( num_comps != 3 )
        return false;

    double *samples = func->getSamples();
    unsigned stop_count = gradient->childCount();
    bool is_continuation = false;
    // Check if this sampled function is the continuation of the previous one
    if ( stop_count > 0 ) {
        // Get previous stop
        Inkscape::XML::Node *prev_stop = gradient->nthChild(stop_count-1);
        // Read its properties
        double prev_offset;
        sp_repr_get_double(prev_stop, "offset", &prev_offset);
        SPCSSAttr *css = sp_repr_css_attr(prev_stop, "style");
        guint32 prev_stop_color = sp_svg_read_color(sp_repr_css_property(css, "stop-color", NULL), 0);
        sp_repr_css_attr_unref(css);
        // Convert colors
        double r = SP_RGBA32_R_F (prev_stop_color);
        double g = SP_RGBA32_G_F (prev_stop_color);
        double b = SP_RGBA32_B_F (prev_stop_color);
        if ( fabs(prev_offset - offset0) < EPSILON &&
             fabs(samples[0] - r) < EPSILON &&
             fabs(samples[1] - g) < EPSILON &&
             fabs(samples[2] - b) < EPSILON ) {

            is_continuation = true;
        }
    }

    int i = is_continuation ? num_comps : 0;
    while (i < sample_size*num_comps) {
        Inkscape::XML::Node *stop = _xml_doc->createElement("svg:stop");
        SPCSSAttr *css = sp_repr_css_attr_new();
        Inkscape::CSSOStringStream os_opacity;
        os_opacity << opacity;
        sp_repr_css_set_property(css, "stop-opacity", os_opacity.str().c_str());
        gchar c[64];
        sp_svg_write_color (c, 64, SP_RGBA32_F_COMPOSE (samples[i], samples[i+1], samples[i+2], 1.0));
        sp_repr_css_set_property(css, "stop-color", c);
        sp_repr_css_change(stop, css, "style");
        sp_repr_css_attr_unref(css);
        sp_repr_set_css_double(stop, "offset", ( i < num_comps ) ? offset0 : offset1);

        gradient->appendChild(stop);
        Inkscape::GC::release(stop);
        i += num_comps;
    }
  
    return true;
}

bool SvgBuilder::_addStopsToGradient(Inkscape::XML::Node *gradient, Function *func,
                                     double opacity) {
    
    int type = func->getType();
    if ( type == 0 ) {  // Sampled
        SampledFunction *sampledFunc = (SampledFunction*)func;
        _addSamplesToGradient(gradient, sampledFunc, 0.0, 1.0, opacity);
    } else if ( type == 3 ) { // Stitching
        StitchingFunction *stitchingFunc = (StitchingFunction*)func;
        double *bounds = stitchingFunc->getBounds();
        int num_funcs = stitchingFunc->getNumFuncs();
        // Add samples from all the stitched functions
        for ( int i = 0 ; i < num_funcs ; i++ ) {
            Function *func = stitchingFunc->getFunc(i);
            if ( func->getType() != 0 ) // Only sampled functions are supported
                continue;
           
            SampledFunction *sampledFunc = (SampledFunction*)func;
            _addSamplesToGradient(gradient, sampledFunc, bounds[i],
                                  bounds[i+1], opacity);
        }
    } else { // Unsupported function type
        return false;
    }

    return true;
}

/**
 * \brief Sets _invalidated_style to true to indicate that styles have to be updated
 * Used for text output when glyphs are buffered till a font change
 */
void SvgBuilder::updateStyle(GfxState *state) {
    if (_in_text_object) {
        _invalidated_style = true;
        _current_state = state;
    }
}

/**
 * This array holds info about translating font weight names to more or less CSS equivalents
 */
static char *font_weight_translator[][2] = {
    {"bold", "bold"},
    {"light", "300"},
    {"black", "900"},
    {"heavy", "900"},
    {"ultrabold", "800"},
    {"extrabold", "800"},
    {"demibold", "600"},
    {"semibold", "600"},
    {"medium", "500"},
    {"book", "normal"},
    {"regular", "normal"},
    {"roman", "normal"},
    {"normal", "normal"},
    {"ultralight", "200"},
    {"extralight", "200"},
    {"thin", "100"}
};

/**
 * \brief Updates _font_style according to the font set in parameter state
 */
void SvgBuilder::updateFont(GfxState *state) {

    TRACE(("updateFont()\n"));
    _need_font_update = false;
    // Flush buffered text before resetting matrices and font style
    _flushText();

    if (_font_style) {
        //sp_repr_css_attr_unref(_font_style);
    }
    _font_style = sp_repr_css_attr_new();
    GfxFont *font = state->getFont();
    // Store original name
    if (font->getOrigName()) {
        _font_specification = font->getOrigName()->getCString();
    } else {
        _font_specification = font->getName()->getCString();
    }

    // Prune the font name to get the correct font family name
    // In a PDF font names can look like this: IONIPB+MetaPlusBold-Italic
    char *font_family = NULL;
    char *font_style = NULL;
    char *font_style_lowercase = NULL;
    char *plus_sign = strstr(_font_specification, "+");
    if (plus_sign) {
        font_family = g_strdup(plus_sign + 1);
        _font_specification = plus_sign + 1;
    } else {
        font_family = g_strdup(_font_specification);
    }
    char *minus_sign = g_strrstr(font_family, "-");
    if (minus_sign) {
        font_style = minus_sign + 1;
        font_style_lowercase = g_ascii_strdown(font_style, -1);
        minus_sign[0] = 0;
    }

    // Font family
    if (font->getFamily()) {
        const gchar *family = font->getFamily()->getCString();
        sp_repr_css_set_property(_font_style, "font-family", font->getFamily()->getCString());
    } else {
        sp_repr_css_set_property(_font_style, "font-family", font_family);
    }

    // Font style
    if (font->isItalic()) {
        sp_repr_css_set_property(_font_style, "font-style", "italic");
    } else if (font_style) {
        if ( strstr(font_style_lowercase, "italic") ||
             strstr(font_style_lowercase, "slanted") ) {
            sp_repr_css_set_property(_font_style, "font-style", "italic");
        } else if (strstr(font_style_lowercase, "oblique")) {
            sp_repr_css_set_property(_font_style, "font-style", "oblique");
        }
    }

    // Font variant -- default 'normal' value
    sp_repr_css_set_property(_font_style, "font-variant", "normal");

    // Font weight
    GfxFont::Weight font_weight = font->getWeight();
    if ( font_weight != GfxFont::WeightNotDefined ) {
        if ( font_weight == GfxFont::W400 ) {
            sp_repr_css_set_property(_font_style, "font-weight", "normal");
        } else if ( font_weight == GfxFont::W700 ) {
            sp_repr_css_set_property(_font_style, "font-weight", "bold");
        } else {
            gchar weight_num[4] = "100";
            weight_num[0] = (gchar)( '1' + (font_weight - GfxFont::W100) );
            sp_repr_css_set_property(_font_style, "font-weight", (gchar *)&weight_num);
        }
    } else if (font_style) {
        // Apply the font weight translations
        int num_translations = sizeof(font_weight_translator) / ( 2 * sizeof(char *) );
        for ( int i = 0 ; i < num_translations ; i++ ) {
            if (strstr(font_style_lowercase, font_weight_translator[i][0])) {
                sp_repr_css_set_property(_font_style, "font-weight",
                                         font_weight_translator[i][1]);
            }
        }
    }
    g_free(font_family);
    if (font_style_lowercase) {
        g_free(font_style_lowercase);
    }

    // Font stretch
    GfxFont::Stretch font_stretch = font->getStretch();
    gchar *stretch_value = NULL;
    switch (font_stretch) {
        case GfxFont::UltraCondensed:
            stretch_value = "ultra-condensed";
            break;
        case GfxFont::ExtraCondensed:
            stretch_value = "extra-condensed";
            break;
        case GfxFont::Condensed:
            stretch_value = "condensed";
            break;
        case GfxFont::SemiCondensed:
            stretch_value = "semi-condensed";
            break;
        case GfxFont::Normal:
            stretch_value = "normal";
            break;
        case GfxFont::SemiExpanded:
            stretch_value = "semi-expanded";
            break;
        case GfxFont::Expanded:
            stretch_value = "expanded";
            break;
        case GfxFont::ExtraExpanded:
            stretch_value = "extra-expanded";
            break;
        case GfxFont::UltraExpanded:
            stretch_value = "ultra-expanded";
            break;
        default:
            break;
    }
    if ( stretch_value != NULL ) {
        sp_repr_css_set_property(_font_style, "font-stretch", stretch_value);
    }

    // Font size
    Inkscape::CSSOStringStream os_font_size;
    double *text_matrix = state->getTextMat();
    double font_size = sqrt( text_matrix[0] * text_matrix[3] - text_matrix[1] * text_matrix[2] );
    font_size *= state->getFontSize() * state->getHorizScaling();
    os_font_size << font_size;
    sp_repr_css_set_property(_font_style, "font-size", os_font_size.str().c_str());

    // Writing mode
    if ( font->getWMode() == 0 ) {
        sp_repr_css_set_property(_font_style, "writing-mode", "lr");
    } else {
        sp_repr_css_set_property(_font_style, "writing-mode", "tb");
    }

    // Calculate new text matrix
    double *font_matrix = font->getFontMatrix();
    NR::Matrix nr_font_matrix(font_matrix[0], font_matrix[1], font_matrix[2],
                              font_matrix[3], font_matrix[4], font_matrix[5]);
    NR::Matrix new_text_matrix(text_matrix[0], text_matrix[1],
                               -text_matrix[2], -text_matrix[3],
                               0.0, 0.0);
    new_text_matrix *= NR::scale( 1.0 / font_size, 1.0 / font_size );
    _text_matrix = nr_font_matrix * new_text_matrix;

    _current_font = font;
}

/**
 * \brief Writes the buffered characters to the SVG document
 */
void SvgBuilder::_flushText() {
    // Ignore empty strings
    if ( _glyphs.size() < 1 ) {
        _glyphs.clear();
        return;
    }
    const SvgGlyph& first_glyph = _glyphs[0];
    int render_mode = first_glyph.render_mode;
    // Ignore invisible characters
    if ( render_mode == 3 ) {
        _glyphs.clear();
        return;
    }

    Inkscape::XML::Node *text_node = _xml_doc->createElement("svg:text");
    text_node->setAttribute("xml:space", "preserve");
    // Set current text position
    sp_repr_set_svg_double(text_node, "x", first_glyph.transformed_position[0]);
    sp_repr_set_svg_double(text_node, "y", first_glyph.transformed_position[1]);
    // Set style
    sp_repr_css_change(text_node, first_glyph.style, "style");
    text_node->setAttribute("inkscape:font-specification", _font_specification);
    // Set text matrix
    gchar *transform = sp_svg_transform_write(_text_matrix);
    text_node->setAttribute("transform", transform);
    g_free(transform);

    bool new_tspan = true;
    Inkscape::XML::Node *tspan_node = NULL;
    Glib::ustring x_coords;
    Glib::ustring y_coords;
    Glib::ustring text_buffer;
    bool is_vertical = !strcmp(sp_repr_css_property(_font_style, "writing-mode", "lr"), "tb");  // FIXME

    // Output all buffered glyphs
    std::vector<SvgGlyph>::iterator i = _glyphs.begin();
    while (1) {
        const SvgGlyph& glyph = (*i);
        std::vector<SvgGlyph>::iterator prev_iterator = i - 1;
        // Check if we need to make a new tspan
        if (glyph.style_changed) {
            new_tspan = true;
        } else if ( i != _glyphs.begin() ) {
            const SvgGlyph& prev_glyph = (*prev_iterator);
            if ( ( is_vertical && prev_glyph.transformed_position[0] != glyph.transformed_position[0] ) ||
                 ( !is_vertical && prev_glyph.transformed_position[1] != glyph.transformed_position[1] ) ) {
                new_tspan = true;
            }
        }
        // Create tspan node if needed
        if ( new_tspan || i == _glyphs.end() ) {
            if (tspan_node) {
                // Set the x and y coordinate arrays
                tspan_node->setAttribute("x", x_coords.c_str());
                tspan_node->setAttribute("y", y_coords.c_str());
                TRACE(("tspan content: %s\n", text_buffer.c_str()));
                // Add text content node to tspan
                Inkscape::XML::Node *text_content = _xml_doc->createTextNode(text_buffer.c_str());
                tspan_node->appendChild(text_content);
                Inkscape::GC::release(text_content);
                text_node->appendChild(tspan_node);
                // Clear temporary buffers
                x_coords.clear();
                y_coords.clear();
                text_buffer.clear();
                Inkscape::GC::release(tspan_node);
            }
            if ( i == _glyphs.end() ) {
                sp_repr_css_attr_unref((*prev_iterator).style);
                break;
            } else {
                tspan_node = _xml_doc->createElement("svg:tspan");
                tspan_node->setAttribute("sodipodi:role", "line");
                // Set style and unref SPCSSAttr if it won't be needed anymore
                if ( i != _glyphs.begin() ) {
                    sp_repr_css_change(tspan_node, glyph.style, "style");
                    if (glyph.style_changed) {  // Free previous style
                        sp_repr_css_attr_unref((*prev_iterator).style);
                    }
                }
            }
            new_tspan = false;
        }
        if ( x_coords.length() > 0 ) {
            x_coords.append(" ");
            y_coords.append(" ");
        }
        // Append the coordinates to their respective strings
        Inkscape::CSSOStringStream os_x;
        os_x << glyph.transformed_position[0];
        x_coords.append(os_x.str());
        Inkscape::CSSOStringStream os_y;
        os_y << glyph.transformed_position[1];
        y_coords.append(os_y.str());

        // Append the character to the text buffer
        text_buffer.append((char *)&glyph.code, 1);

        i++;
    }
    _container->appendChild(text_node);
    Inkscape::GC::release(text_node);

    _glyphs.clear();
}

void SvgBuilder::beginString(GfxState *state, GooString *s) {
    if (_need_font_update) {
        updateFont(state);
    }
    IFTRACE(double *m = state->getTextMat());
    TRACE(("tm: %f %f %f %f %f %f\n",m[0], m[1],m[2], m[3], m[4], m[5]));
    IFTRACE(m = _current_font->getFontMatrix());
    TRACE(("fm: %f %f %f %f %f %f\n",m[0], m[1],m[2], m[3], m[4], m[5]));
    IFTRACE(m = state->getCTM());
    TRACE(("ctm: %f %f %f %f %f %f\n",m[0], m[1],m[2], m[3], m[4], m[5]));
}

/**
 * \brief Adds the specified character to the text buffer
 * Takes care of converting it to UTF-8 and generates a new style repr if style
 * has changed since the last call.
 */
void SvgBuilder::addChar(GfxState *state, double x, double y,
                         double dx, double dy,
                         double originX, double originY,
                         CharCode code, int nBytes, Unicode *u, int uLen) {


    bool is_space = ( uLen == 1 && u[0] == 32 );
    // Skip beginning space
    if ( is_space && _glyphs.size() < 1 ) {
         return;
    }
    // Allow only one space in a row
    if ( is_space && _glyphs[_glyphs.size() - 1].code_size == 1 &&
         _glyphs[_glyphs.size() - 1].code[0] == 32 ) {
        return;
    }

    SvgGlyph new_glyph;
    new_glyph.is_space = is_space;
    new_glyph.position = NR::Point( x - originX, y - originY );
    new_glyph.transformed_position = new_glyph.position * _text_matrix;
    new_glyph.dx = dx;
    new_glyph.dy = dy;

    // Convert the character to UTF-8 since that's our SVG document's encoding
    static UnicodeMap *u_map = NULL;
    if ( u_map == NULL ) {
        GooString *enc = new GooString("UTF-8");
        u_map = globalParams->getUnicodeMap(enc);
        u_map->incRefCnt();
        delete enc;
    }
    int code_size = 0;
    for ( int i = 0 ; i < uLen ; i++ ) {
        code_size += u_map->mapUnicode(u[i], (char *)&new_glyph.code[code_size], sizeof(new_glyph.code) - code_size);
    }
    new_glyph.code_size = code_size;

    // Copy current style if it has changed since the previous glyph
    if (_invalidated_style || _glyphs.size() == 0 ) {
        new_glyph.style_changed = true;
        int render_mode = state->getRender();
        // Set style
        bool has_fill = !( render_mode & 1 );
        bool has_stroke = ( render_mode & 3 ) == 1 || ( render_mode & 3 ) == 2;
        new_glyph.style = _setStyle(state, has_fill, has_stroke);
        new_glyph.render_mode = render_mode;
        sp_repr_css_merge(new_glyph.style, _font_style); // Merge with font style
        _invalidated_style = false;
    } else {
        new_glyph.style_changed = false;
        // Point to previous glyph's style information
        const SvgGlyph& prev_glyph = _glyphs.back();
        new_glyph.style = prev_glyph.style;
        new_glyph.render_mode = prev_glyph.render_mode;
    }
    _glyphs.push_back(new_glyph);
}

void SvgBuilder::endString(GfxState *state) {
}

void SvgBuilder::beginTextObject(GfxState *state) {
    _in_text_object = true;
    _invalidated_style = true;  // Force copying of current state
    _current_state = state;
}

void SvgBuilder::endTextObject(GfxState *state) {
    _flushText();
    // TODO: clip if render_mode >= 4
    _in_text_object = false;
}

/**
 * Helper functions for supporting direct PNG output into a base64 encoded stream
 */
void png_write_base64stream(png_structp png_ptr, png_bytep data, png_size_t length)
{
    Inkscape::IO::Base64OutputStream *stream =
            (Inkscape::IO::Base64OutputStream*)png_get_io_ptr(png_ptr); // Get pointer to stream
    for ( unsigned i = 0 ; i < length ; i++ ) {
        stream->put((int)data[i]);
    }
}

void png_flush_base64stream(png_structp png_ptr)
{
    Inkscape::IO::Base64OutputStream *stream =
            (Inkscape::IO::Base64OutputStream*)png_get_io_ptr(png_ptr); // Get pointer to stream
    stream->flush();
}

/**
 * \brief Creates an <image> element containing the given ImageStream as a PNG
 *
 */
Inkscape::XML::Node *SvgBuilder::_createImage(Stream *str, int width, int height,
        GfxImageColorMap *color_map, int *mask_colors, bool alpha_only, bool invert_alpha) {

    // Create PNG write struct
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if ( png_ptr == NULL ) {
        return NULL;
    }
    // Create PNG info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if ( info_ptr == NULL ) {
        png_destroy_write_struct(&png_ptr, NULL);
        return NULL;
    }
    // Set error handler
    if (setjmp(png_ptr->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return NULL;
    }

    // Set read/write functions
    Inkscape::IO::StringOutputStream base64_string;
    Inkscape::IO::Base64OutputStream base64_stream(base64_string);
    FILE *fp;
    gchar *file_name;
    bool embed_image = true;
    if (embed_image) {
        base64_stream.setColumnWidth(0);   // Disable line breaks
        png_set_write_fn(png_ptr, &base64_stream, png_write_base64stream, png_flush_base64stream);
    } else {
        static int counter = 0;
        file_name = g_strdup_printf("createImage%d.png", counter++);
        fp = fopen(file_name, "wb");
        if ( fp == NULL ) {
            png_destroy_write_struct(&png_ptr, &info_ptr);
            g_free(file_name);
            return NULL;
        }
        png_init_io(png_ptr, fp);
    }

    // Set header data
    if (!invert_alpha) {
        png_set_invert_alpha(png_ptr);
    }
    png_color_8 sig_bit;
    if (alpha_only) {
        png_set_IHDR(png_ptr, info_ptr,
                     width,
                     height,
                     8, /* bit_depth */
                     PNG_COLOR_TYPE_GRAY,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE,
                     PNG_FILTER_TYPE_BASE);
        sig_bit.red = 0;
        sig_bit.green = 0;
        sig_bit.blue = 0;
        sig_bit.gray = 8;
        sig_bit.alpha = 0;
    } else {
        png_set_IHDR(png_ptr, info_ptr,
                     width,
                     height,
                     8, /* bit_depth */
                     PNG_COLOR_TYPE_RGB_ALPHA,
                     PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE,
                     PNG_FILTER_TYPE_BASE);
        sig_bit.red = 8;
        sig_bit.green = 8;
        sig_bit.blue = 8;
        sig_bit.alpha = 8;
    }
    png_set_sBIT(png_ptr, info_ptr, &sig_bit);
    png_set_bgr(png_ptr);
    // Write the file header
    png_write_info(png_ptr, info_ptr);

    // Convert pixels
    ImageStream *image_stream;
    if (alpha_only) {
        if (color_map) {
            image_stream = new ImageStream(str, width, color_map->getNumPixelComps(),
                                           color_map->getBits());
        } else {
            image_stream = new ImageStream(str, width, 1, 1);
        }
        image_stream->reset();

        // Convert grayscale values
        if (color_map) {
            unsigned char *buffer = new unsigned char[width];
            for ( int y = 0 ; y < height ; y++ ) {
                unsigned char *row = image_stream->getLine();
                color_map->getGrayLine(row, buffer, width);
                png_write_row(png_ptr, (png_bytep)buffer);
            }
            delete buffer;
        } else {
            for ( int y = 0 ; y < height ; y++ ) {
                unsigned char *row = image_stream->getLine();
                png_write_row(png_ptr, (png_bytep)row);
            }
        }
    } else if (color_map) {
        image_stream = new ImageStream(str, width,
                                       color_map->getNumPixelComps(),
                                       color_map->getBits());
        image_stream->reset();

        // Convert RGB values
        unsigned int *buffer = new unsigned int[width];
        if (mask_colors) {
            for ( int y = 0 ; y < height ; y++ ) {
                unsigned char *row = image_stream->getLine();
                color_map->getRGBLine(row, buffer, width);

                unsigned int *dest = buffer;
                for ( int x = 0 ; x < width ; x++ ) {
                    // Check each color component against the mask
                    for ( int i = 0; i < color_map->getNumPixelComps() ; i++) {
                        if ( row[i] < mask_colors[2*i] * 255 ||
                             row[i] > mask_colors[2*i + 1] * 255 ) {
                            *dest = *dest | 0xff000000;
                            break;
                        }
                    }
                    // Advance to the next pixel
                    row += color_map->getNumPixelComps();
                    dest++;
                }
                // Write it to the PNG
                png_write_row(png_ptr, (png_bytep)buffer);
            }
        } else {
            for ( int i = 0 ; i < height ; i++ ) {
                unsigned char *row = image_stream->getLine();
                memset((void*)buffer, 0xff, sizeof(int) * width);
                color_map->getRGBLine(row, buffer, width);
                png_write_row(png_ptr, (png_bytep)buffer);
            }
        }
        delete buffer;

    } else {    // A colormap must be provided, so quit
        png_destroy_write_struct(&png_ptr, &info_ptr);
        if (!embed_image) {
            fclose(fp);
            g_free(file_name);
        }
        return NULL;
    }
    delete image_stream;
    str->close();
    // Close PNG
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    base64_stream.close();

    // Create repr
    Inkscape::XML::Node *image_node = _xml_doc->createElement("svg:image");
    sp_repr_set_svg_double(image_node, "width", width);
    sp_repr_set_svg_double(image_node, "height", height);
    // Set transformation
    svgSetTransform(image_node, 1.0/(double)width, 0.0, 0.0, -1.0/(double)height, 0.0, 1.0);

    // Create href
    if (embed_image) {
        // Append format specification to the URI
        Glib::ustring& png_data = base64_string.getString();
        png_data.insert(0, "data:image/png;base64,");
        image_node->setAttribute("xlink:href", png_data.c_str());
    } else {
        fclose(fp);
        image_node->setAttribute("xlink:href", file_name);
        g_free(file_name);
    }

    return image_node;
}

void SvgBuilder::addImage(GfxState *state, Stream *str, int width, int height,
                          GfxImageColorMap *color_map, int *mask_colors) {

     Inkscape::XML::Node *image_node = _createImage(str, width, height, color_map, mask_colors);
     if (image_node) {
         _container->appendChild(image_node);
        Inkscape::GC::release(image_node);
     }
}


} } } /* namespace Inkscape, Extension, Internal */

#endif /* HAVE_POPPLER */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

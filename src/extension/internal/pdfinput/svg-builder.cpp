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

#include "Function.h"
#include "GfxState.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

#define LOG(expr) ( expr );

/**
 * \class SvgBuilder
 * 
 */

SvgBuilder::SvgBuilder(SPDocument *document) {
    doc = document;
    xml_doc = sp_document_repr_doc(doc);
    container = root = doc->rroot;
}

SvgBuilder::SvgBuilder(SvgBuilder *parent, Inkscape::XML::Node *root) {
    doc = parent->doc;
    xml_doc = parent->xml_doc;
    container = this->root = root;
}

SvgBuilder::~SvgBuilder() {
}

void SvgBuilder::setDocumentSize(double width, double height) {
    sp_repr_set_svg_double(root, "width", width);
    sp_repr_set_svg_double(root, "height", height);
    this->width = width;
    this->height = height;
}

void SvgBuilder::saveState() {
    groupDepth.push_back(0);
    pushGroup();
}

void SvgBuilder::restoreState() {
    int depth = groupDepth.back();
    while (groupDepth.back() > 0) {
        popGroup();
    }
    groupDepth.pop_back();
}

Inkscape::XML::Node *SvgBuilder::pushGroup() {
    Inkscape::XML::Node *node = xml_doc->createElement("svg:g");
    container->appendChild(node);
    container = node;
    Inkscape::GC::release(node);
    groupDepth.back()++;

    return container;
}

Inkscape::XML::Node *SvgBuilder::popGroup() {
    if (container != root) {  // Pop if the current container isn't root
        container = container->parent();
        groupDepth[groupDepth.size()-1] = --groupDepth.back();
    }

    return container;
}

Inkscape::XML::Node *SvgBuilder::getContainer() {
    return container;
}

static gchar *svgConvertRGBToText(double r, double g, double b) {
    static gchar tmp[1023] = {0};
    snprintf(tmp, 1023,
             "rgb(%i, %i, %i)",
             SP_COLOR_F_TO_U(r),
             SP_COLOR_F_TO_U(g),
             SP_COLOR_F_TO_U(b));
    return (gchar*)&tmp;
}

static gchar *svgConvertGfxRGB(GfxRGB *color) {
    double r = color->r / 65535.0;
    double g = color->g / 65535.0;
    double b = color->b / 65535.0;
    return svgConvertRGBToText(r, g, b);
}

static gchar *svgInterpretTransform(double c0, double c1, double c2, double c3,
                                    double c4, double c5) {
    NR::Matrix matrix(c0, c1, c2, c3, c4, c5);
    return sp_svg_transform_write(matrix);
}

static gchar *svgInterpretTransform(double *transform) {
    return svgInterpretTransform(transform[0], transform[1], transform[2],
                                 transform[3], transform[4], transform[5]);
}

static gchar *svgInterpretPath(GfxPath *path) {
    GfxSubpath *subpath;
    Inkscape::SVG::PathString pathString;
    int i, j;
    for (i = 0; i < path->getNumSubpaths(); ++i) {
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
void SvgBuilder::setStrokeStyle(SPCSSAttr *css, GfxState *state) {

    // Check line width
    if (state->getLineWidth() <= 0.0) {
        // Ignore stroke
        return;
    }

    // Stroke color/pattern
    if (state->getStrokeColorSpace()->getMode() == csPattern) {
        gchar *urltext = createPattern(state->getStrokePattern());
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
    if (dash_length > 0) {
        Inkscape::CSSOStringStream os_array;
        for (int i = 0; i < dash_length; i++) {
            os_array << dash_pattern[i];
            if (i < (dash_length - 1)) {
                os_array << ",";
            }
        }
        printf("aaaa %s\n", os_array.str().c_str());
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
void SvgBuilder::setFillStyle(SPCSSAttr *css, GfxState *state, bool even_odd) {

    // Fill color/pattern
    if (state->getFillColorSpace()->getMode() == csPattern) {
        gchar *urltext = createPattern(state->getFillPattern());
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
SPCSSAttr *SvgBuilder::setStyle(GfxState *state, bool fill, bool stroke, bool even_odd) {
    SPCSSAttr *css = sp_repr_css_attr_new();
    if (fill) {
        setFillStyle(css, state, even_odd);
    } else {
        sp_repr_css_set_property(css, "fill", "none");
    }
    
    if (stroke) {
        setStrokeStyle(css, state);
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
    Inkscape::XML::Node *path = xml_doc->createElement("svg:path");
    gchar *pathtext = svgInterpretPath(state->getPath());
    path->setAttribute("d", pathtext);
    g_free(pathtext);

    // Set style
    SPCSSAttr *css = setStyle(state, fill, stroke, even_odd);
    sp_repr_css_change(path, css, "style");
    sp_repr_css_attr_unref(css);

    container->appendChild(path);
    Inkscape::GC::release(path);
}

void SvgBuilder::clip(GfxState *state, bool even_odd) {
    pushGroup();
    setClipPath(state, even_odd);
}

void SvgBuilder::setClipPath(GfxState *state, bool even_odd) {
    // Create the clipPath repr
    Inkscape::XML::Node *clip_path = xml_doc->createElement("svg:clipPath");
    clip_path->setAttribute("clipPathUnits", "userSpaceOnUse");
    // Create the path
    Inkscape::XML::Node *path = xml_doc->createElement("svg:path");
    gchar *pathtext = svgInterpretPath(state->getPath());
    path->setAttribute("d", pathtext);
    g_free(pathtext);
    clip_path->appendChild(path);
    Inkscape::GC::release(path);
    // Append clipPath to defs and get id
    SP_OBJECT_REPR (SP_DOCUMENT_DEFS (doc))->appendChild(clip_path);
    gchar *urltext = g_strdup_printf ("url(#%s)", clip_path->attribute("id"));
    Inkscape::GC::release(clip_path);
    container->setAttribute("clip-path", urltext);
    g_free(urltext);
}

bool SvgBuilder::getTransform(double *transform) {
    NR::Matrix svd;
    gchar const *tr = container->attribute("transform");
    bool valid = sp_svg_transform_read(tr, &svd);
    if (valid) {
        for (unsigned i = 0; i < 6; i++) {
            transform[i] = svd[i];
        }
        return true;
    } else {
        return false;
    }
}

void SvgBuilder::setTransform(double c0, double c1, double c2, double c3,
                              double c4, double c5) {

    gchar *transform_text = svgInterpretTransform(c0, c1, c2, c3, c4, c5);
    LOG(g_message("setTransform: %f %f %f %f %f %f", c0, c1, c2, c3, c4, c5));
    container->setAttribute("transform", transform_text);
    g_free(transform_text);
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
    if (pattern != NULL) {
        if (pattern->getType() == 2) {    // shading pattern
            GfxShading *shading = ((GfxShadingPattern *)pattern)->getShading();
            int shadingType = shading->getType();
            if (shadingType == 2 || // axial shading
                shadingType == 3) {   // radial shading
                return true;
            }
            return false;
        } else if (pattern->getType() == 1) {   // tiling pattern
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
gchar *SvgBuilder::createPattern(GfxPattern *pattern) {
    gchar *id = NULL;
    if (pattern != NULL) {
        if (pattern->getType() == 2) {  // Shading pattern
            id = createGradient((GfxShadingPattern*)pattern);
        } else if (pattern->getType() == 1) {   // Tiling pattern
            id = createTilingPattern((GfxTilingPattern*)pattern);
        }
    } else {
        return NULL;
    }
    gchar *urltext = g_strdup_printf ("url(#%s)", id);
    g_free(id);
    return urltext;
}

gchar *SvgBuilder::createTilingPattern(GfxTilingPattern *tiling_pattern) {
    return NULL;
}

/**
 * \brief Creates a linear or radial gradient from poppler's data structure
 * \return id of the created object
 */
gchar *SvgBuilder::createGradient(GfxShadingPattern *shading_pattern) {
    GfxShading *shading = shading_pattern->getShading();
    Inkscape::XML::Node *gradient;
    Function *func;
    int num_funcs;
    bool extend0, extend1;
    if (shading->getType() == 2) {  // Axial shading
        gradient = xml_doc->createElement("svg:linearGradient");
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
        gradient = xml_doc->createElement("svg:radialGradient");
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
    NR::Matrix flip(1.0, 0.0, 0.0, -1.0, 0.0, height*PT_PER_PX);
    pat_matrix *= flip;
    gradient->setAttribute("gradientTransform", sp_svg_transform_write(pat_matrix));

    if (extend0 && extend1) {
        gradient->setAttribute("spreadMethod", "pad");
    }
    if (num_funcs > 1 || !addStopsToGradient(gradient, func, 1.0)) {
        Inkscape::GC::release(gradient);
        return NULL;
    }

    Inkscape::XML::Node *defs = SP_OBJECT_REPR (SP_DOCUMENT_DEFS (doc));
    defs->appendChild(gradient);
    gchar *id = g_strdup(gradient->attribute("id"));
    Inkscape::GC::release(gradient);

    return id;
}

#define EPSILON 0.0001
bool SvgBuilder::addSamplesToGradient(Inkscape::XML::Node *gradient,
                                      SampledFunction *func, double offset0,
                                      double offset1, double opacity) {

    // Check whether this sampled function can be converted to color stops
    int sample_size = func->getSampleSize(0);
    if (sample_size != 2)
        return false;
    int num_comps = func->getOutputSize();
    if (num_comps != 3)
        return false;

    double *samples = func->getSamples();
    // See if this is the continuation of the previous sampled function
    bool is_continuation = false;
    unsigned stop_count = gradient->childCount();
    if (stop_count > 0) {
        // Get previous stop
        Inkscape::XML::Node *prev_stop = gradient->nthChild(stop_count-1);

        // Read its properties
        double offset;
        sp_repr_get_double(prev_stop, "offset", &offset);
        SPCSSAttr *css = sp_repr_css_attr(prev_stop, "style");
        guint32 prev_color = sp_svg_read_color(sp_repr_css_property(css, "stop-color", NULL), 0);
        double alpha = sp_repr_css_double_property(css, "stop-opacity", 1.0);
        sp_repr_css_attr_unref(css);
        // Convert colors
        double r = SP_RGBA32_R_F(prev_color);
        double g = SP_RGBA32_G_F(prev_color);
        double b = SP_RGBA32_B_F(prev_color);

        if (fabs(offset - offset0) < EPSILON &&
            fabs(samples[0] - r) < EPSILON &&
            fabs(samples[1] - g) < EPSILON &&
            fabs(samples[2] - b) < EPSILON &&
            fabs(alpha - opacity) < EPSILON) {  // Identical

            is_continuation = true;
            LOG(g_message("gradient offsets merged"));
        }
    }

    int i = 0;
    if (is_continuation)
        i = num_comps;
    while (i < sample_size*num_comps) {
        Inkscape::XML::Node *stop = xml_doc->createElement("svg:stop");
        SPCSSAttr *css = sp_repr_css_attr_new();
        Inkscape::CSSOStringStream os_opacity;
        os_opacity << opacity;
        sp_repr_css_set_property(css, "stop-opacity", os_opacity.str().c_str());
        sp_repr_css_set_property(css, "stop-color",
                                 svgConvertRGBToText(samples[i], samples[i+1],
                                         samples[i+2]));
        sp_repr_css_change(stop, css, "style");
        sp_repr_css_attr_unref(css);
        sp_repr_set_css_double(stop, "offset", (i < num_comps) ? offset0 : offset1);

        gradient->appendChild(stop);
        Inkscape::GC::release(stop);
        i += num_comps;
    }
  
    return true;
}

bool SvgBuilder::addStopsToGradient(Inkscape::XML::Node *gradient, Function *func,
                                    double opacity) {
    
    int type = func->getType();
    if (type == 0) {  // Sampled
        SampledFunction *sampledFunc = (SampledFunction*)func;
        addSamplesToGradient(gradient, sampledFunc, 0.0, 1.0, opacity);
    } else if (type == 3) { // Stitching
        StitchingFunction *stitchingFunc = (StitchingFunction*)func;
        double *bounds = stitchingFunc->getBounds();
        // bounds might be outside of [0..1]!!!!
        double *encode = stitchingFunc->getEncode();
        // should check whether all of them are [0..1]
        int num_funcs = stitchingFunc->getNumFuncs();
        // Add samples from all the stitched functions
        for (int i=0; i<num_funcs; i++) {
            Function *func = stitchingFunc->getFunc(i);
            if (func->getType() != 0) // Only sampled functions are supported
                continue;
            
            LOG(g_message("t%i i%i o%i m%f m%f",
                   func->getType(),
                   func->getInputSize(), func->getOutputSize(),
                   func->getDomainMin(0), func->getDomainMax(0)));
            
            SampledFunction *sampledFunc = (SampledFunction*)func;
            addSamplesToGradient(gradient, sampledFunc, bounds[i],
                                 bounds[i+1], opacity);
        }
    } else { // Unsupported function type
        return false;
    }

    return true;
}

} } } /* namespace Inkscape, Extension, Internal, PdfInput */

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

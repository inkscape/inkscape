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
#include "display/nr-filter-utils.h"
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
 * \struct SvgTransparencyGroup
 * \brief Holds information about a PDF transparency group
 */
struct SvgTransparencyGroup {
    double bbox[6];
    Inkscape::XML::Node *container;

    bool isolated;
    bool knockout;
    bool for_softmask;

    SvgTransparencyGroup *next;
};

/**
 * \class SvgBuilder
 * 
 */

SvgBuilder::SvgBuilder(SPDocument *document, gchar *docname, XRef *xref) {
    _is_top_level = true;
    _doc = document;
    _docname = docname;
    _xref = xref;
    _xml_doc = sp_document_repr_doc(_doc);
    _container = _root = _doc->rroot;
    _root->setAttribute("xml:space", "preserve");
    _init();

    // Set default preference settings
    _preferences = _xml_doc->createElement("svgbuilder:prefs");
    _preferences->setAttribute("embedImages", "1");
}

SvgBuilder::SvgBuilder(SvgBuilder *parent, Inkscape::XML::Node *root) {
    _is_top_level = false;
    _doc = parent->_doc;
    _docname = parent->_docname;
    _xref = parent->_xref;
    _xml_doc = parent->_xml_doc;
    _preferences = parent->_preferences;
    _container = this->_root = root;
    _init();
}

SvgBuilder::~SvgBuilder() {
}

void SvgBuilder::_init() {
    _in_text_object = false;
    _need_font_update = true;
    _invalidated_style = true;
    _font_style = NULL;
    _current_font = NULL;
    _current_state = NULL;

    _transp_group_stack = NULL;
    SvgGraphicsState initial_state;
    initial_state.softmask = NULL;
    initial_state.group_depth = 0;
    _state_stack.push_back(initial_state);
    _node_stack.push_back(_container);
}

void SvgBuilder::setDocumentSize(double width, double height) {
    sp_repr_set_svg_double(_root, "width", width);
    sp_repr_set_svg_double(_root, "height", height);
    this->_width = width;
    this->_height = height;
}

/**
 * \brief Sets groupmode of the current container to 'layer' and sets its label if given
 */
void SvgBuilder::setAsLayer(char *layer_name) {
    _container->setAttribute("inkscape:groupmode", "layer");
    if (layer_name) {
        _container->setAttribute("inkscape:label", layer_name);
    }
}

/**
 * \brief Sets the current container's opacity
 */
void SvgBuilder::setGroupOpacity(double opacity) {
    sp_repr_set_svg_double(_container, "opacity", CLAMP(opacity, 0.0, 1.0));
}

void SvgBuilder::saveState() {
    SvgGraphicsState new_state;
    new_state.group_depth = 0;
    new_state.softmask = _state_stack.back().softmask;
    _state_stack.push_back(new_state);
    pushGroup();
}

void SvgBuilder::restoreState() {
    while( _state_stack.back().group_depth > 0 ) {
        popGroup();
    }
    _state_stack.pop_back();
}

Inkscape::XML::Node *SvgBuilder::pushNode(const char *name) {
    Inkscape::XML::Node *node = _xml_doc->createElement(name);
    _node_stack.push_back(node);
    _container = node;
    return node;
}

Inkscape::XML::Node *SvgBuilder::popNode() {
    Inkscape::XML::Node *node = NULL;
    if ( _node_stack.size() > 1 ) {
        node = _node_stack.back();
        _node_stack.pop_back();
        _container = _node_stack.back();    // Re-set container
    } else {
        TRACE(("popNode() called when stack is empty\n"));
        node = _root;
    }
    return node;
}

Inkscape::XML::Node *SvgBuilder::pushGroup() {
    Inkscape::XML::Node *saved_container = _container;
    Inkscape::XML::Node *node = pushNode("svg:g");
    saved_container->appendChild(node);
    Inkscape::GC::release(node);
    _state_stack.back().group_depth++;
    // Set as a layer if this is a top-level group
    if ( _container->parent() == _root && _is_top_level ) {
        static int layer_count = 1;
        if ( layer_count > 1 ) {
            gchar *layer_name = g_strdup_printf("%s%d", _docname, layer_count);
            setAsLayer(layer_name);
            g_free(layer_name);
        } else {
            setAsLayer(_docname);
        }
    }

    return _container;
}

Inkscape::XML::Node *SvgBuilder::popGroup() {
    if (_container != _root) {  // Pop if the current container isn't root
        popNode();
        _state_stack.back().group_depth--;
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
             NR::clamp(SP_COLOR_F_TO_U(r)),
             NR::clamp(SP_COLOR_F_TO_U(g)),
             NR::clamp(SP_COLOR_F_TO_U(b)));
    return (gchar *)&tmp;
}

static gchar *svgConvertGfxRGB(GfxRGB *color) {
    double r = (double)color->r / 65535.0;
    double g = (double)color->g / 65535.0;
    double b = (double)color->b / 65535.0;
    return svgConvertRGBToText(r, g, b);
}

static void svgSetTransform(Inkscape::XML::Node *node, double c0, double c1,
                            double c2, double c3, double c4, double c5) {
    Geom::Matrix matrix(c0, c1, c2, c3, c4, c5);
    gchar *transform_text = sp_svg_transform_write(matrix);
    node->setAttribute("transform", transform_text);
    g_free(transform_text);
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
 * \brief Emits the current path in poppler's GfxState data structure
 * The path is set to be filled with the given shading.
 */
void SvgBuilder::addShadedFill(GfxShading *shading, double *matrix, GfxPath *path,
                               bool even_odd) {

    Inkscape::XML::Node *path_node = _xml_doc->createElement("svg:path");
    gchar *pathtext = svgInterpretPath(path);
    path_node->setAttribute("d", pathtext);
    g_free(pathtext);

    // Set style
    SPCSSAttr *css = sp_repr_css_attr_new();
    gchar *id = _createGradient(shading, matrix, true);
    if (id) {
        gchar *urltext = g_strdup_printf ("url(#%s)", id);
        sp_repr_css_set_property(css, "fill", urltext);
        g_free(urltext);
        g_free(id);
    } else {
        sp_repr_css_attr_unref(css);
        Inkscape::GC::release(path_node);
        return;
    }
    if (even_odd) {
        sp_repr_css_set_property(css, "fill-rule", "evenodd");
    }
    sp_repr_css_set_property(css, "stroke", "none");
    sp_repr_css_change(path_node, css, "style");
    sp_repr_css_attr_unref(css);

    _container->appendChild(path_node);
    Inkscape::GC::release(path_node);

    // Remove the clipping path emitted before the 'sh' operator
    int up_walk = 0;
    Inkscape::XML::Node *node = _container->parent();
    while( node && node->childCount() == 1 && up_walk < 3 ) {
        gchar const *clip_path_url = node->attribute("clip-path");
        if (clip_path_url) {
            // Obtain clipping path's id from the URL
            gchar clip_path_id[32];
            strncpy(clip_path_id, clip_path_url + 5, strlen(clip_path_url) - 6);
            SPObject *clip_obj = _doc->getObjectById(clip_path_id);
            if (clip_obj) {
                clip_obj->deleteObject();
                node->setAttribute("clip-path", NULL);
                TRACE(("removed clipping path: %s\n", clip_path_id));
            }
            break;
        }
        node = node->parent();
        up_walk++;
    }
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
    if (even_odd) {
        path->setAttribute("clip-rule", "evenodd");
    }
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
    Geom::Matrix svd;
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

    // Avoid transforming a group with an already set clip-path
    if ( _container->attribute("clip-path") != NULL ) {
        pushGroup();
    }
    TRACE(("setTransform: %f %f %f %f %f %f\n", c0, c1, c2, c3, c4, c5));
    svgSetTransform(_container, c0, c1, c2, c3, c4, c5);
}

void SvgBuilder::setTransform(double const *transform) {
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
            GfxShadingPattern *shading_pattern = (GfxShadingPattern*)pattern;
            id = _createGradient(shading_pattern->getShading(),
                                 shading_pattern->getMatrix());
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
    Geom::Matrix pat_matrix(p2u[0], p2u[1], p2u[2], p2u[3], p2u[4], p2u[5]);
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
 * \param shading poppler's data structure for the shading
 * \param matrix gradient transformation, can be null
 * \param for_shading true if we're creating this for a shading operator; false otherwise
 * \return id of the created object
 */
gchar *SvgBuilder::_createGradient(GfxShading *shading, double *matrix, bool for_shading) {
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
    // If needed, flip the gradient transform around the y axis
    if (matrix) {
        Geom::Matrix pat_matrix(matrix[0], matrix[1], matrix[2], matrix[3],
                              matrix[4], matrix[5]);
        if ( !for_shading && _is_top_level ) {
            Geom::Matrix flip(1.0, 0.0, 0.0, -1.0, 0.0, _height * PT_PER_PX);
            pat_matrix *= flip;
        }
        gchar *transform_text = sp_svg_transform_write(pat_matrix);
        gradient->setAttribute("gradientTransform", transform_text);
        g_free(transform_text);
    }

    if ( extend0 && extend1 ) {
        gradient->setAttribute("spreadMethod", "pad");
    }

    if ( num_funcs > 1 || !_addGradientStops(gradient, shading, func) ) {
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
/**
 * \brief Adds a stop with the given properties to the gradient's representation
 */
void SvgBuilder::_addStopToGradient(Inkscape::XML::Node *gradient, double offset,
                                    GfxRGB *color, double opacity) {
    Inkscape::XML::Node *stop = _xml_doc->createElement("svg:stop");
    SPCSSAttr *css = sp_repr_css_attr_new();
    Inkscape::CSSOStringStream os_opacity;
    gchar *color_text = NULL;
    if ( _transp_group_stack != NULL && _transp_group_stack->for_softmask ) {
        double gray = (double)color->r / 65535.0;
        gray = CLAMP(gray, 0.0, 1.0);
        os_opacity << gray;
        color_text = (char*) "#ffffff";
    } else {
        os_opacity << opacity;
        color_text = svgConvertGfxRGB(color);
    }
    sp_repr_css_set_property(css, "stop-opacity", os_opacity.str().c_str());
    sp_repr_css_set_property(css, "stop-color", color_text);

    sp_repr_css_change(stop, css, "style");
    sp_repr_css_attr_unref(css);
    sp_repr_set_css_double(stop, "offset", offset);

    gradient->appendChild(stop);
    Inkscape::GC::release(stop);
}

static bool svgGetShadingColorRGB(GfxShading *shading, double offset, GfxRGB *result) {
    GfxColorSpace *color_space = shading->getColorSpace();
    GfxColor temp;
    if ( shading->getType() == 2 ) {  // Axial shading
        ((GfxAxialShading*)shading)->getColor(offset, &temp);
    } else if ( shading->getType() == 3 ) { // Radial shading
        ((GfxRadialShading*)shading)->getColor(offset, &temp);
    } else {
        return false;
    }
    // Convert it to RGB
    color_space->getRGB(&temp, result);

    return true;
}

#define INT_EPSILON 8
bool SvgBuilder::_addGradientStops(Inkscape::XML::Node *gradient, GfxShading *shading,
                                   Function *func) {
    int type = func->getType();
    if ( type == 0 || type == 2 ) {  // Sampled or exponential function
        GfxRGB stop1, stop2;
        if ( !svgGetShadingColorRGB(shading, 0.0, &stop1) ||
             !svgGetShadingColorRGB(shading, 1.0, &stop2) ) {
            return false;
        } else {
            _addStopToGradient(gradient, 0.0, &stop1, 1.0);
            _addStopToGradient(gradient, 1.0, &stop2, 1.0);
        }
    } else if ( type == 3 ) { // Stitching
        StitchingFunction *stitchingFunc = (StitchingFunction*)func;
        double *bounds = stitchingFunc->getBounds();
        int num_funcs = stitchingFunc->getNumFuncs();
        // Add stops from all the stitched functions
        for ( int i = 0 ; i < num_funcs ; i++ ) {
            GfxRGB color;
            svgGetShadingColorRGB(shading, bounds[i], &color);
            bool is_continuation = false;
            if ( i > 0 ) {  // Compare to previous stop
                GfxRGB prev_color;
                svgGetShadingColorRGB(shading, bounds[i-1], &prev_color);
                if ( abs(color.r - prev_color.r) < INT_EPSILON &&
                     abs(color.g - prev_color.g) < INT_EPSILON &&
                     abs(color.b - prev_color.b) < INT_EPSILON ) {
                    is_continuation = true;
                }
            }
            // Add stops
            if ( !is_continuation ) {
                _addStopToGradient(gradient, bounds[i], &color, 1.0);
            }
            if ( is_continuation || ( i == num_funcs - 1 ) ) {
                GfxRGB next_color;
                svgGetShadingColorRGB(shading, bounds[i+1], &next_color);
                _addStopToGradient(gradient, bounds[i+1], &next_color, 1.0);
            }
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
    {(char*) "bold",        (char*) "bold"},
    {(char*) "light",       (char*) "300"},
    {(char*) "black",       (char*) "900"},
    {(char*) "heavy",       (char*) "900"},
    {(char*) "ultrabold",   (char*) "800"},
    {(char*) "extrabold",   (char*) "800"},
    {(char*) "demibold",    (char*) "600"},
    {(char*) "semibold",    (char*) "600"},
    {(char*) "medium",      (char*) "500"},
    {(char*) "book",        (char*) "normal"},
    {(char*) "regular",     (char*) "normal"},
    {(char*) "roman",       (char*) "normal"},
    {(char*) "normal",      (char*) "normal"},
    {(char*) "ultralight",  (char*) "200"},
    {(char*) "extralight",  (char*) "200"},
    {(char*) "thin",        (char*) "100"}
};

/**
 * \brief Updates _font_style according to the font set in parameter state
 */
void SvgBuilder::updateFont(GfxState *state) {

    TRACE(("updateFont()\n"));
    _need_font_update = false;
    updateTextMatrix(state);    // Ensure that we have a text matrix built

    if (_font_style) {
        //sp_repr_css_attr_unref(_font_style);
    }
    _font_style = sp_repr_css_attr_new();
    GfxFont *font = state->getFont();
    // Store original name
    if (font->getOrigName()) {
        _font_specification = font->getOrigName()->getCString();
    } else if (font->getName()) {
        _font_specification = font->getName()->getCString();
    } else {
        _font_specification = (char*) "Arial";
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
    char *style_delim = NULL;
    if ( ( style_delim = g_strrstr(font_family, "-") ) ||
         ( style_delim = g_strrstr(font_family, ",") ) ) {
        font_style = style_delim + 1;
        font_style_lowercase = g_ascii_strdown(font_style, -1);
        style_delim[0] = 0;
    }

    // Font family
    if (font->getFamily()) {
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
    char *css_font_weight = NULL;
    if ( font_weight != GfxFont::WeightNotDefined ) {
        if ( font_weight == GfxFont::W400 ) {
            css_font_weight = (char*) "normal";
        } else if ( font_weight == GfxFont::W700 ) {
            css_font_weight = (char*) "bold";
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
                css_font_weight = font_weight_translator[i][1];
            }
        }
    } else {
        css_font_weight = (char*) "normal";
    }
    if (css_font_weight) {
        sp_repr_css_set_property(_font_style, "font-weight", css_font_weight);
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
            stretch_value = (char*) "ultra-condensed";
            break;
        case GfxFont::ExtraCondensed:
            stretch_value = (char*) "extra-condensed";
            break;
        case GfxFont::Condensed:
            stretch_value = (char*) "condensed";
            break;
        case GfxFont::SemiCondensed:
            stretch_value = (char*) "semi-condensed";
            break;
        case GfxFont::Normal:
            stretch_value = (char*) "normal";
            break;
        case GfxFont::SemiExpanded:
            stretch_value = (char*) "semi-expanded";
            break;
        case GfxFont::Expanded:
            stretch_value = (char*) "expanded";
            break;
        case GfxFont::ExtraExpanded:
            stretch_value = (char*) "extra-expanded";
            break;
        case GfxFont::UltraExpanded:
            stretch_value = (char*) "ultra-expanded";
            break;
        default:
            break;
    }
    if ( stretch_value != NULL ) {
        sp_repr_css_set_property(_font_style, "font-stretch", stretch_value);
    }

    // Font size
    Inkscape::CSSOStringStream os_font_size;
    double css_font_size = _font_scaling * state->getFontSize();
    if ( font->getType() == fontType3 ) {
        double *font_matrix = font->getFontMatrix();
        if ( font_matrix[0] != 0.0 ) {
            css_font_size *= font_matrix[3] / font_matrix[0];
        }
    }
    os_font_size << css_font_size;
    sp_repr_css_set_property(_font_style, "font-size", os_font_size.str().c_str());

    // Writing mode
    if ( font->getWMode() == 0 ) {
        sp_repr_css_set_property(_font_style, "writing-mode", "lr");
    } else {
        sp_repr_css_set_property(_font_style, "writing-mode", "tb");
    }

    _current_font = font;
    _invalidated_style = true;
}

/**
 * \brief Shifts the current text position by the given amount (specified in text space)
 */
void SvgBuilder::updateTextShift(GfxState *state, double shift) {
    double shift_value = -shift * 0.001 * fabs(state->getFontSize());
    if (state->getFont()->getWMode()) {
        _text_position[1] += shift_value;
    } else {
        _text_position[0] += shift_value;
    }
}

/**
 * \brief Updates current text position
 */
void SvgBuilder::updateTextPosition(double tx, double ty) {
    Geom::Point new_position(tx, ty);
    _text_position = new_position;
}

/**
 * \brief Flushes the buffered characters
 */
void SvgBuilder::updateTextMatrix(GfxState *state) {
    _flushText();
    // Update text matrix
    double *text_matrix = state->getTextMat();
    double w_scale = sqrt( text_matrix[0] * text_matrix[0] + text_matrix[2] * text_matrix[2] );
    double h_scale = sqrt( text_matrix[1] * text_matrix[1] + text_matrix[3] * text_matrix[3] );
    double max_scale;
    if ( w_scale > h_scale ) {
        max_scale = w_scale;
    } else {
        max_scale = h_scale;
    }
    // Calculate new text matrix
    Geom::Matrix new_text_matrix(text_matrix[0] * state->getHorizScaling(),
                               text_matrix[1] * state->getHorizScaling(),
                               -text_matrix[2], -text_matrix[3],
                               0.0, 0.0);

    if ( fabs( max_scale - 1.0 ) > EPSILON ) {
        // Cancel out scaling by font size in text matrix
        for ( int i = 0 ; i < 4 ; i++ ) {
            new_text_matrix[i] /= max_scale;
        }
    }
    _text_matrix = new_text_matrix;
    _font_scaling = max_scale;
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
    std::vector<SvgGlyph>::iterator i = _glyphs.begin();
    const SvgGlyph& first_glyph = (*i);
    int render_mode = first_glyph.render_mode;
    // Ignore invisible characters
    if ( render_mode == 3 ) {
        _glyphs.clear();
        return;
    }

    Inkscape::XML::Node *text_node = _xml_doc->createElement("svg:text");
    // Set text matrix
    Geom::Matrix text_transform(_text_matrix);
    text_transform[4] = first_glyph.position[0];
    text_transform[5] = first_glyph.position[1];
    gchar *transform = sp_svg_transform_write(text_transform);
    text_node->setAttribute("transform", transform);
    g_free(transform);

    bool new_tspan = true;
    bool same_coords[2] = {true, true};
    Geom::Point last_delta_pos;
    unsigned int glyphs_in_a_row = 0;
    Inkscape::XML::Node *tspan_node = NULL;
    Glib::ustring x_coords;
    Glib::ustring y_coords;
    Glib::ustring text_buffer;

    // Output all buffered glyphs
    while (1) {
        const SvgGlyph& glyph = (*i);
        std::vector<SvgGlyph>::iterator prev_iterator = i - 1;
        // Check if we need to make a new tspan
        if (glyph.style_changed) {
            new_tspan = true;
        } else if ( i != _glyphs.begin() ) {
            const SvgGlyph& prev_glyph = (*prev_iterator);
            if ( !( ( glyph.dy == 0.0 && prev_glyph.dy == 0.0 &&
                     glyph.text_position[1] == prev_glyph.text_position[1] ) ||
                    ( glyph.dx == 0.0 && prev_glyph.dx == 0.0 &&
                     glyph.text_position[0] == prev_glyph.text_position[0] ) ) ) {
                new_tspan = true;
            }
        }

        // Create tspan node if needed
        if ( new_tspan || i == _glyphs.end() ) {
            if (tspan_node) {
                // Set the x and y coordinate arrays
                if ( same_coords[0] ) {
                    sp_repr_set_svg_double(tspan_node, "x", last_delta_pos[0]);
                } else {
                    tspan_node->setAttribute("x", x_coords.c_str());
                }
                if ( same_coords[1] ) {
                    sp_repr_set_svg_double(tspan_node, "y", last_delta_pos[1]);
                } else {
                    tspan_node->setAttribute("y", y_coords.c_str());
                }
                TRACE(("tspan content: %s\n", text_buffer.c_str()));
                if ( glyphs_in_a_row > 1 ) {
                    tspan_node->setAttribute("sodipodi:role", "line");
                }
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
                glyphs_in_a_row = 0;
            }
            if ( i == _glyphs.end() ) {
                sp_repr_css_attr_unref((*prev_iterator).style);
                break;
            } else {
                tspan_node = _xml_doc->createElement("svg:tspan");
                
                ///////
                // Create a font specification string and save the attribute in the style
                PangoFontDescription *descr = pango_font_description_from_string(glyph.font_specification);
                Glib::ustring properFontSpec = font_factory::Default()->ConstructFontSpecification(descr);
                pango_font_description_free(descr);
                sp_repr_css_set_property(glyph.style, "-inkscape-font-specification", properFontSpec.c_str());
                
                // Set style and unref SPCSSAttr if it won't be needed anymore
                sp_repr_css_change(tspan_node, glyph.style, "style");
                if ( glyph.style_changed && i != _glyphs.begin() ) {    // Free previous style
                    sp_repr_css_attr_unref((*prev_iterator).style);
                }
            }
            new_tspan = false;
        }
        if ( glyphs_in_a_row > 0 ) {
            x_coords.append(" ");
            y_coords.append(" ");
            // Check if we have the same coordinates
            const SvgGlyph& prev_glyph = (*prev_iterator);
            for ( int p = 0 ; p < 2 ; p++ ) {
                if ( glyph.text_position[p] != prev_glyph.text_position[p] ) {
                    same_coords[p] = false;
                }
            }
        }
        // Append the coordinates to their respective strings
        Geom::Point delta_pos( glyph.text_position - first_glyph.text_position );
        delta_pos[1] += glyph.rise;
        delta_pos[1] *= -1.0;   // flip it
        delta_pos *= _font_scaling;
        Inkscape::CSSOStringStream os_x;
        os_x << delta_pos[0];
        x_coords.append(os_x.str());
        Inkscape::CSSOStringStream os_y;
        os_y << delta_pos[1];
        y_coords.append(os_y.str());
        last_delta_pos = delta_pos;

        // Append the character to the text buffer
        text_buffer.append((char *)&glyph.code, 1);

        glyphs_in_a_row++;
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
        Geom::Point delta(dx, dy);
         _text_position += delta;
         return;
    }
    // Allow only one space in a row
    if ( is_space && _glyphs[_glyphs.size() - 1].code_size == 1 &&
         _glyphs[_glyphs.size() - 1].code[0] == 32 ) {
        Geom::Point delta(dx, dy);
        _text_position += delta;
        return;
    }

    SvgGlyph new_glyph;
    new_glyph.is_space = is_space;
    new_glyph.position = Geom::Point( x - originX, y - originY );
    new_glyph.text_position = _text_position;
    new_glyph.dx = dx;
    new_glyph.dy = dy;
    Geom::Point delta(dx, dy);
    _text_position += delta;

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
    new_glyph.font_specification = _font_specification;
    new_glyph.rise = state->getRise();

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
    // Decide whether we should embed this image
    double attr_value = 1.0;
    sp_repr_get_double(_preferences, "embedImages", &attr_value);
    bool embed_image = ( attr_value != 0.0 );
    // Set read/write functions
    Inkscape::IO::StringOutputStream base64_string;
    Inkscape::IO::Base64OutputStream base64_stream(base64_string);
    FILE *fp = NULL;
    gchar *file_name = NULL;
    if (embed_image) {
        base64_stream.setColumnWidth(0);   // Disable line breaks
        png_set_write_fn(png_ptr, &base64_stream, png_write_base64stream, png_flush_base64stream);
    } else {
        static int counter = 0;
        file_name = g_strdup_printf("%s_img%d.png", _docname, counter++);
        fp = fopen(file_name, "wb");
        if ( fp == NULL ) {
            png_destroy_write_struct(&png_ptr, &info_ptr);
            g_free(file_name);
            return NULL;
        }
        png_init_io(png_ptr, fp);
    }

    // Set header data
    if ( !invert_alpha && !alpha_only ) {
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
        unsigned char *buffer = new unsigned char[width];
        int invert_bit = invert_alpha ? 1 : 0;
        for ( int y = 0 ; y < height ; y++ ) {
            unsigned char *row = image_stream->getLine();
            if (color_map) {
                color_map->getGrayLine(row, buffer, width);
            } else {
                unsigned char *buf_ptr = buffer;
                for ( int x = 0 ; x < width ; x++ ) {
                    if ( row[x] ^ invert_bit ) {
                        *buf_ptr++ = 0;
                    } else {
                        *buf_ptr++ = 255;
                    }
                }
            }
            png_write_row(png_ptr, (png_bytep)buffer);
        }
        delete buffer;
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
    sp_repr_set_svg_double(image_node, "width", 1);
    sp_repr_set_svg_double(image_node, "height", 1);
    // Set transformation
    if (_is_top_level) {
        svgSetTransform(image_node, 1.0, 0.0, 0.0, -1.0, 0.0, 1.0);
    }

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

/**
 * \brief Creates a <mask> with the specified width and height and adds to <defs>
 *  If we're not the top-level SvgBuilder, creates a <defs> too and adds the mask to it.
 * \return the created XML node
 */
Inkscape::XML::Node *SvgBuilder::_createMask(double width, double height) {
    Inkscape::XML::Node *mask_node = _xml_doc->createElement("svg:mask");
    mask_node->setAttribute("maskUnits", "userSpaceOnUse");
    sp_repr_set_svg_double(mask_node, "x", 0.0);
    sp_repr_set_svg_double(mask_node, "y", 0.0);
    sp_repr_set_svg_double(mask_node, "width", width);
    sp_repr_set_svg_double(mask_node, "height", height);
    // Append mask to defs
    if (_is_top_level) {
        SP_OBJECT_REPR (SP_DOCUMENT_DEFS (_doc))->appendChild(mask_node);
        Inkscape::GC::release(mask_node);
        return SP_OBJECT_REPR (SP_DOCUMENT_DEFS (_doc))->lastChild();
    } else {    // Work around for renderer bug when mask isn't defined in pattern
        static int mask_count = 0;
        Inkscape::XML::Node *defs = _root->firstChild();
        if ( !( defs && !strcmp(defs->name(), "svg:defs") ) ) {
            // Create <defs> node
            defs = _xml_doc->createElement("svg:defs");
            _root->addChild(defs, NULL);
            Inkscape::GC::release(defs);
            defs = _root->firstChild();
        }
        gchar *mask_id = g_strdup_printf("_mask%d", mask_count++);
        mask_node->setAttribute("id", mask_id);
        g_free(mask_id);
        defs->appendChild(mask_node);
        Inkscape::GC::release(mask_node);
        return defs->lastChild();
    }
}

void SvgBuilder::addImage(GfxState *state, Stream *str, int width, int height,
                          GfxImageColorMap *color_map, int *mask_colors) {

     Inkscape::XML::Node *image_node = _createImage(str, width, height, color_map, mask_colors);
     if (image_node) {
         _container->appendChild(image_node);
        Inkscape::GC::release(image_node);
     }
}

void SvgBuilder::addImageMask(GfxState *state, Stream *str, int width, int height,
                              bool invert) {

    // Create a rectangle
    Inkscape::XML::Node *rect = _xml_doc->createElement("svg:rect");
    sp_repr_set_svg_double(rect, "x", 0.0);
    sp_repr_set_svg_double(rect, "y", 0.0);
    sp_repr_set_svg_double(rect, "width", 1.0);
    sp_repr_set_svg_double(rect, "height", 1.0);
    svgSetTransform(rect, 1.0, 0.0, 0.0, -1.0, 0.0, 1.0);
    // Get current fill style and set it on the rectangle
    SPCSSAttr *css = sp_repr_css_attr_new();
    _setFillStyle(css, state, false);
    sp_repr_css_change(rect, css, "style");
    sp_repr_css_attr_unref(css);

    // Scaling 1x1 surfaces might not work so skip setting a mask with this size
    if ( width > 1 || height > 1 ) {
        Inkscape::XML::Node *mask_image_node = _createImage(str, width, height, NULL, NULL, true, invert);
        if (mask_image_node) {
            // Create the mask
            Inkscape::XML::Node *mask_node = _createMask(1.0, 1.0);
            // Remove unnecessary transformation from the mask image
            mask_image_node->setAttribute("transform", NULL);
            mask_node->appendChild(mask_image_node);
            Inkscape::GC::release(mask_image_node);
            gchar *mask_url = g_strdup_printf("url(#%s)", mask_node->attribute("id"));
            rect->setAttribute("mask", mask_url);
            g_free(mask_url);
        }
    }

    // Add the rectangle to the container
    _container->appendChild(rect);
    Inkscape::GC::release(rect);
}

void SvgBuilder::addMaskedImage(GfxState *state, Stream *str, int width, int height,
                                GfxImageColorMap *color_map,
                                Stream *mask_str, int mask_width, int mask_height,
                                bool invert_mask) {

    Inkscape::XML::Node *mask_image_node = _createImage(mask_str, mask_width, mask_height,
                                                        NULL, NULL, true, invert_mask);
    Inkscape::XML::Node *image_node = _createImage(str, width, height, color_map, NULL);
    if ( mask_image_node && image_node ) {
        // Create mask for the image
        Inkscape::XML::Node *mask_node = _createMask(1.0, 1.0);
        // Remove unnecessary transformation from the mask image
        mask_image_node->setAttribute("transform", NULL);
        mask_node->appendChild(mask_image_node);
        // Scale the mask to the size of the image
        Geom::Matrix mask_transform((double)width, 0.0, 0.0, (double)height, 0.0, 0.0);
        gchar *transform_text = sp_svg_transform_write(mask_transform);
        mask_node->setAttribute("maskTransform", transform_text);
        g_free(transform_text);
        // Set mask and add image
        gchar *mask_url = g_strdup_printf("url(#%s)", mask_node->attribute("id"));
        image_node->setAttribute("mask", mask_url);
        g_free(mask_url);
        _container->appendChild(image_node);
    }
    if (mask_image_node) {
        Inkscape::GC::release(mask_image_node);
    }
    if (image_node) {
        Inkscape::GC::release(image_node);
    }
}
    
void SvgBuilder::addSoftMaskedImage(GfxState *state, Stream *str, int width, int height,
                                    GfxImageColorMap *color_map,
                                    Stream *mask_str, int mask_width, int mask_height,
                                    GfxImageColorMap *mask_color_map) {

    Inkscape::XML::Node *mask_image_node = _createImage(mask_str, mask_width, mask_height,
                                                        mask_color_map, NULL, true);
    Inkscape::XML::Node *image_node = _createImage(str, width, height, color_map, NULL);
    if ( mask_image_node && image_node ) {
        // Create mask for the image
        Inkscape::XML::Node *mask_node = _createMask(1.0, 1.0);
        // Remove unnecessary transformation from the mask image
        mask_image_node->setAttribute("transform", NULL);
        mask_node->appendChild(mask_image_node);
        // Set mask and add image
        gchar *mask_url = g_strdup_printf("url(#%s)", mask_node->attribute("id"));
        image_node->setAttribute("mask", mask_url);
        g_free(mask_url);
        _container->appendChild(image_node);
    }
    if (mask_image_node) {
        Inkscape::GC::release(mask_image_node);
    }
    if (image_node) {
        Inkscape::GC::release(image_node);
    }
}

/**
 * \brief Starts building a new transparency group
 */
void SvgBuilder::pushTransparencyGroup(GfxState *state, double *bbox,
                                       GfxColorSpace *blending_color_space,
                                       bool isolated, bool knockout,
                                       bool for_softmask) {

    // Push node stack
    pushNode("svg:g");

    // Setup new transparency group
    SvgTransparencyGroup *transpGroup = new SvgTransparencyGroup;
    memcpy(&transpGroup->bbox, bbox, sizeof(bbox));
    transpGroup->isolated = isolated;
    transpGroup->knockout = knockout;
    transpGroup->for_softmask = for_softmask;
    transpGroup->container = _container;

    // Push onto the stack
    transpGroup->next = _transp_group_stack;
    _transp_group_stack = transpGroup;
}

void SvgBuilder::popTransparencyGroup(GfxState *state) {
    // Restore node stack
    popNode();
}

/**
 * \brief Places the current transparency group into the current container
 */
void SvgBuilder::paintTransparencyGroup(GfxState *state, double *bbox) {
    SvgTransparencyGroup *transpGroup = _transp_group_stack;
    _container->appendChild(transpGroup->container);
    Inkscape::GC::release(transpGroup->container);
    // Pop the stack
    _transp_group_stack = transpGroup->next;
    delete transpGroup;
}

/**
 * \brief Creates a mask using the current transparency group as its content
 */
void SvgBuilder::setSoftMask(GfxState *state, double *bbox, bool alpha,
                             Function *transfer_func, GfxColor *backdrop_color) {

    // Create mask
    Inkscape::XML::Node *mask_node = _createMask(1.0, 1.0);
    // Add the softmask content to it
    SvgTransparencyGroup *transpGroup = _transp_group_stack;
    mask_node->appendChild(transpGroup->container);
    Inkscape::GC::release(transpGroup->container);
    // Apply the mask
    _state_stack.back().softmask = mask_node;
    pushGroup();
    gchar *mask_url = g_strdup_printf("url(#%s)", mask_node->attribute("id"));
    _container->setAttribute("mask", mask_url);
    g_free(mask_url);
    // Pop the stack
    _transp_group_stack = transpGroup->next;
    delete transpGroup;
}

void SvgBuilder::clearSoftMask(GfxState *state) {
    if (_state_stack.back().softmask) {
        _state_stack.back().softmask = NULL;
        popGroup();
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

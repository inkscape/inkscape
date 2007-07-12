#ifndef __EXTENSION_INTERNAL_PDFINPUT_SVGBUILDER_H__
#define __EXTENSION_INTERNAL_PDFINPUT_SVGBUILDER_H__

 /** \file
 * SVG representation creator using libpoppler.
 *
 * Authors:
 *   miklos erdelyi
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_POPPLER

class SPDocument;
namespace Inkscape {
    namespace XML {
        class Document;
        class Node;
    }
}

class Function;
class SampledFunction;
struct GfxState;
class GfxPattern;
class GfxShadingPattern;
class GfxTilingPattern;

class SPCSSAttr;

#include <vector>
#include <glib/gtypes.h>

namespace Inkscape {
namespace Extension {
namespace Internal {

/**
 * \class SvgBuilder
 *
 * Builds the inner SVG representation from the calls of PdfParser
 *
 */
class SvgBuilder {
public:
    SvgBuilder(SPDocument *document);
    SvgBuilder(SvgBuilder *parent, Inkscape::XML::Node *root);
    ~SvgBuilder();

    // Property setting
    void setDocumentSize(double width, double height);  // Document size in px

    // Handling the node stack
    Inkscape::XML::Node *pushGroup();
    Inkscape::XML::Node *popGroup();
    Inkscape::XML::Node *getContainer();    // Returns current group node

    // Path adding
    void addPath(GfxState *state, bool fill, bool stroke, bool even_odd=false);

    bool isPatternTypeSupported(GfxPattern *pattern);

    // State manipulation
    void saveState();
    void restoreState();

    // Clipping
    void clip(GfxState *state, bool even_odd=false);
    void setClipPath(GfxState *state, bool even_odd=false);

    // Transforming
    void setTransform(double c0, double c1, double c2, double c3, double c4,
                      double c5);
    void setTransform(double *transform);
    bool getTransform(double *transform);

private:
    // Pattern creation
    gchar *createPattern(GfxPattern *pattern);
    gchar *createGradient(GfxShadingPattern *shading_pattern);
    bool addStopsToGradient(Inkscape::XML::Node *gradient, Function *func, double opacity);
    bool addSamplesToGradient(Inkscape::XML::Node *gradient, SampledFunction *func,
                              double offset0, double offset1, double opacity);
    gchar *createTilingPattern(GfxTilingPattern *tiling_pattern);
    // Style setting
    SPCSSAttr *setStyle(GfxState *state, bool fill, bool stroke, bool even_odd);
    void setStrokeStyle(SPCSSAttr *css, GfxState *state);
    void setFillStyle(SPCSSAttr *css, GfxState *state, bool even_odd);

    std::vector<int> groupDepth;    // Depth of nesting groups

    SPDocument *doc;
    Inkscape::XML::Document *xml_doc;
    Inkscape::XML::Node *root;  // Root node from the point of view of this SvgBuilder
    Inkscape::XML::Node *container; // Current container (group/pattern/mask)
    double width, height;       // Document size in px
};


} } } /* namespace Inkscape, Extension, Internal, PdfInput */

#endif /* HAVE_POPPLER */

#endif /* __EXTENSION_INTERNAL_PDFINPUT_SVGBUILDER_H__ */

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

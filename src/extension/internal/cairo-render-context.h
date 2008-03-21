#ifndef EXTENSION_INTERNAL_CAIRO_RENDER_CONTEXT_H_SEEN
#define EXTENSION_INTERNAL_CAIRO_RENDER_CONTEXT_H_SEEN

/** \file
 * Declaration of CairoRenderContext, a class used for rendering with Cairo.
 */
/*
 * Authors:
 * 	   Miklos Erdelyi <erdelyim@gmail.com>
 *
 * Copyright (C) 2006 Miklos Erdelyi
 *
 * Licensed under GNU GPL
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "extension/extension.h"
#include <set>
#include <string>

#include "libnr/nr-path.h"
#include <libnr/nr-matrix-ops.h>

#include "style.h"

#include <cairo.h>

class SPClipPath;
class SPMask;

namespace Inkscape {
namespace Extension {
namespace Internal {

class CairoRenderer;
class CairoRenderContext;
class CairoRenderState;
class CairoGlyphInfo;

// Holds info for rendering a glyph
struct CairoGlyphInfo {
    unsigned long index;
    double x;
    double y;
};

struct CairoRenderState {
    unsigned int merge_opacity : 1;     // whether fill/stroke opacity can be mul'd with item opacity
    unsigned int need_layer : 1;
    unsigned int has_overflow : 1;
    unsigned int parent_has_userspace : 1;  // whether the parent's ctm should be applied
    float opacity;
    bool has_filtereffect;

    SPClipPath *clip_path;
    SPMask* mask;

    NR::Matrix transform;     // the CTM
};

class CairoRenderContext {
    friend class CairoRenderer;
public:
    CairoRenderContext *cloneMe(void) const;
    CairoRenderContext *cloneMe(double width, double height) const;
    bool finish(void);

    CairoRenderer *getRenderer(void) const;
    cairo_t *getCairoContext(void) const;

    typedef enum CairoRenderMode {
        RENDER_MODE_NORMAL,
        RENDER_MODE_CLIP
    };

    typedef enum CairoClipMode {
        CLIP_MODE_PATH,
        CLIP_MODE_MASK
    };

    bool setImageTarget(cairo_format_t format);
    bool setPdfTarget(gchar const *utf8_fn);
    bool setPsTarget(gchar const *utf8_fn);
    /** Set the cairo_surface_t from an external source */
    bool setSurfaceTarget(cairo_surface_t *surface, bool is_vector);

    void setPSLevel(unsigned int level);
    unsigned int getPSLevel(void);
    void setPDFLevel(unsigned int level);
    void setTextToPath(bool texttopath);
    bool getTextToPath(void);
    void setFilterToBitmap(bool filtertobitmap);
    bool getFilterToBitmap(void);
    void setBitmapResolution(int resolution);
    int getBitmapResolution(void);

    /** Creates the cairo_surface_t for the context with the
    given width, height and with the currently set target
    surface type. */
    bool setupSurface(double width, double height);

    cairo_surface_t *getSurface(void);

    /** Saves the contents of the context to a PNG file. */
    bool saveAsPng(const char *file_name);

    /* Render/clip mode setting/query */
    void setRenderMode(CairoRenderMode mode);
    CairoRenderMode getRenderMode(void) const;
    void setClipMode(CairoClipMode mode);
    CairoClipMode getClipMode(void) const;

    void addBpath(NArtBpath const *bp);
    void setBpath(NArtBpath const *bp);

    void pushLayer(void);
    void popLayer(void);

    /* Graphics state manipulation */
    void pushState(void);
    void popState(void);
    CairoRenderState *getCurrentState(void) const;
    CairoRenderState *getParentState(void) const;
    void setStateForStyle(SPStyle const *style);

    void transform(NR::Matrix const *transform);
    void setTransform(NR::Matrix const *transform);
    void getTransform(NR::Matrix *copy) const;
    void getParentTransform(NR::Matrix *copy) const;

    /* Clipping methods */
    void addClipPath(NArtBpath const *bp, SPIEnum const *fill_rule);
    void addClippingRect(double x, double y, double width, double height);

    /* Rendering methods */
    bool renderPath(NRBPath const *bpath, SPStyle const *style, NRRect const *pbox);
    bool renderImage(unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
                     NR::Matrix const *image_transform, SPStyle const *style);
    bool renderGlyphtext(PangoFont *font, NR::Matrix const *font_matrix,
                         std::vector<CairoGlyphInfo> const &glyphtext, SPStyle const *style);

    /* More general rendering methods will have to be added (like fill, stroke) */

protected:
    CairoRenderContext(CairoRenderer *renderer);
    virtual ~CairoRenderContext(void);

    float _width;
    float _height;
    unsigned short _dpi;
    unsigned int _pdf_level;
    unsigned int _ps_level;
    bool _is_texttopath;
    bool _is_filtertobitmap;
    int _bitmapresolution;

    FILE *_stream;

    unsigned int _is_valid : 1;
    unsigned int _vector_based_target : 1;

    cairo_t *_cr;
    cairo_surface_t *_surface;
    cairo_surface_type_t _target;
    cairo_format_t _target_format;
    PangoLayout *_layout;

    unsigned int _clip_rule : 8;
    unsigned int _clip_winding_failed : 1;

    GSList *_state_stack;
    CairoRenderState *_state;    // the current state

    CairoRenderer *_renderer;

    CairoRenderMode _render_mode;
    CairoClipMode _clip_mode;

    cairo_pattern_t *_createPatternForPaintServer(SPPaintServer const *const paintserver,
                                                  NRRect const *pbox, float alpha);
    cairo_pattern_t *_createPatternPainter(SPPaintServer const *const paintserver, NRRect const *pbox);

    unsigned int _showGlyphs(cairo_t *cr, PangoFont *font, std::vector<CairoGlyphInfo> const &glyphtext, bool is_stroke);

    bool _finishSurfaceSetup(cairo_surface_t *surface);
    void _setFillStyle(SPStyle const *style, NRRect const *pbox);
    void _setStrokeStyle(SPStyle const *style, NRRect const *pbox);

    void _initCairoMatrix(cairo_matrix_t *matrix, NR::Matrix const *transform);
    void _concatTransform(cairo_t *cr, double xx, double yx, double xy, double yy, double x0, double y0);
    void _concatTransform(cairo_t *cr, NR::Matrix const *transform);

    CairoRenderState *_createState(void);
};

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* !EXTENSION_INTERNAL_CAIRO_RENDER_CONTEXT_H_SEEN */

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

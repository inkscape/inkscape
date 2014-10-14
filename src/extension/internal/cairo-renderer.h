#ifndef EXTENSION_INTERNAL_CAIRO_RENDERER_H_SEEN
#define EXTENSION_INTERNAL_CAIRO_RENDERER_H_SEEN

/** \file
 * Declaration of CairoRenderer, a class used for rendering via a CairoRenderContext.
 */
/*
 * Authors:
 * 	   Miklos Erdelyi <erdelyim@gmail.com>
 *   Abhishek Sharma
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

//#include "libnrtype/font-instance.h"
#include "style.h"
#include "sp-item.h"
#include <cairo.h>

class SPClipPath;
class SPMask;
class SPHatchPath;

namespace Inkscape {
namespace Extension {
namespace Internal {

class CairoRenderer;
class CairoRenderContext;

class CairoRenderer {
public:
    CairoRenderer(void);
    virtual ~CairoRenderer(void);
    
    CairoRenderContext *createContext(void);
    void destroyContext(CairoRenderContext *ctx);

    void setStateForItem(CairoRenderContext *ctx, SPItem const *item);

    void applyClipPath(CairoRenderContext *ctx, SPClipPath const *cp);
    void applyMask(CairoRenderContext *ctx, SPMask const *mask);

    /** Initializes the CairoRenderContext according to the specified
    SPDocument. A set*Target function can only be called on the context
    before setupDocument. */
    bool setupDocument(CairoRenderContext *ctx, SPDocument *doc, bool pageBoundingBox, float bleedmargin_px, SPItem *base);

    /** Traverses the object tree and invokes the render methods. */
    void renderItem(CairoRenderContext *ctx, SPItem *item);
    void renderHatchPath(CairoRenderContext *ctx, SPHatchPath const &hatchPath, unsigned key);
};

// FIXME: this should be a static method of CairoRenderer
void calculatePreserveAspectRatio(unsigned int aspect_align, unsigned int aspect_clip, double vp_width,
                                  double vp_height, double *x, double *y, double *width, double *height);

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* !EXTENSION_INTERNAL_CAIRO_RENDERER_H_SEEN */

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

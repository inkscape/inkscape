#define __SP_CAIRO_RENDERER_C__

/** \file
 * Rendering with Cairo.
 */
/*
 * Author:
 *   Miklos Erdelyi <erdelyim@gmail.com>
 *
 * Copyright (C) 2006 Miklos Erdelyi
 *
 * Licensed under GNU GPL
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef PANGO_ENABLE_BACKEND
#define PANGO_ENABLE_BACKEND
#endif

#ifndef PANGO_ENABLE_ENGINE
#define PANGO_ENABLE_ENGINE
#endif


#include <signal.h>
#include <errno.h>

#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-translate-ops.h>
#include <libnr/nr-scale-matrix-ops.h>

#include "libnr/nr-matrix-rotate-ops.h"
#include "libnr/nr-matrix-translate-ops.h"
#include "libnr/nr-rotate-fns.h"
#include "libnr/nr-scale-ops.h"
#include "libnr/nr-scale-translate-ops.h"
#include "libnr/nr-translate-matrix-ops.h"
#include "libnr/nr-translate-scale-ops.h"
#include "libnr/nr-convert2geom.h"
#include <2geom/transforms.h>
#include <2geom/pathvector.h>

#include <glib/gmem.h>

#include <glibmm/i18n.h>
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"
#include "display/nr-arena-group.h"
#include "display/curve.h"
#include "display/canvas-bpath.h"
#include "sp-item.h"
#include "sp-item-group.h"
#include "style.h"
#include "marker.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-root.h"
#include "sp-shape.h"
#include "sp-use.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "sp-image.h"
#include "sp-symbol.h"
#include "sp-pattern.h"
#include "sp-mask.h"
#include "sp-clippath.h"

#include <unit-constants.h>
#include "helper/png-write.h"
#include "helper/pixbuf-ops.h"

#include "cairo-renderer.h"
#include "cairo-render-context.h"
#include "extension/system.h"

#include "io/sys.h"

#include <cairo.h>

// include support for only the compiled-in surface types
#ifdef CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif
#ifdef CAIRO_HAS_PS_SURFACE
#include <cairo-ps.h>
#endif

//#define TRACE(_args) g_printf _args
#define TRACE(_args)

// FIXME: expose these from sp-clippath/mask.cpp
struct SPClipPathView {
    SPClipPathView *next;
    unsigned int key;
    NRArenaItem *arenaitem;
    NRRect bbox;
};

struct SPMaskView {
    SPMaskView *next;
    unsigned int key;
    NRArenaItem *arenaitem;
    NRRect bbox;
};

namespace Inkscape {
namespace Extension {
namespace Internal {

CairoRenderer::CairoRenderer(void)
{}

CairoRenderer::~CairoRenderer(void)
{
    /* restore default signal handling for SIGPIPE */
#if !defined(_WIN32) && !defined(__WIN32__)
    (void) signal(SIGPIPE, SIG_DFL);
#endif

    return;
}

CairoRenderContext*
CairoRenderer::createContext(void)
{
    CairoRenderContext *new_context = new CairoRenderContext(this);
    g_assert( new_context != NULL );

    new_context->_state_stack = NULL;
    new_context->_state = NULL;

    // create initial render state
    CairoRenderState *state = new_context->_createState();
    state->transform = Geom::identity();
    new_context->_state_stack = g_slist_prepend(new_context->_state_stack, state);
    new_context->_state = state;

    return new_context;
}

void
CairoRenderer::destroyContext(CairoRenderContext *ctx)
{
    delete ctx;
}

/*

Here comes the rendering part which could be put into the 'render' methods of SPItems'

*/

/* The below functions are copy&pasted plus slightly modified from *_invoke_print functions. */
static void sp_item_invoke_render(SPItem *item, CairoRenderContext *ctx);
static void sp_group_render(SPItem *item, CairoRenderContext *ctx);
static void sp_use_render(SPItem *item, CairoRenderContext *ctx);
static void sp_shape_render(SPItem *item, CairoRenderContext *ctx);
static void sp_text_render(SPItem *item, CairoRenderContext *ctx);
static void sp_flowtext_render(SPItem *item, CairoRenderContext *ctx);
static void sp_image_render(SPItem *item, CairoRenderContext *ctx);
static void sp_symbol_render(SPItem *item, CairoRenderContext *ctx);
static void sp_asbitmap_render(SPItem *item, CairoRenderContext *ctx);

/* TODO FIXME: this does not render painting-marker-01-f.svg of SVG1.1 Test suite correctly. (orientation of one of the markers middle left ) */
static void sp_shape_render (SPItem *item, CairoRenderContext *ctx)
{
    NRRect pbox;

    SPShape *shape = SP_SHAPE(item);

    if (!shape->curve) return;

    /* fixme: Think (Lauris) */
    sp_item_invoke_bbox(item, &pbox, Geom::identity(), TRUE);

    SPStyle* style = SP_OBJECT_STYLE (item);
    CairoRenderer *renderer = ctx->getRenderer();

    Geom::PathVector const & pathv = shape->curve->get_pathvector();

    ctx->renderPathVector(pathv, style, &pbox);

    /* TODO: make code prettier: lots of variables can be taken out of the loop! */
    for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
        if ( shape->marker[SP_MARKER_LOC_START] ) {
            SPMarker* marker = SP_MARKER (shape->marker[SP_MARKER_LOC_START]);
            SPItem* marker_item = sp_item_first_item_child (SP_OBJECT (shape->marker[SP_MARKER_LOC_START]));

            Geom::Matrix tr(sp_shape_marker_get_transform_at_start(path_it->front()));

            if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
                tr = Geom::Scale(style->stroke_width.computed) * tr;
            }

            tr = (Geom::Matrix)marker_item->transform * (Geom::Matrix)marker->c2p * tr;

            Geom::Matrix old_tr = marker_item->transform;
            marker_item->transform = tr;
            renderer->renderItem (ctx, marker_item);
            marker_item->transform = old_tr;
        }

        if ( shape->marker[SP_MARKER_LOC_MID] && (path_it->size_default() > 1) ) {
            Geom::Path::const_iterator curve_it1 = path_it->begin();      // incoming curve
            Geom::Path::const_iterator curve_it2 = ++(path_it->begin());  // outgoing curve
            while (curve_it2 != path_it->end_default())
            {
                /* Put marker between curve_it1 and curve_it2.
                 * Loop to end_default (so including closing segment), because when a path is closed,
                 * there should be a midpoint marker between last segment and closing straight line segment */

                SPMarker* marker = SP_MARKER (shape->marker[SP_MARKER_LOC_MID]);
                SPItem* marker_item = sp_item_first_item_child (SP_OBJECT (shape->marker[SP_MARKER_LOC_MID]));

                Geom::Matrix tr(sp_shape_marker_get_transform(*curve_it1, *curve_it2));

                if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
                    tr = Geom::Scale(style->stroke_width.computed) * tr;
                }

                tr = (Geom::Matrix)marker_item->transform * (Geom::Matrix)marker->c2p * tr;

                Geom::Matrix old_tr = marker_item->transform;
                marker_item->transform = tr;
                renderer->renderItem (ctx, marker_item);
                marker_item->transform = old_tr;

                ++curve_it1;
                ++curve_it2;
            }
        }

        if ( shape->marker[SP_MARKER_LOC_END] ) {
            SPMarker* marker = SP_MARKER (shape->marker[SP_MARKER_LOC_END]);
            SPItem* marker_item = sp_item_first_item_child (SP_OBJECT (shape->marker[SP_MARKER_LOC_END]));

            /* Get reference to last curve in the path.
             * For moveto-only path, this returns the "closing line segment". */
            unsigned int index = path_it->size_default();
            if (index > 0) {
                index--;
            }
            Geom::Curve const &lastcurve = (*path_it)[index];

            Geom::Matrix tr = sp_shape_marker_get_transform_at_end(lastcurve);

            if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
                tr = Geom::Scale(style->stroke_width.computed) * tr;
            }

            tr = (Geom::Matrix)marker_item->transform * (Geom::Matrix)marker->c2p * tr;

            Geom::Matrix old_tr = marker_item->transform;
            marker_item->transform = tr;
            renderer->renderItem (ctx, marker_item);
            marker_item->transform = old_tr;
        }
    }
}

static void sp_group_render(SPItem *item, CairoRenderContext *ctx)
{
    SPGroup *group = SP_GROUP(item);
    CairoRenderer *renderer = ctx->getRenderer();
    TRACE(("group op: %f\n", SP_SCALE24_TO_FLOAT(SP_OBJECT_STYLE(item)->opacity.value)));

    GSList *l = g_slist_reverse(group->childList(false));
    while (l) {
        SPObject *o = SP_OBJECT (l->data);
        if (SP_IS_ITEM(o)) {
            renderer->renderItem (ctx, SP_ITEM (o));
        }
        l = g_slist_remove (l, o);
    }
}

static void sp_use_render(SPItem *item, CairoRenderContext *ctx)
{
    bool translated = false;
    SPUse *use = SP_USE(item);
    CairoRenderer *renderer = ctx->getRenderer();

    if ((use->x._set && use->x.computed != 0) || (use->y._set && use->y.computed != 0)) {
        Geom::Matrix tp(Geom::Translate(use->x.computed, use->y.computed));
        ctx->pushState();
        ctx->transform(&tp);
        translated = true;
    }

    if (use->child && SP_IS_ITEM(use->child)) {
        renderer->renderItem(ctx, SP_ITEM(use->child));
    }

    if (translated) {
        ctx->popState();
    }
}

static void sp_text_render(SPItem *item, CairoRenderContext *ctx)
{
    SPText *group = SP_TEXT (item);
    group->layout.showGlyphs(ctx);
}

static void sp_flowtext_render(SPItem *item, CairoRenderContext *ctx)
{
    SPFlowtext *group = SP_FLOWTEXT(item);
    group->layout.showGlyphs(ctx);
}

static void sp_image_render(SPItem *item, CairoRenderContext *ctx)
{
    SPImage *image;
    guchar *px;
    int w, h, rs;

    image = SP_IMAGE (item);

    if (!image->pixbuf) return;
    if ((image->width.computed <= 0.0) || (image->height.computed <= 0.0)) return;

    px = gdk_pixbuf_get_pixels (image->pixbuf);
    w = gdk_pixbuf_get_width (image->pixbuf);
    h = gdk_pixbuf_get_height (image->pixbuf);
    rs = gdk_pixbuf_get_rowstride (image->pixbuf);

    double x = image->x.computed;
    double y = image->y.computed;
    double width = image->width.computed;
    double height = image->height.computed;

    if (image->aspect_align != SP_ASPECT_NONE) {
        calculatePreserveAspectRatio (image->aspect_align, image->aspect_clip, (double)w, (double)h,
                                                     &x, &y, &width, &height);
    }

    if (image->aspect_clip == SP_ASPECT_SLICE && !ctx->getCurrentState()->has_overflow) {
        ctx->addClippingRect(image->x.computed, image->y.computed, image->width.computed, image->height.computed);
    }

    Geom::Translate tp(x, y);
    Geom::Scale s(width / (double)w, height / (double)h);
    Geom::Matrix t(s * tp);

    ctx->renderImage (px, w, h, rs, &t, SP_OBJECT_STYLE (item));
}

static void sp_symbol_render(SPItem *item, CairoRenderContext *ctx)
{
    SPSymbol *symbol = SP_SYMBOL(item);
    if (!SP_OBJECT_IS_CLONED (symbol))
        return;

    /* Cloned <symbol> is actually renderable */
    ctx->pushState();
    ctx->transform(&symbol->c2p);

    // apply viewbox if set
    if (0 /*symbol->viewBox_set*/) {
        Geom::Matrix vb2user;
        double x, y, width, height;
        double view_width, view_height;
        x = 0.0;
        y = 0.0;
        width = 1.0;
        height = 1.0;

        view_width = symbol->viewBox.x1 - symbol->viewBox.x0;
        view_height = symbol->viewBox.y1 - symbol->viewBox.y0;

        calculatePreserveAspectRatio(symbol->aspect_align, symbol->aspect_clip, view_width, view_height,
                                     &x, &y,&width, &height);

        // [itemTransform *] translate(x, y) * scale(w/vw, h/vh) * translate(-vx, -vy);
        vb2user = Geom::identity();
        vb2user[0] = width / view_width;
        vb2user[3] = height / view_height;
        vb2user[4] = x - symbol->viewBox.x0 * vb2user[0];
        vb2user[5] = y - symbol->viewBox.y0 * vb2user[3];

        ctx->transform(&vb2user);
    }

    sp_group_render(item, ctx);
    ctx->popState();
}

static void sp_root_render(SPItem *item, CairoRenderContext *ctx)
{
    SPRoot *root = SP_ROOT(item);
    CairoRenderer *renderer = ctx->getRenderer();

    if (!ctx->getCurrentState()->has_overflow && SP_OBJECT(item)->parent)
        ctx->addClippingRect(root->x.computed, root->y.computed, root->width.computed, root->height.computed);

    ctx->pushState();
    renderer->setStateForItem(ctx, item);
    Geom::Matrix tempmat (root->c2p);
    ctx->transform(&tempmat);
    sp_group_render(item, ctx);
    ctx->popState();
}

/**
    this function convert the item to a raster image and include the raster into the cairo renderer
*/
static void sp_asbitmap_render(SPItem *item, CairoRenderContext *ctx)
{
    //the code now was copied from sp_selection_create_bitmap_copy

    SPDocument *document = SP_OBJECT(item)->document;
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(document);

    // Get the bounding box of the selection
    //boost::optional<Geom::Rect> _bbox = item->getBounds(sp_item_i2d_affine(item));
    // NRRect bbox = item->getBounds(sp_item_i2d_affine(item));
    NRRect bbox(item->getBounds(sp_item_i2d_affine(item)));


    // List of the items to show; all others will be hidden
    GSList *items = NULL; //g_slist_copy ((GSList *) selection->itemList());
    items = g_slist_append(items, item);

    // Remember parent and z-order of the topmost one
    gint pos = SP_OBJECT_REPR(g_slist_last(items)->data)->position();
    SPObject *parent_object = SP_OBJECT_PARENT(g_slist_last(items)->data);
    Inkscape::XML::Node *parent = SP_OBJECT_REPR(parent_object);

    // Calculate resolution
    double res;
    /** @TODO reimplement the resolution stuff
    */
    res = ctx->getBitmapResolution();
    if(res == 0) {
        res = PX_PER_IN;
    }


    // The width and height of the bitmap in pixels
    unsigned width = (unsigned) floor ((bbox.x1 - bbox.x0) * (res / PX_PER_IN));
    unsigned height =(unsigned) floor ((bbox.y1 - bbox.y0) * (res / PX_PER_IN));
    
    double scale_x = (bbox.x1 - bbox.x0) / width;
    double scale_y = (bbox.y1 - bbox.y0) / height;
    

    // Find out if we have to run a filter
    gchar const *run = NULL;
    gchar const *filter = NULL;
    /** @TODO reimplement the filter stuff
    //gchar const *filter = prefs_get_string_attribute ("options.createbitmap", "filter");
    if (filter) {
        // filter command is given;
        // see if we have a parameter to pass to it
        gchar const *param1 = prefs_get_string_attribute ("options.createbitmap", "filter_param1");
        if (param1) {
            if (param1[strlen(param1) - 1] == '%') {
                // if the param string ends with %, interpret it as a percentage of the image's max dimension
                gchar p1[256];
                g_ascii_dtostr (p1, 256, ceil (g_ascii_strtod (param1, NULL) * MAX(width, height) / 100));
                // the first param is always the image filename, the second is param1
                run = g_strdup_printf ("%s \"%s\" %s", filter, filepath, p1);
            } else {
                // otherwise pass the param1 unchanged
                run = g_strdup_printf ("%s \"%s\" %s", filter, filepath, param1);
            }
        } else {
            // run without extra parameter
            run = g_strdup_printf ("%s \"%s\"", filter, filepath);
        }
    }
    */
    // Calculate the matrix that will be applied to the image so that it exactly overlaps the source objects
    Geom::Matrix eek = sp_item_i2d_affine (SP_ITEM(parent_object));
    Geom::Matrix t;

    double shift_x = bbox.x0;
    double shift_y = bbox.y1;
    if (res == PX_PER_IN) { // for default 90 dpi, snap it to pixel grid
        shift_x = round (shift_x);
        shift_y = -round (-shift_y); // this gets correct rounding despite coordinate inversion, remove the negations when the inversion is gone
    }
    t = (Geom::Matrix)(Geom::Scale(scale_x, -scale_y) * (Geom::Matrix)(Geom::Translate (shift_x, shift_y)* eek.inverse()));

    //t = t * ((Geom::Matrix)ctx->getCurrentState()->transform).inverse();

    // Do the export
    GdkPixbuf *pb = sp_generate_internal_bitmap(document, NULL,
        bbox.x0, bbox.y0, bbox.x1, bbox.y1, width, height, res, res, (guint32) 0xffffff00, items );

    // Run filter, if any
    /*
    if (run) {
        g_print ("Running external filter: %s\n", run);
        system (run);
    }
    */
    if (pb) {
        unsigned char *px = gdk_pixbuf_get_pixels (pb);
        unsigned int w = gdk_pixbuf_get_width(pb);
        unsigned int h = gdk_pixbuf_get_height(pb);
        unsigned int rs = gdk_pixbuf_get_rowstride(pb);
        Geom::Matrix matrix;
        matrix = t;
        //matrix = ((Geom::Matrix)ctx->getCurrentState()->transform).inverse();
        //matrix.set_identity();

        ctx->renderImage (px, w, h, rs, &matrix, SP_OBJECT_STYLE (item));
    /*
        // Create the repr for the image
        Inkscape::XML::Node * repr = xml_doc->createElement("svg:image");
        repr->setAttribute("xlink:href", filename);
        repr->setAttribute("sodipodi:absref", filepath);
        if (res == PX_PER_IN) { // for default 90 dpi, snap it to pixel grid
            sp_repr_set_svg_double(repr, "width", width);
            sp_repr_set_svg_double(repr, "height", height);
        } else {
            sp_repr_set_svg_double(repr, "width", (bbox.x1 - bbox.x0));
            sp_repr_set_svg_double(repr, "height", (bbox.y1 - bbox.y0));
        }

        // Write transform
        gchar *c=sp_svg_transform_write(t);
        repr->setAttribute("transform", c);
        g_free(c);

        // add the new repr to the parent
        parent->appendChild(repr);

        // move to the saved position
        repr->setPosition(pos > 0 ? pos + 1 : 1);
    */
        gdk_pixbuf_unref (pb);
    }
    g_slist_free (items);
}

static void sp_item_invoke_render(SPItem *item, CairoRenderContext *ctx)
{
    // Check item's visibility
    if (item->isHidden()) {
        return;
    }

    SPStyle* style = SP_OBJECT_STYLE (item);
    if((ctx->getFilterToBitmap() == TRUE) && (style->filter.set != 0)) {
        return sp_asbitmap_render(item, ctx);
    }

    if (SP_IS_ROOT(item)) {
        TRACE(("root\n"));
        return sp_root_render(item, ctx);
    } else if (SP_IS_GROUP(item)) {
        TRACE(("group\n"));
        return sp_group_render(item, ctx);
    } else if (SP_IS_SHAPE(item)) {
        TRACE(("shape\n"));
        return sp_shape_render(item, ctx);
    } else if (SP_IS_USE(item)) {
        TRACE(("use begin---\n"));
        sp_use_render(item, ctx);
        TRACE(("---use end\n"));
    } else if (SP_IS_SYMBOL(item)) {
        TRACE(("symbol\n"));
        return sp_symbol_render(item, ctx);
    } else if (SP_IS_TEXT(item)) {
        TRACE(("text\n"));
        return sp_text_render(item, ctx);
    } else if (SP_IS_FLOWTEXT(item)) {
        TRACE(("flowtext\n"));
        return sp_flowtext_render(item, ctx);
    } else if (SP_IS_IMAGE(item)) {
        TRACE(("image\n"));
        return sp_image_render(item, ctx);
    }
}

void
CairoRenderer::setStateForItem(CairoRenderContext *ctx, SPItem const *item)
{
    SPStyle const *style = SP_OBJECT_STYLE(item);
    ctx->setStateForStyle(style);

    CairoRenderState *state = ctx->getCurrentState();
    state->clip_path = item->clip_ref->getObject();
    state->mask = item->mask_ref->getObject();
    state->item_transform = Geom::Matrix (item->transform);

    // If parent_has_userspace is true the parent state's transform
    // has to be used for the mask's/clippath's context.
    // This is so because we use the image's/(flow)text's transform for positioning
    // instead of explicitly specifying it and letting the renderer do the
    // transformation before rendering the item.
    if (SP_IS_TEXT(item) || SP_IS_FLOWTEXT(item) || SP_IS_IMAGE(item))
        state->parent_has_userspace = TRUE;
    TRACE(("set op: %f\n", state->opacity));
}

void
CairoRenderer::renderItem(CairoRenderContext *ctx, SPItem *item)
{
    ctx->pushState();
    setStateForItem(ctx, item);

    CairoRenderState *state = ctx->getCurrentState();
    state->need_layer = ( state->mask || state->clip_path || state->opacity != 1.0 );

    if (state->need_layer) {
        state->merge_opacity = FALSE;
        ctx->pushLayer();
    }
    Geom::Matrix tempmat (item->transform);
    ctx->transform(&tempmat);
    sp_item_invoke_render(item, ctx);

    if (state->need_layer)
        ctx->popLayer();

    ctx->popState();
}

bool
CairoRenderer::setupDocument(CairoRenderContext *ctx, SPDocument *doc, bool pageBoundingBox, SPItem *base)
{
    g_assert( ctx != NULL );

    if (ctx->_vector_based_target) {
        // width and height in pt
        ctx->_width = sp_document_width(doc) * PT_PER_PX;
        ctx->_height = sp_document_height(doc) * PT_PER_PX;
    } else {
        ctx->_width = sp_document_width(doc);
        ctx->_height = sp_document_height(doc);
    }

    NRRect d;
    if (pageBoundingBox || !base) {
        d.x0 = d.y0 = 0;
        d.x1 = ceil(ctx->_width);
        d.y1 = ceil(ctx->_height);
    } else {
        sp_item_invoke_bbox(base, &d, sp_item_i2r_affine(base), TRUE);
        if (ctx->_vector_based_target) {
            // convert from px to pt
            d.x0 *= PT_PER_PX;
            d.x1 *= PT_PER_PX;
            d.y0 *= PT_PER_PX;
            d.y1 *= PT_PER_PX;
        }
    }
    TRACE(("%f x %f\n", ctx->_width, ctx->_height));

    bool ret = ctx->setupSurface(d.x1-d.x0, d.y1-d.y0);

    if (ret && !pageBoundingBox && base)
    {
        Geom::Matrix tp(Geom::Translate(-d.x0 * (ctx->_vector_based_target ? PX_PER_PT : 1.0),
                                    (d.y1 - ctx->_height) * (ctx->_vector_based_target ? PX_PER_PT : 1.0)));
        ctx->transform(&tp);

        ctx->_width  = d.x1 - d.x0;
        ctx->_height = d.y1 - d.y0;
    }
    
    return ret;
}

#include "macros.h" // SP_PRINT_*

void
CairoRenderer::applyClipPath(CairoRenderContext *ctx, SPClipPath const *cp)
{
    g_assert( ctx != NULL && ctx->_is_valid );

    if (cp == NULL)
        return;

    CairoRenderContext::CairoRenderMode saved_mode = ctx->getRenderMode();
    ctx->setRenderMode(CairoRenderContext::RENDER_MODE_CLIP);

    Geom::Matrix saved_ctm;
    if (cp->clipPathUnits == SP_CONTENT_UNITS_OBJECTBOUNDINGBOX) {
        //SP_PRINT_DRECT("clipd", cp->display->bbox);
        NRRect clip_bbox(cp->display->bbox);
        Geom::Matrix t(Geom::Scale(clip_bbox.x1 - clip_bbox.x0, clip_bbox.y1 - clip_bbox.y0));
        t[4] = clip_bbox.x0;
        t[5] = clip_bbox.y0;
        t *= ctx->getCurrentState()->transform;
        ctx->getTransform(&saved_ctm);
        ctx->setTransform(&t);
    }

    TRACE(("BEGIN clip\n"));
    SPObject *co = SP_OBJECT(cp);
    for (SPObject *child = sp_object_first_child(co) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_ITEM(child)) {
            SPItem *item = SP_ITEM(child);

            // combine transform of the item in clippath and the item using clippath:
            Geom::Matrix tempmat (item->transform);
            tempmat = tempmat * (ctx->getCurrentState()->item_transform);

            // render this item in clippath
            ctx->pushState();
            ctx->transform(&tempmat);
            setStateForItem(ctx, item);
            sp_item_invoke_render(item, ctx);
            ctx->popState();
        }
    }
    TRACE(("END clip\n"));

    // do clipping only if this was the first call to applyClipPath
    if (ctx->getClipMode() == CairoRenderContext::CLIP_MODE_PATH
        && saved_mode == CairoRenderContext::RENDER_MODE_NORMAL)
        cairo_clip(ctx->_cr);

    if (cp->clipPathUnits == SP_CONTENT_UNITS_OBJECTBOUNDINGBOX)
        ctx->setTransform(&saved_ctm);

    ctx->setRenderMode(saved_mode);
}

void
CairoRenderer::applyMask(CairoRenderContext *ctx, SPMask const *mask)
{
    g_assert( ctx != NULL && ctx->_is_valid );

    if (mask == NULL)
        return;

    //SP_PRINT_DRECT("maskd", &mask->display->bbox);
    NRRect mask_bbox(mask->display->bbox);
    // TODO: should the bbox be transformed if maskUnits != userSpaceOnUse ?
    if (mask->maskContentUnits == SP_CONTENT_UNITS_OBJECTBOUNDINGBOX) {
        Geom::Matrix t(Geom::Scale(mask_bbox.x1 - mask_bbox.x0, mask_bbox.y1 - mask_bbox.y0));
        t[4] = mask_bbox.x0;
        t[5] = mask_bbox.y0;
        t *= ctx->getCurrentState()->transform;
        ctx->setTransform(&t);
    }

    // clip mask contents
    ctx->addClippingRect(mask_bbox.x0, mask_bbox.y0, mask_bbox.x1 - mask_bbox.x0, mask_bbox.y1 - mask_bbox.y0);

    ctx->pushState();

    TRACE(("BEGIN mask\n"));
    SPObject *co = SP_OBJECT(mask);
    for (SPObject *child = sp_object_first_child(co) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_ITEM(child)) {
            SPItem *item = SP_ITEM(child);
            renderItem(ctx, item);
        }
    }
    TRACE(("END mask\n"));

    ctx->popState();
}

void
calculatePreserveAspectRatio(unsigned int aspect_align, unsigned int aspect_clip, double vp_width, double vp_height,
                             double *x, double *y, double *width, double *height)
{
    if (aspect_align == SP_ASPECT_NONE)
        return;

    double scalex, scaley, scale;
    double new_width, new_height;
    scalex = *width / vp_width;
    scaley = *height / vp_height;
    scale = (aspect_clip == SP_ASPECT_MEET) ? MIN(scalex, scaley) : MAX(scalex, scaley);
    new_width = vp_width * scale;
    new_height = vp_height * scale;
    /* Now place viewbox to requested position */
    switch (aspect_align) {
        case SP_ASPECT_XMIN_YMIN:
            break;
        case SP_ASPECT_XMID_YMIN:
            *x -= 0.5 * (new_width - *width);
            break;
        case SP_ASPECT_XMAX_YMIN:
            *x -= 1.0 * (new_width - *width);
            break;
        case SP_ASPECT_XMIN_YMID:
            *y -= 0.5 * (new_height - *height);
            break;
        case SP_ASPECT_XMID_YMID:
            *x -= 0.5 * (new_width - *width);
            *y -= 0.5 * (new_height - *height);
            break;
        case SP_ASPECT_XMAX_YMID:
            *x -= 1.0 * (new_width - *width);
            *y -= 0.5 * (new_height - *height);
            break;
        case SP_ASPECT_XMIN_YMAX:
            *y -= 1.0 * (new_height - *height);
            break;
        case SP_ASPECT_XMID_YMAX:
            *x -= 0.5 * (new_width - *width);
            *y -= 1.0 * (new_height - *height);
            break;
        case SP_ASPECT_XMAX_YMAX:
            *x -= 1.0 * (new_width - *width);
            *y -= 1.0 * (new_height - *height);
            break;
        default:
            break;
    }
    *width = new_width;
    *height = new_height;
}

#include "clear-n_.h"

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

#undef TRACE

/* End of GNU GPL code */

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

#define __NR_ARENA_GLYPHS_C__

/*
 * RGBA display list system for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 *
 */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <libnr/nr-blit.h>
#include <libnr/nr-path.h>
#include <libnr/n-art-bpath.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-convert2geom.h>
#include <2geom/matrix.h>
#include "../style.h"
#include "nr-arena.h"
#include "nr-arena-glyphs.h"
#include <cairo.h>
#include "inkscape-cairo.h"

#ifdef test_glyph_liv
#include "../display/canvas-bpath.h"
#include <libnrtype/font-instance.h>
#include <libnrtype/raster-glyph.h>
#include <libnrtype/RasterFont.h>

// defined in nr-arena-shape.cpp
void nr_pixblock_render_shape_mask_or(NRPixBlock &m, Shape *theS);
#endif

#ifdef ENABLE_SVG_FONTS
#include "nr-svgfonts.h"
#endif //#ifdef ENABLE_SVG_FONTS

static void nr_arena_glyphs_class_init(NRArenaGlyphsClass *klass);
static void nr_arena_glyphs_init(NRArenaGlyphs *glyphs);
static void nr_arena_glyphs_finalize(NRObject *object);

static guint nr_arena_glyphs_update(NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset);
static guint nr_arena_glyphs_clip(NRArenaItem *item, NRRectL *area, NRPixBlock *pb);
static NRArenaItem *nr_arena_glyphs_pick(NRArenaItem *item, NR::Point p, double delta, unsigned int sticky);

static NRArenaItemClass *glyphs_parent_class;

NRType
nr_arena_glyphs_get_type(void)
{
    static NRType type = 0;
    if (!type) {
        type = nr_object_register_type(NR_TYPE_ARENA_ITEM,
                                       "NRArenaGlyphs",
                                       sizeof(NRArenaGlyphsClass),
                                       sizeof(NRArenaGlyphs),
                                       (void (*)(NRObjectClass *)) nr_arena_glyphs_class_init,
                                       (void (*)(NRObject *)) nr_arena_glyphs_init);
    }
    return type;
}

static void
nr_arena_glyphs_class_init(NRArenaGlyphsClass *klass)
{
    NRObjectClass *object_class;
    NRArenaItemClass *item_class;

    object_class = (NRObjectClass *) klass;
    item_class = (NRArenaItemClass *) klass;

    glyphs_parent_class = (NRArenaItemClass *) ((NRObjectClass *) klass)->parent;

    object_class->finalize = nr_arena_glyphs_finalize;
    object_class->cpp_ctor = NRObject::invoke_ctor<NRArenaGlyphs>;

    item_class->update = nr_arena_glyphs_update;
    item_class->clip = nr_arena_glyphs_clip;
    item_class->pick = nr_arena_glyphs_pick;
}

static void
nr_arena_glyphs_init(NRArenaGlyphs *glyphs)
{
    glyphs->style = NULL;
    glyphs->g_transform.set_identity();
    glyphs->font = NULL;
    glyphs->glyph = 0;

    glyphs->rfont = NULL;
    glyphs->sfont = NULL;
    glyphs->x = glyphs->y = 0.0;
}

static void
nr_arena_glyphs_finalize(NRObject *object)
{
    NRArenaGlyphs *glyphs = static_cast<NRArenaGlyphs *>(object);

    if (glyphs->rfont) {
        glyphs->rfont->Unref();
        glyphs->rfont=NULL;
    }
    if (glyphs->sfont) {
        glyphs->sfont->Unref();
        glyphs->sfont=NULL;
    }

    if (glyphs->font) {
        glyphs->font->Unref();
        glyphs->font=NULL;
    }

    if (glyphs->style) {
        sp_style_unref(glyphs->style);
        glyphs->style = NULL;
    }

    ((NRObjectClass *) glyphs_parent_class)->finalize(object);
}

static guint
nr_arena_glyphs_update(NRArenaItem *item, NRRectL */*area*/, NRGC *gc, guint /*state*/, guint /*reset*/)
{
    NRArenaGlyphs *glyphs;
    raster_font *rfont;

    glyphs = NR_ARENA_GLYPHS(item);

    if (!glyphs->font || !glyphs->style)
        return NR_ARENA_ITEM_STATE_ALL;
    if ((glyphs->style->fill.isNone()) && (glyphs->style->stroke.isNone()))
        return NR_ARENA_ITEM_STATE_ALL;

    NRRect bbox;
    bbox.x0 = bbox.y0 = NR_HUGE;
    bbox.x1 = bbox.y1 = -NR_HUGE;

    float const scale = NR::expansion(gc->transform);

    if (!glyphs->style->fill.isNone()) {
        NR::Matrix t;
        t = glyphs->g_transform * gc->transform;
        glyphs->x = t[4];
        glyphs->y = t[5];
        t[4]=0;
        t[5]=0;
        rfont = glyphs->font->RasterFont(t, 0);
        if (glyphs->rfont) glyphs->rfont->Unref();
        glyphs->rfont = rfont;

        if (glyphs->style->stroke.isNone() || fabs(glyphs->style->stroke_width.computed * scale) <= 0.01) { // Optimization: do fill bbox only if there's no stroke
            NRRect narea;
            if ( glyphs->rfont ) glyphs->rfont->BBox(glyphs->glyph, &narea);
            bbox.x0 = narea.x0 + glyphs->x;
            bbox.y0 = narea.y0 + glyphs->y;
            bbox.x1 = narea.x1 + glyphs->x;
            bbox.y1 = narea.y1 + glyphs->y;
        }
    }

    if (!glyphs->style->stroke.isNone()) {
        /* Build state data */
        NR::Matrix t;
        t = glyphs->g_transform * gc->transform;
        glyphs->x = t[4];
        glyphs->y = t[5];
        t[4]=0;
        t[5]=0;

        if ( fabs(glyphs->style->stroke_width.computed * scale) > 0.01 ) { // sinon c'est 0=oon veut pas de bord
            font_style nstyl;
            nstyl.transform = t;
            nstyl.stroke_width=MAX(0.125, glyphs->style->stroke_width.computed * scale);
            if ( glyphs->style->stroke_linecap.computed == SP_STROKE_LINECAP_BUTT ) nstyl.stroke_cap=butt_straight;
            if ( glyphs->style->stroke_linecap.computed == SP_STROKE_LINECAP_ROUND ) nstyl.stroke_cap=butt_round;
            if ( glyphs->style->stroke_linecap.computed == SP_STROKE_LINECAP_SQUARE ) nstyl.stroke_cap=butt_square;
            if ( glyphs->style->stroke_linejoin.computed == SP_STROKE_LINEJOIN_MITER ) nstyl.stroke_join=join_pointy;
            if ( glyphs->style->stroke_linejoin.computed == SP_STROKE_LINEJOIN_ROUND ) nstyl.stroke_join=join_round;
            if ( glyphs->style->stroke_linejoin.computed == SP_STROKE_LINEJOIN_BEVEL ) nstyl.stroke_join=join_straight;
            nstyl.stroke_miter_limit = glyphs->style->stroke_miterlimit.value;
            nstyl.nbDash=0;
            nstyl.dash_offset = 0;
            nstyl.dashes=NULL;
            if ( glyphs->style->stroke_dash.n_dash > 0 ) {
                nstyl.dash_offset = glyphs->style->stroke_dash.offset * scale;
                nstyl.nbDash=glyphs->style->stroke_dash.n_dash;
                nstyl.dashes=(double*)malloc(nstyl.nbDash*sizeof(double));
                for (int i = 0; i < nstyl.nbDash; i++) nstyl.dashes[i]= glyphs->style->stroke_dash.dash[i] * scale;
            }
            rfont = glyphs->font->RasterFont( nstyl);
            if ( nstyl.dashes ) free(nstyl.dashes);
            if (glyphs->sfont) glyphs->sfont->Unref();
            glyphs->sfont = rfont;

            NRRect narea;
            if ( glyphs->sfont ) glyphs->sfont->BBox(glyphs->glyph, &narea);
            narea.x0-=nstyl.stroke_width;
            narea.y0-=nstyl.stroke_width;
            narea.x1+=nstyl.stroke_width;
            narea.y1+=nstyl.stroke_width;
            bbox.x0 = narea.x0 + glyphs->x;
            bbox.y0 = narea.y0 + glyphs->y;
            bbox.x1 = narea.x1 + glyphs->x;
            bbox.y1 = narea.y1 + glyphs->y;
        }
    }
    if (nr_rect_d_test_empty(&bbox)) return NR_ARENA_ITEM_STATE_ALL;

    item->bbox.x0 = (gint32)(bbox.x0 - 1.0);
    item->bbox.y0 = (gint32)(bbox.y0 - 1.0);
    item->bbox.x1 = (gint32)(bbox.x1 + 1.0);
    item->bbox.y1 = (gint32)(bbox.y1 + 1.0);
    nr_arena_request_render_rect(item->arena, &item->bbox);

    return NR_ARENA_ITEM_STATE_ALL;
}

static guint
nr_arena_glyphs_clip(NRArenaItem *item, NRRectL */*area*/, NRPixBlock */*pb*/)
{
    NRArenaGlyphs *glyphs;

    glyphs = NR_ARENA_GLYPHS(item);

    if (!glyphs->font ) return item->state;

    /* TODO : render to greyscale pixblock provided for clipping */

    return item->state;
}

static NRArenaItem *
nr_arena_glyphs_pick(NRArenaItem *item, NR::Point p, gdouble /*delta*/, unsigned int /*sticky*/)
{
    NRArenaGlyphs *glyphs;

    glyphs = NR_ARENA_GLYPHS(item);

    if (!glyphs->font ) return NULL;
    if (!glyphs->style) return NULL;

    double const x = p[NR::X];
    double const y = p[NR::Y];
    /* With text we take a simple approach: pick if the point is in a characher bbox */
    if ((x >= item->bbox.x0) && (y >= item->bbox.y0) && (x <= item->bbox.x1) && (y <= item->bbox.y1)) return item;

    return NULL;
}

void
nr_arena_glyphs_set_path(NRArenaGlyphs *glyphs, SPCurve */*curve*/, unsigned int /*lieutenant*/, font_instance *font, gint glyph, NR::Matrix const *transform)
{
    nr_return_if_fail(glyphs != NULL);
    nr_return_if_fail(NR_IS_ARENA_GLYPHS(glyphs));

    nr_arena_item_request_render(NR_ARENA_ITEM(glyphs));

    if (transform) {
        glyphs->g_transform = *transform;
    } else {
        glyphs->g_transform.set_identity();
    }

    if (font) font->Ref();
    if (glyphs->font) glyphs->font->Unref();
    glyphs->font=font;
    glyphs->glyph = glyph;

    nr_arena_item_request_update(NR_ARENA_ITEM(glyphs), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_glyphs_set_style(NRArenaGlyphs *glyphs, SPStyle *style)
{
    nr_return_if_fail(glyphs != NULL);
    nr_return_if_fail(NR_IS_ARENA_GLYPHS(glyphs));

    if (style) sp_style_ref(style);
    if (glyphs->style) sp_style_unref(glyphs->style);
    glyphs->style = style;

    nr_arena_item_request_update(NR_ARENA_ITEM(glyphs), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static guint
nr_arena_glyphs_fill_mask(NRArenaGlyphs *glyphs, NRRectL *area, NRPixBlock *m)
{
    /* fixme: area == m->area, so merge these */

    NRArenaItem *item = NR_ARENA_ITEM(glyphs);

    if (glyphs->rfont && nr_rect_l_test_intersect(area, &item->bbox)) {
        raster_glyph *g = glyphs->rfont->GetGlyph(glyphs->glyph);
        if ( g ) g->Blit(NR::Point(glyphs->x, glyphs->y), *m);
    }

    return item->state;
}

static guint
nr_arena_glyphs_stroke_mask(NRArenaGlyphs *glyphs, NRRectL *area, NRPixBlock *m)
{
    NRArenaItem *item = NR_ARENA_ITEM(glyphs);
    if (glyphs->sfont && nr_rect_l_test_intersect(area, &item->bbox)) {
        raster_glyph *g=glyphs->sfont->GetGlyph(glyphs->glyph);
        if ( g ) g->Blit(NR::Point(glyphs->x, glyphs->y),*m);
    }

    return item->state;
}

static void nr_arena_glyphs_group_class_init(NRArenaGlyphsGroupClass *klass);
static void nr_arena_glyphs_group_init(NRArenaGlyphsGroup *group);
static void nr_arena_glyphs_group_finalize(NRObject *object);

static guint nr_arena_glyphs_group_update(NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset);
static unsigned int nr_arena_glyphs_group_render(cairo_t *ct, NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags);
static unsigned int nr_arena_glyphs_group_clip(NRArenaItem *item, NRRectL *area, NRPixBlock *pb);
static NRArenaItem *nr_arena_glyphs_group_pick(NRArenaItem *item, NR::Point p, gdouble delta, unsigned int sticky);

static NRArenaGroupClass *group_parent_class;

NRType
nr_arena_glyphs_group_get_type(void)
{
    static NRType type = 0;
    if (!type) {
        type = nr_object_register_type(NR_TYPE_ARENA_GROUP,
                                       "NRArenaGlyphsGroup",
                                       sizeof(NRArenaGlyphsGroupClass),
                                       sizeof(NRArenaGlyphsGroup),
                                       (void (*)(NRObjectClass *)) nr_arena_glyphs_group_class_init,
                                       (void (*)(NRObject *)) nr_arena_glyphs_group_init);
    }
    return type;
}

static void
nr_arena_glyphs_group_class_init(NRArenaGlyphsGroupClass *klass)
{
    NRObjectClass *object_class;
    NRArenaItemClass *item_class;

    object_class = (NRObjectClass *) klass;
    item_class = (NRArenaItemClass *) klass;

    group_parent_class = (NRArenaGroupClass *) ((NRObjectClass *) klass)->parent;

    object_class->finalize = nr_arena_glyphs_group_finalize;
    object_class->cpp_ctor = NRObject::invoke_ctor<NRArenaGlyphsGroup>;

    item_class->update = nr_arena_glyphs_group_update;
    item_class->render = nr_arena_glyphs_group_render;
    item_class->clip = nr_arena_glyphs_group_clip;
    item_class->pick = nr_arena_glyphs_group_pick;
}

static void
nr_arena_glyphs_group_init(NRArenaGlyphsGroup *group)
{
    group->style = NULL;
    group->paintbox.x0 = group->paintbox.y0 = 0.0F;
    group->paintbox.x1 = group->paintbox.y1 = 1.0F;

    group->fill_painter = NULL;
    group->stroke_painter = NULL;
}

static void
nr_arena_glyphs_group_finalize(NRObject *object)
{
    NRArenaGlyphsGroup *group=static_cast<NRArenaGlyphsGroup *>(object);

    if (group->fill_painter) {
        sp_painter_free(group->fill_painter);
        group->fill_painter = NULL;
    }

    if (group->stroke_painter) {
        sp_painter_free(group->stroke_painter);
        group->stroke_painter = NULL;
    }

    if (group->style) {
        sp_style_unref(group->style);
        group->style = NULL;
    }

    ((NRObjectClass *) group_parent_class)->finalize(object);
}

static guint
nr_arena_glyphs_group_update(NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset)
{
    NRArenaGlyphsGroup *group = NR_ARENA_GLYPHS_GROUP(item);

    if (group->fill_painter) {
        sp_painter_free(group->fill_painter);
        group->fill_painter = NULL;
    }

    if (group->stroke_painter) {
        sp_painter_free(group->stroke_painter);
        group->stroke_painter = NULL;
    }

    item->render_opacity = TRUE;
    if (group->style->fill.isPaintserver()) {
        group->fill_painter = sp_paint_server_painter_new(SP_STYLE_FILL_SERVER(group->style),
                                                          gc->transform, gc->parent->transform,
                                                          &group->paintbox);
        item->render_opacity = FALSE;
    }

    if (group->style->stroke.isPaintserver()) {
        group->stroke_painter = sp_paint_server_painter_new(SP_STYLE_STROKE_SERVER(group->style),
                                                            gc->transform, gc->parent->transform,
                                                            &group->paintbox);
        item->render_opacity = FALSE;
    }

    if ( item->render_opacity == TRUE && !group->style->stroke.isNone() && !group->style->fill.isNone() ) {
        item->render_opacity=FALSE;
    }

    if (((NRArenaItemClass *) group_parent_class)->update)
        return ((NRArenaItemClass *) group_parent_class)->update(item, area, gc, state, reset);

    return NR_ARENA_ITEM_STATE_ALL;
}


static unsigned int
nr_arena_glyphs_group_render(cairo_t *ct, NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int /*flags*/)
{
    NRArenaItem *child;

    NRArenaGroup *group = NR_ARENA_GROUP(item);
    NRArenaGlyphsGroup *ggroup = NR_ARENA_GLYPHS_GROUP(item);
    SPStyle const *style = ggroup->style;

    guint ret = item->state;

    if (item->arena->rendermode == Inkscape::RENDERMODE_OUTLINE) {

        if (!ct)
            return item->state;

        guint32 rgba = item->arena->outlinecolor;
        // FIXME: we use RGBA buffers but cairo writes BGRA (on i386), so we must cheat
        // by setting color channels in the "wrong" order
        cairo_set_source_rgba(ct, SP_RGBA32_B_F(rgba), SP_RGBA32_G_F(rgba), SP_RGBA32_R_F(rgba), SP_RGBA32_A_F(rgba));
        cairo_set_tolerance(ct, 1.25); // low quality, but good enough for outline mode

        for (child = group->children; child != NULL; child = child->next) {
            NRArenaGlyphs *g = NR_ARENA_GLYPHS(child);

            Geom::PathVector const * pathv = g->font->PathVector(g->glyph);

            cairo_new_path(ct);
            Geom::Matrix transform = to_2geom(g->g_transform * group->ctm);
            feed_pathvector_to_cairo (ct, *pathv, transform, (pb->area).upgrade(), false, 0);
            cairo_fill(ct);
            pb->empty = FALSE;
        }

        return ret;
    }



    /* Fill */
    if (!style->fill.isNone() || item->arena->rendermode == Inkscape::RENDERMODE_OUTLINE) {
        NRPixBlock m;
        nr_pixblock_setup_fast(&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);

        // if memory allocation failed, abort
        if (m.size != NR_PIXBLOCK_SIZE_TINY && m.data.px == NULL) {
            nr_pixblock_release (&m);
            return (item->state);
        }

        m.visible_area = pb->visible_area;

        /* Render children fill mask */
        for (child = group->children; child != NULL; child = child->next) {
            ret = nr_arena_glyphs_fill_mask(NR_ARENA_GLYPHS(child), area, &m);
            if (!(ret & NR_ARENA_ITEM_STATE_RENDER)) {
                nr_pixblock_release(&m);
                return ret;
            }
        }

        /* Composite into buffer */
        if (style->fill.isPaintserver()) {
            if (ggroup->fill_painter) {
                nr_arena_render_paintserver_fill(pb, area, ggroup->fill_painter, SP_SCALE24_TO_FLOAT(style->fill_opacity.value), &m);
            }
        } else if (style->fill.isColor() || item->arena->rendermode == Inkscape::RENDERMODE_OUTLINE) {
            guint32 rgba;
            if (item->arena->rendermode == Inkscape::RENDERMODE_OUTLINE) {
                // In outline mode, render fill only, using outlinecolor
                rgba = item->arena->outlinecolor;
            } else if ( item->render_opacity ) {
                rgba = style->fill.value.color.toRGBA32( SP_SCALE24_TO_FLOAT(style->fill_opacity.value) *
                                                         SP_SCALE24_TO_FLOAT(style->opacity.value) );
            } else {
                rgba = style->fill.value.color.toRGBA32( SP_SCALE24_TO_FLOAT(style->fill_opacity.value) );
            }
            nr_blit_pixblock_mask_rgba32(pb, &m, rgba);
            pb->empty = FALSE;
        }

        nr_pixblock_release(&m);
    }

    /* Stroke */
    if (!style->stroke.isNone() && !(item->arena->rendermode == Inkscape::RENDERMODE_OUTLINE)) {
        NRPixBlock m;
        guint32 rgba;
        nr_pixblock_setup_fast(&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);

        // if memory allocation failed, abort
        if (m.size != NR_PIXBLOCK_SIZE_TINY && m.data.px == NULL) {
            nr_pixblock_release (&m);
            return (item->state);
        }

        m.visible_area = pb->visible_area;
        /* Render children stroke mask */
        for (child = group->children; child != NULL; child = child->next) {
            ret = nr_arena_glyphs_stroke_mask(NR_ARENA_GLYPHS(child), area, &m);
            if (!(ret & NR_ARENA_ITEM_STATE_RENDER)) {
                nr_pixblock_release(&m);
                return ret;
            }
        }
        /* Composite into buffer */
        if (style->stroke.isPaintserver()) {
            if (ggroup->stroke_painter) {
                nr_arena_render_paintserver_fill(pb, area, ggroup->stroke_painter, SP_SCALE24_TO_FLOAT(style->stroke_opacity.value), &m);
            }
        } else if (style->stroke.isColor()) {
            if ( item->render_opacity ) {
                rgba = style->stroke.value.color.toRGBA32( SP_SCALE24_TO_FLOAT(style->stroke_opacity.value) *
                                                           SP_SCALE24_TO_FLOAT(style->opacity.value) );
            } else {
                rgba = style->stroke.value.color.toRGBA32( SP_SCALE24_TO_FLOAT(style->stroke_opacity.value) );
            }
            nr_blit_pixblock_mask_rgba32(pb, &m, rgba);
            pb->empty = FALSE;
        } else {
            // nothing
        }
        nr_pixblock_release(&m);
    }

    return ret;
}

static unsigned int
nr_arena_glyphs_group_clip(NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
    NRArenaGroup *group = NR_ARENA_GROUP(item);

    guint ret = item->state;

    /* Render children fill mask */
    for (NRArenaItem *child = group->children; child != NULL; child = child->next) {
        ret = nr_arena_glyphs_fill_mask(NR_ARENA_GLYPHS(child), area, pb);
        if (!(ret & NR_ARENA_ITEM_STATE_RENDER)) return ret;
    }

    return ret;
}

static NRArenaItem *
nr_arena_glyphs_group_pick(NRArenaItem *item, NR::Point p, gdouble delta, unsigned int sticky)
{
    NRArenaItem *picked = NULL;

    if (((NRArenaItemClass *) group_parent_class)->pick)
        picked = ((NRArenaItemClass *) group_parent_class)->pick(item, p, delta, sticky);

    if (picked) picked = item;

    return picked;
}

void
nr_arena_glyphs_group_clear(NRArenaGlyphsGroup *sg)
{
    NRArenaGroup *group = NR_ARENA_GROUP(sg);

    nr_arena_item_request_render(NR_ARENA_ITEM(group));

    while (group->children) {
        nr_arena_item_remove_child(NR_ARENA_ITEM(group), group->children);
    }
}

void
nr_arena_glyphs_group_add_component(NRArenaGlyphsGroup *sg, font_instance *font, int glyph, NR::Matrix const *transform)
{
    NRArenaGroup *group;

    group = NR_ARENA_GROUP(sg);

    Geom::PathVector const * pathv = ( font
                                       ? font->PathVector(glyph)
                                       : NULL );
    if ( pathv ) {
        nr_arena_item_request_render(NR_ARENA_ITEM(group));

        NRArenaItem *new_arena = NRArenaGlyphs::create(group->arena);
        nr_arena_item_append_child(NR_ARENA_ITEM(group), new_arena);
        nr_arena_item_unref(new_arena);
        nr_arena_glyphs_set_path(NR_ARENA_GLYPHS(new_arena), NULL, FALSE, font, glyph, transform);
        nr_arena_glyphs_set_style(NR_ARENA_GLYPHS(new_arena), sg->style);
    }
}

void
nr_arena_glyphs_group_set_style(NRArenaGlyphsGroup *sg, SPStyle *style)
{
    nr_return_if_fail(sg != NULL);
    nr_return_if_fail(NR_IS_ARENA_GLYPHS_GROUP(sg));

    NRArenaGroup *group = NR_ARENA_GROUP(sg);

    if (style) sp_style_ref(style);
    if (sg->style) sp_style_unref(sg->style);
    sg->style = style;

    for (NRArenaItem *child = group->children; child != NULL; child = child->next) {
        nr_return_if_fail(NR_IS_ARENA_GLYPHS(child));
        nr_arena_glyphs_set_style(NR_ARENA_GLYPHS(child), sg->style);
    }

    nr_arena_item_request_update(NR_ARENA_ITEM(sg), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_glyphs_group_set_paintbox(NRArenaGlyphsGroup *gg, NRRect const *pbox)
{
    nr_return_if_fail(gg != NULL);
    nr_return_if_fail(NR_IS_ARENA_GLYPHS_GROUP(gg));
    nr_return_if_fail(pbox != NULL);

    if ((pbox->x0 < pbox->x1) && (pbox->y0 < pbox->y1)) {
        gg->paintbox.x0 = pbox->x0;
        gg->paintbox.y0 = pbox->y0;
        gg->paintbox.x1 = pbox->x1;
        gg->paintbox.y1 = pbox->y1;
    } else {
        /* fixme: We kill warning, although not sure what to do here (Lauris) */
        gg->paintbox.x0 = gg->paintbox.y0 = 0.0F;
        gg->paintbox.x1 = gg->paintbox.y1 = 256.0F;
    }

    nr_arena_item_request_update(NR_ARENA_ITEM(gg), NR_ARENA_ITEM_STATE_ALL, FALSE);
}


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

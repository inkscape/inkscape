#define __NR_ARENA_SHAPE_C__

/*
 * RGBA display list system for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */



#include <display/nr-arena.h>
#include <display/nr-arena-shape.h>
#include <libnr/n-art-bpath.h>
#include <libnr/nr-path.h>
#include <libnr/nr-pixops.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-blit.h>
#include <livarot/Path.h>
#include <livarot/float-line.h>
#include <livarot/int-line.h>
#include <style.h>

//int  showRuns=0;
void nr_pixblock_render_shape_mask_or(NRPixBlock &m,Shape* theS);

static void nr_arena_shape_class_init(NRArenaShapeClass *klass);
static void nr_arena_shape_init(NRArenaShape *shape);
static void nr_arena_shape_finalize(NRObject *object);

static NRArenaItem *nr_arena_shape_children(NRArenaItem *item);
static void nr_arena_shape_add_child(NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref);
static void nr_arena_shape_remove_child(NRArenaItem *item, NRArenaItem *child);
static void nr_arena_shape_set_child_position(NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref);

static guint nr_arena_shape_update(NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset);
static unsigned int nr_arena_shape_render(NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags);
static guint nr_arena_shape_clip(NRArenaItem *item, NRRectL *area, NRPixBlock *pb);
static NRArenaItem *nr_arena_shape_pick(NRArenaItem *item, NR::Point p, double delta, unsigned int sticky);

static NRArenaItemClass *shape_parent_class;

NRType
nr_arena_shape_get_type(void)
{
    static NRType type = 0;
    if (!type) {
        type = nr_object_register_type(NR_TYPE_ARENA_ITEM,
                                       "NRArenaShape",
                                       sizeof(NRArenaShapeClass),
                                       sizeof(NRArenaShape),
                                       (void (*)(NRObjectClass *)) nr_arena_shape_class_init,
                                       (void (*)(NRObject *)) nr_arena_shape_init);
    }
    return type;
}

static void
nr_arena_shape_class_init(NRArenaShapeClass *klass)
{
    NRObjectClass *object_class;
    NRArenaItemClass *item_class;

    object_class = (NRObjectClass *) klass;
    item_class = (NRArenaItemClass *) klass;

    shape_parent_class = (NRArenaItemClass *)  ((NRObjectClass *) klass)->parent;

    object_class->finalize = nr_arena_shape_finalize;
    object_class->cpp_ctor = NRObject::invoke_ctor<NRArenaShape>;

    item_class->children = nr_arena_shape_children;
    item_class->add_child = nr_arena_shape_add_child;
    item_class->set_child_position = nr_arena_shape_set_child_position;
    item_class->remove_child = nr_arena_shape_remove_child;
    item_class->update = nr_arena_shape_update;
    item_class->render = nr_arena_shape_render;
    item_class->clip = nr_arena_shape_clip;
    item_class->pick = nr_arena_shape_pick;
}

static void
nr_arena_shape_init(NRArenaShape *shape)
{
    shape->curve = NULL;
    shape->style = NULL;
    shape->paintbox.x0 = shape->paintbox.y0 = 0.0F;
    shape->paintbox.x1 = shape->paintbox.y1 = 256.0F;

    nr_matrix_set_identity(&shape->ctm);
    shape->fill_painter = NULL;
    shape->stroke_painter = NULL;
    shape->cached_fill = NULL;
    shape->cached_stroke = NULL;
    shape->cached_fpartialy = false;
    shape->cached_spartialy = false;
    shape->fill_shp = NULL;
    shape->stroke_shp = NULL;

    shape->delayed_shp = false;

    shape->approx_bbox.x0 = shape->approx_bbox.y0 = 0;
    shape->approx_bbox.x1 = shape->approx_bbox.y1 = 0;
    nr_matrix_set_identity(&shape->cached_fctm);
    nr_matrix_set_identity(&shape->cached_sctm);

    shape->markers = NULL;
}

static void
nr_arena_shape_finalize(NRObject *object)
{
    NRArenaShape *shape = (NRArenaShape *) object;

    if (shape->fill_shp) delete shape->fill_shp;
    if (shape->stroke_shp) delete shape->stroke_shp;
    if (shape->cached_fill) delete shape->cached_fill;
    if (shape->cached_stroke) delete shape->cached_stroke;
    if (shape->fill_painter) sp_painter_free(shape->fill_painter);
    if (shape->stroke_painter) sp_painter_free(shape->stroke_painter);

    if (shape->style) sp_style_unref(shape->style);
    if (shape->curve) sp_curve_unref(shape->curve);

    ((NRObjectClass *) shape_parent_class)->finalize(object);
}

static NRArenaItem *
nr_arena_shape_children(NRArenaItem *item)
{
    NRArenaShape *shape = (NRArenaShape *) item;

    return shape->markers;
}

static void
nr_arena_shape_add_child(NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref)
{
    NRArenaShape *shape = (NRArenaShape *) item;

    if (!ref) {
        shape->markers = nr_arena_item_attach(item, child, NULL, shape->markers);
    } else {
        ref->next = nr_arena_item_attach(item, child, ref, ref->next);
    }

    nr_arena_item_request_update(item, NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static void
nr_arena_shape_remove_child(NRArenaItem *item, NRArenaItem *child)
{
    NRArenaShape *shape = (NRArenaShape *) item;

    if (child->prev) {
        nr_arena_item_detach(item, child);
    } else {
        shape->markers = nr_arena_item_detach(item, child);
    }

    nr_arena_item_request_update(item, NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static void
nr_arena_shape_set_child_position(NRArenaItem *item, NRArenaItem *child, NRArenaItem *ref)
{
    NRArenaShape *shape = (NRArenaShape *) item;

    if (child->prev) {
        nr_arena_item_detach(item, child);
    } else {
        shape->markers = nr_arena_item_detach(item, child);
    }

    if (!ref) {
        shape->markers = nr_arena_item_attach(item, child, NULL, shape->markers);
    } else {
        ref->next = nr_arena_item_attach(item, child, ref, ref->next);
    }

    nr_arena_item_request_render(child);
}

void nr_arena_shape_update_stroke(NRArenaShape *shape, NRGC* gc, NRRectL *area);
void nr_arena_shape_update_fill(NRArenaShape *shape, NRGC *gc, NRRectL *area, bool force_shape = false);
void nr_arena_shape_add_bboxes(NRArenaShape* shape,NRRect &bbox);

static guint
nr_arena_shape_update(NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset)
{
    NRRect bbox;

    NRArenaShape *shape = NR_ARENA_SHAPE(item);

    unsigned int beststate = NR_ARENA_ITEM_STATE_ALL;

    unsigned int newstate;
    for (NRArenaItem *child = shape->markers; child != NULL; child = child->next) {
        newstate = nr_arena_item_invoke_update(child, area, gc, state, reset);
        beststate = beststate & newstate;
    }

    if (!(state & NR_ARENA_ITEM_STATE_RENDER)) {
        /* We do not have to create rendering structures */
        shape->ctm = gc->transform;
        if (state & NR_ARENA_ITEM_STATE_BBOX) {
            if (shape->curve) {
                NRBPath bp;
                /* fixme: */
                bbox.x0 = bbox.y0 = NR_HUGE;
                bbox.x1 = bbox.y1 = -NR_HUGE;
                bp.path = shape->curve->bpath;
                nr_path_matrix_bbox_union(&bp, gc->transform, &bbox);
                item->bbox.x0 = (gint32)(bbox.x0 - 1.0F);
                item->bbox.y0 = (gint32)(bbox.y0 - 1.0F);
                item->bbox.x1 = (gint32)(bbox.x1 + 1.9999F);
                item->bbox.y1 = (gint32)(bbox.y1 + 1.9999F);
            }
            if (beststate & NR_ARENA_ITEM_STATE_BBOX) {
                for (NRArenaItem *child = shape->markers; child != NULL; child = child->next) {
                    nr_rect_l_union(&item->bbox, &item->bbox, &child->bbox);
                }
            }
        }
        return (state | item->state);
    }

    shape->delayed_shp=true;
    shape->ctm = gc->transform;
    bbox.x0 = bbox.y0 = NR_HUGE;
    bbox.x1 = bbox.y1 = -NR_HUGE;

    if (shape->curve) {
        NRBPath bp;
        /* fixme: */
        bbox.x0 = bbox.y0 = NR_HUGE;
        bbox.x1 = bbox.y1 = -NR_HUGE;
        bp.path = shape->curve->bpath;
        nr_path_matrix_bbox_union(&bp, gc->transform, &bbox);
        if (shape->_stroke.paint.type() != NRArenaShape::Paint::NONE) {
            float width, scale;
            scale = NR_MATRIX_DF_EXPANSION(&gc->transform);
            width = MAX(0.125, shape->_stroke.width * scale);
            if ( fabs(shape->_stroke.width * scale) > 0.01 ) { // sinon c'est 0=oon veut pas de bord
                bbox.x0-=width;
                bbox.x1+=width;
                bbox.y0-=width;
                bbox.y1+=width;
            }
            // those pesky miters, now
            float miterMax=width*shape->_stroke.mitre_limit;
            if ( miterMax > 0.01 ) {
                // grunt mode. we should compute the various miters instead (one for each point on the curve)
                bbox.x0-=miterMax;
                bbox.x1+=miterMax;
                bbox.y0-=miterMax;
                bbox.y1+=miterMax;
            }
        }
    } else {
    }
    shape->approx_bbox.x0 = (gint32)(bbox.x0 - 1.0F);
    shape->approx_bbox.y0 = (gint32)(bbox.y0 - 1.0F);
    shape->approx_bbox.x1 = (gint32)(bbox.x1 + 1.9999F);
    shape->approx_bbox.y1 = (gint32)(bbox.y1 + 1.9999F);
    if ( area && nr_rect_l_test_intersect(area, &shape->approx_bbox) ) shape->delayed_shp=false;

    /* Release state data */
    if (TRUE || !nr_matrix_test_transform_equal(&gc->transform, &shape->ctm, NR_EPSILON)) {
        /* Concept test */
        if (shape->fill_shp) {
            delete shape->fill_shp;
            shape->fill_shp = NULL;
        }
    }
    if (shape->stroke_shp) {
        delete shape->stroke_shp;
        shape->stroke_shp = NULL;
    }
    if (shape->fill_painter) {
        sp_painter_free(shape->fill_painter);
        shape->fill_painter = NULL;
    }
    if (shape->stroke_painter) {
        sp_painter_free(shape->stroke_painter);
        shape->stroke_painter = NULL;
    }

    if (!shape->curve || !shape->style) return NR_ARENA_ITEM_STATE_ALL;
    if (sp_curve_is_empty(shape->curve)) return NR_ARENA_ITEM_STATE_ALL;
    if ( ( shape->_fill.paint.type() == NRArenaShape::Paint::NONE ) &&
         ( shape->_stroke.paint.type() == NRArenaShape::Paint::NONE ) )
    {
        return NR_ARENA_ITEM_STATE_ALL;
    }

    /* Build state data */
    if ( shape->delayed_shp ) {
        item->bbox=shape->approx_bbox;
    } else {
        nr_arena_shape_update_stroke(shape, gc, area);
        nr_arena_shape_update_fill(shape, gc, area);

        bbox.x0 = bbox.y0 = bbox.x1 = bbox.y1 = 0.0;
        nr_arena_shape_add_bboxes(shape,bbox);

        shape->approx_bbox.x0 = (gint32)(bbox.x0 - 1.0F);
        shape->approx_bbox.y0 = (gint32)(bbox.y0 - 1.0F);
        shape->approx_bbox.x1 = (gint32)(bbox.x1 + 1.9999F);
        shape->approx_bbox.y1 = (gint32)(bbox.y1 + 1.9999F);
    }

    if (nr_rect_d_test_empty(&bbox)) return NR_ARENA_ITEM_STATE_ALL;

    item->bbox.x0 = (gint32)(bbox.x0 - 1.0F);
    item->bbox.y0 = (gint32)(bbox.y0 - 1.0F);
    item->bbox.x1 = (gint32)(bbox.x1 + 1.0F);
    item->bbox.y1 = (gint32)(bbox.y1 + 1.0F);
    nr_arena_request_render_rect(item->arena, &item->bbox);

    item->render_opacity = TRUE;
    if ( shape->_fill.paint.type() == NRArenaShape::Paint::SERVER ) {
        if (gc && gc->parent) {
            shape->fill_painter = sp_paint_server_painter_new(shape->_fill.paint.server(),
                                                              NR::Matrix(&gc->transform), NR::Matrix(&gc->parent->transform),
                                                              &shape->paintbox);
        }
        item->render_opacity = FALSE;
    }
    if ( shape->_stroke.paint.type() == NRArenaShape::Paint::SERVER ) {
        if (gc && gc->parent) {
            shape->stroke_painter = sp_paint_server_painter_new(shape->_stroke.paint.server(),
                                                                NR::Matrix(&gc->transform), NR::Matrix(&gc->parent->transform),
                                                                &shape->paintbox);
        }
        item->render_opacity = FALSE;
    }
    if ( item->render_opacity == TRUE
         && shape->_fill.paint.type()   != NRArenaShape::Paint::NONE
         && shape->_stroke.paint.type() != NRArenaShape::Paint::NONE )
    {
        // don't merge item opacity with paint opacity if there is a stroke on the fill
        item->render_opacity = FALSE;
    }

    if (beststate & NR_ARENA_ITEM_STATE_BBOX) {
        for (NRArenaItem *child = shape->markers; child != NULL; child = child->next) {
            nr_rect_l_union(&item->bbox, &item->bbox, &child->bbox);
        }
    }

    return NR_ARENA_ITEM_STATE_ALL;
}

int matrix_is_isometry(NR::Matrix p) {
    NR::Matrix   tp;
    // transposition
    tp[0]=p[0];
    tp[1]=p[2];
    tp[2]=p[1];
    tp[3]=p[3];
    for (int i = 4; i < 6; i++) // shut valgrind up :)
        tp[i] = p[i] = 0;
    NR::Matrix   isom = tp*p; // A^T * A = adjunct?
    // Is the adjunct nearly an identity function?
    if (isom.is_translation(0.01)) {
        // the transformation is an isometry -> no need to recompute
        // the uncrossed polygon
        if ( p.det() < 0 )
            return -1;
        else
            return 1;
    }
    return 0;
}

static bool is_inner_area(NRRectL const &outer, NRRectL const &inner) {
    return (outer.x0 <= inner.x0 && outer.y0 <= inner.y0 && outer.x1 >= inner.x1 && outer.y1 >= inner.y1);
}

/** force_shape is used for clipping paths, when we need the shape for clipping even if it's not filled */
void
nr_arena_shape_update_fill(NRArenaShape *shape, NRGC *gc, NRRectL *area, bool force_shape)
{
    if ((shape->_fill.paint.type() != NRArenaShape::Paint::NONE || force_shape) &&
        ((shape->curve->end > 2) || (shape->curve->bpath[1].code == NR_CURVETO)) ) {
        if (TRUE || !shape->fill_shp) {
            NR::Matrix  cached_to_new;
            int isometry = 0;
            if ( shape->cached_fill ) {
                if (shape->cached_fctm == gc->transform) {
                    isometry = 2; // identity
                } else {
                    cached_to_new = shape->cached_fctm.inverse() * gc->transform;
                    isometry = matrix_is_isometry(cached_to_new);
                }
                if (0 != isometry && !is_inner_area(shape->cached_farea, *area))
                    isometry = 0;
            }
            if ( isometry == 0 ) {
                if ( shape->cached_fill == NULL ) shape->cached_fill=new Shape;
                shape->cached_fill->Reset();

                Path*  thePath=new Path;
                Shape* theShape=new Shape;
                {
                    NR::Matrix   tempMat(gc->transform);
                    thePath->LoadArtBPath(shape->curve->bpath,tempMat,true);
                }

                if (is_inner_area(*area, NR_ARENA_ITEM(shape)->bbox)) {
                    thePath->Convert(1.0);
                    shape->cached_fpartialy = false;
                } else {
                    thePath->Convert(area, 1.0);
                    shape->cached_fpartialy = true;
                }

                thePath->Fill(theShape, 0);

                if ( shape->_fill.rule == NRArenaShape::EVEN_ODD ) {
                    shape->cached_fill->ConvertToShape(theShape, fill_oddEven);
                    // alternatively, this speeds up rendering of oddeven shapes but disables AA :(
                    //shape->cached_fill->Copy(theShape);
                } else {
                    shape->cached_fill->ConvertToShape(theShape, fill_nonZero);
                }
                shape->cached_fctm=gc->transform;
                shape->cached_farea = *area;
                delete theShape;
                delete thePath;
                if ( shape->fill_shp == NULL )
                    shape->fill_shp = new Shape;

                shape->fill_shp->Copy(shape->cached_fill);

            } else if ( 2 == isometry ) {
                if ( shape->fill_shp == NULL ) {
                    shape->fill_shp = new Shape;
                    shape->fill_shp->Copy(shape->cached_fill);
                }
            } else {

                if ( shape->fill_shp == NULL )
                    shape->fill_shp = new Shape;

                shape->fill_shp->Reset(shape->cached_fill->numberOfPoints(),
                                       shape->cached_fill->numberOfEdges());
                for (int i = 0; i < shape->cached_fill->numberOfPoints(); i++)
                    shape->fill_shp->AddPoint(shape->cached_fill->getPoint(i).x * cached_to_new);
                if ( isometry == 1 ) {
                    for (int i = 0; i < shape->cached_fill->numberOfEdges(); i++)
                        shape->fill_shp->AddEdge(shape->cached_fill->getEdge(i).st,
                                                 shape->cached_fill->getEdge(i).en);
                } else if ( isometry == -1 ) { // need to flip poly.
                    for (int i = 0; i < shape->cached_fill->numberOfEdges(); i++)
                        shape->fill_shp->AddEdge(shape->cached_fill->getEdge(i).en,
                                                 shape->cached_fill->getEdge(i).st);
                }
                shape->fill_shp->ForceToPolygon();
                shape->fill_shp->needPointsSorting();
                shape->fill_shp->needEdgesSorting();
            }
            shape->delayed_shp |= shape->cached_fpartialy;
        }
    }
}

void
nr_arena_shape_update_stroke(NRArenaShape *shape,NRGC* gc, NRRectL *area)
{
    SPStyle* style = shape->style;

    float const scale = NR_MATRIX_DF_EXPANSION(&gc->transform);

    if (NR_ARENA_ITEM(shape)->arena->rendermode == RENDERMODE_OUTLINE ||
        ((shape->_stroke.paint.type() != NRArenaShape::Paint::NONE) &&
         ( fabs(shape->_stroke.width * scale) > 0.01 ))) { // sinon c'est 0=oon veut pas de bord

        float width = MAX(0.125, shape->_stroke.width * scale);
        if (NR_ARENA_ITEM(shape)->arena->rendermode == RENDERMODE_OUTLINE)
            width = 0.5; // 1 pixel wide, independent of zoom

        NR::Matrix cached_to_new;

        int isometry = 0;
        if ( shape->cached_stroke ) {
            if (shape->cached_sctm == gc->transform) {
                isometry = 2; // identity
            } else {
                cached_to_new = shape->cached_sctm.inverse() * gc->transform;
                isometry = matrix_is_isometry(cached_to_new);
            }
            if (0 != isometry && !is_inner_area(shape->cached_sarea, *area))
                isometry = 0;
        }

        if ( isometry == 0 ) {
            if ( shape->cached_stroke == NULL ) shape->cached_stroke=new Shape;
            shape->cached_stroke->Reset();
            Path*  thePath = new Path;
            Shape* theShape = new Shape;
            {
                NR::Matrix   tempMat(gc->transform);
                thePath->LoadArtBPath(shape->curve->bpath, tempMat, true);
            }

            // add some padding to the rendering area, so clipped path does not go into a render area
            NRRectL padded_area = *area;
            padded_area.x0 -= (NR::ICoord)width;
            padded_area.x1 += (NR::ICoord)width;
            padded_area.y0 -= (NR::ICoord)width;
            padded_area.y1 += (NR::ICoord)width;
            if ((style->stroke_dash.n_dash && NR_ARENA_ITEM(shape)->arena->rendermode != RENDERMODE_OUTLINE) || is_inner_area(padded_area, NR_ARENA_ITEM(shape)->bbox)) {
                thePath->Convert((NR_ARENA_ITEM(shape)->arena->rendermode != RENDERMODE_OUTLINE) ? 1.0 : 4.0);
                shape->cached_spartialy = false;
            }
            else {
                thePath->Convert(&padded_area, (NR_ARENA_ITEM(shape)->arena->rendermode != RENDERMODE_OUTLINE) ? 1.0 : 4.0);
                shape->cached_spartialy = true;
            }

            if (style->stroke_dash.n_dash && NR_ARENA_ITEM(shape)->arena->rendermode != RENDERMODE_OUTLINE) {
                double dlen = 0.0;
                for (int i = 0; i < style->stroke_dash.n_dash; i++) {
                    dlen += style->stroke_dash.dash[i] * scale;
                }
                if (dlen >= 1.0) {
                    NRVpathDash dash;
                    dash.offset = style->stroke_dash.offset * scale;
                    dash.n_dash = style->stroke_dash.n_dash;
                    dash.dash = g_new(double, dash.n_dash);
                    for (int i = 0; i < dash.n_dash; i++) {
                        dash.dash[i] = style->stroke_dash.dash[i] * scale;
                    }
                    int    nbD=dash.n_dash;
                    float  *dashs=(float*)malloc((nbD+1)*sizeof(float));
                    while ( dash.offset >= dlen ) dash.offset-=dlen;
                    dashs[0]=dash.dash[0];
                    for (int i=1; i<nbD; i++) {
                        dashs[i]=dashs[i-1]+dash.dash[i];
                    }
                    // modulo dlen
                    thePath->DashPolyline(0.0,0.0,dlen,nbD,dashs,true,dash.offset);
                    free(dashs);
                    g_free(dash.dash);
                }
            }
            ButtType butt=butt_straight;
            switch (shape->_stroke.cap) {
                case NRArenaShape::BUTT_CAP:
                    butt = butt_straight;
                    break;
                case NRArenaShape::ROUND_CAP:
                    butt = butt_round;
                    break;
                case NRArenaShape::SQUARE_CAP:
                    butt = butt_square;
                    break;
            }
            JoinType join=join_straight;
            switch (shape->_stroke.join) {
                case NRArenaShape::MITRE_JOIN:
                    join = join_pointy;
                    break;
                case NRArenaShape::ROUND_JOIN:
                    join = join_round;
                    break;
                case NRArenaShape::BEVEL_JOIN:
                    join = join_straight;
                    break;
            }

            if (NR_ARENA_ITEM(shape)->arena->rendermode == RENDERMODE_OUTLINE) {
                butt = butt_straight;
                join = join_straight;
            }

            thePath->Stroke(theShape, false, 0.5*width, join, butt,
                            0.5*width*shape->_stroke.mitre_limit);


            if (NR_ARENA_ITEM(shape)->arena->rendermode == RENDERMODE_OUTLINE) {
                // speeds it up, but uses evenodd for the stroke shape (which does not matter for 1-pixel wide outline)
                shape->cached_stroke->Copy(theShape);
            } else {
                shape->cached_stroke->ConvertToShape(theShape, fill_nonZero);
            }

            shape->cached_sctm=gc->transform;
            shape->cached_sarea = *area;
            delete thePath;
            delete theShape;
            if ( shape->stroke_shp == NULL ) shape->stroke_shp=new Shape;

            shape->stroke_shp->Copy(shape->cached_stroke);

        } else if ( 2 == isometry ) {
            if ( shape->stroke_shp == NULL ) {
                shape->stroke_shp=new Shape;
                shape->stroke_shp->Copy(shape->cached_stroke);
            }
        } else {
            if ( shape->stroke_shp == NULL )
                shape->stroke_shp=new Shape;
            shape->stroke_shp->Reset(shape->cached_stroke->numberOfPoints(), shape->cached_stroke->numberOfEdges());
            for (int i = 0; i < shape->cached_stroke->numberOfPoints(); i++)
                shape->stroke_shp->AddPoint(shape->cached_stroke->getPoint(i).x * cached_to_new);
            if ( isometry == 1 ) {
                for (int i = 0; i < shape->cached_stroke->numberOfEdges(); i++)
                    shape->stroke_shp->AddEdge(shape->cached_stroke->getEdge(i).st,
                                               shape->cached_stroke->getEdge(i).en);
            } else if ( isometry == -1 ) {
                for (int i = 0; i < shape->cached_stroke->numberOfEdges(); i++)
                    shape->stroke_shp->AddEdge(shape->cached_stroke->getEdge(i).en,
                                               shape->cached_stroke->getEdge(i).st);
            }
            shape->stroke_shp->ForceToPolygon();
            shape->stroke_shp->needPointsSorting();
            shape->stroke_shp->needEdgesSorting();
        }
        shape->delayed_shp |= shape->cached_spartialy;
    }
}


void
nr_arena_shape_add_bboxes(NRArenaShape* shape, NRRect &bbox)
{
    if ( shape->stroke_shp ) {
        shape->stroke_shp->CalcBBox();
        shape->stroke_shp->leftX=floor(shape->stroke_shp->leftX);
        shape->stroke_shp->rightX=ceil(shape->stroke_shp->rightX);
        shape->stroke_shp->topY=floor(shape->stroke_shp->topY);
        shape->stroke_shp->bottomY=ceil(shape->stroke_shp->bottomY);
        if ( bbox.x0 >= bbox.x1 ) {
            if ( shape->stroke_shp->leftX < shape->stroke_shp->rightX ) {
                bbox.x0=shape->stroke_shp->leftX;
                bbox.x1=shape->stroke_shp->rightX;
            }
        } else {
            if ( shape->stroke_shp->leftX < bbox.x0 )
                bbox.x0=shape->stroke_shp->leftX;
            if ( shape->stroke_shp->rightX > bbox.x1 )
                bbox.x1=shape->stroke_shp->rightX;
        }
        if ( bbox.y0 >= bbox.y1 ) {
            if ( shape->stroke_shp->topY < shape->stroke_shp->bottomY ) {
                bbox.y0=shape->stroke_shp->topY;
                bbox.y1=shape->stroke_shp->bottomY;
            }
        } else {
            if ( shape->stroke_shp->topY < bbox.y0 )
                bbox.y0=shape->stroke_shp->topY;
            if ( shape->stroke_shp->bottomY > bbox.y1 )
                bbox.y1=shape->stroke_shp->bottomY;
        }
    }
    if ( shape->fill_shp ) {
        shape->fill_shp->CalcBBox();
        shape->fill_shp->leftX=floor(shape->fill_shp->leftX);
        shape->fill_shp->rightX=ceil(shape->fill_shp->rightX);
        shape->fill_shp->topY=floor(shape->fill_shp->topY);
        shape->fill_shp->bottomY=ceil(shape->fill_shp->bottomY);
        if ( bbox.x0 >= bbox.x1 ) {
            if ( shape->fill_shp->leftX < shape->fill_shp->rightX ) {
                bbox.x0=shape->fill_shp->leftX;
                bbox.x1=shape->fill_shp->rightX;
            }
        } else {
            if ( shape->fill_shp->leftX < bbox.x0 ) bbox.x0=shape->fill_shp->leftX;
            if ( shape->fill_shp->rightX > bbox.x1 ) bbox.x1=shape->fill_shp->rightX;
        }
        if ( bbox.y0 >= bbox.y1 ) {
            if ( shape->fill_shp->topY < shape->fill_shp->bottomY ) {
                bbox.y0=shape->fill_shp->topY;
                bbox.y1=shape->fill_shp->bottomY;
            }
        } else {
            if ( shape->fill_shp->topY < bbox.y0 ) bbox.y0=shape->fill_shp->topY;
            if ( shape->fill_shp->bottomY > bbox.y1 ) bbox.y1=shape->fill_shp->bottomY;
        }
    }
}
static unsigned int
nr_arena_shape_render(NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags)
{
    NRArenaShape *shape = NR_ARENA_SHAPE(item);

    if (!shape->curve) return item->state;
    if (!shape->style) return item->state;

    if ( shape->delayed_shp ) {
        if ( nr_rect_l_test_intersect(area, &item->bbox) ) {
            NRGC   tempGC(NULL);
            tempGC.transform=shape->ctm;
            shape->delayed_shp = false;
            nr_arena_shape_update_stroke(shape,&tempGC,&pb->visible_area);
            nr_arena_shape_update_fill(shape,&tempGC,&pb->visible_area);
/*      NRRect bbox;
        bbox.x0 = bbox.y0 = bbox.x1 = bbox.y1 = 0.0;
        nr_arena_shape_add_bboxes(shape,bbox);
        item->bbox.x0 = (gint32)(bbox.x0 - 1.0F);
        item->bbox.y0 = (gint32)(bbox.y0 - 1.0F);
        item->bbox.x1 = (gint32)(bbox.x1 + 1.0F);
        item->bbox.y1 = (gint32)(bbox.y1 + 1.0F);
        shape->approx_bbox=item->bbox;*/
        } else {
            return item->state;
        }
    }

    SPStyle const *style = shape->style;
    if ( shape->fill_shp && NR_ARENA_ITEM(shape)->arena->rendermode != RENDERMODE_OUTLINE) {
        NRPixBlock m;
        guint32 rgba;

        nr_pixblock_setup_fast(&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
        m.visible_area = pb->visible_area; 
        nr_pixblock_render_shape_mask_or(m,shape->fill_shp);
        m.empty = FALSE;

        if (shape->_fill.paint.type() == NRArenaShape::Paint::NONE) {
            // do not render fill in any way
        } else if (shape->_fill.paint.type() == NRArenaShape::Paint::COLOR) {
            if ( item->render_opacity ) {
                rgba = sp_color_get_rgba32_falpha(&shape->_fill.paint.color(),
                                                  shape->_fill.opacity *
                                                  SP_SCALE24_TO_FLOAT(style->opacity.value));
            } else {
                rgba = sp_color_get_rgba32_falpha(&shape->_fill.paint.color(),
                                                  shape->_fill.opacity);
            }
            nr_blit_pixblock_mask_rgba32(pb, &m, rgba);
            pb->empty = FALSE;
        } else if (shape->_fill.paint.type() == NRArenaShape::Paint::SERVER) {
            if (shape->fill_painter) {
                nr_arena_render_paintserver_fill(pb, area, shape->fill_painter, shape->_fill.opacity, &m);
            }
        }

        nr_pixblock_release(&m);
    }

    if ( shape->stroke_shp ) {
        NRPixBlock m;
        guint32 rgba;

        nr_pixblock_setup_fast(&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
        m.visible_area = pb->visible_area; 
        nr_pixblock_render_shape_mask_or(m, shape->stroke_shp);
        m.empty = FALSE;

        if (shape->_stroke.paint.type() == NRArenaShape::Paint::COLOR ||
            NR_ARENA_ITEM(shape)->arena->rendermode == RENDERMODE_OUTLINE) {
            if ( NR_ARENA_ITEM(shape)->arena->rendermode == RENDERMODE_OUTLINE) {
                rgba = NR_ARENA_ITEM(shape)->arena->outlinecolor;
            } else if ( item->render_opacity ) {
                rgba = sp_color_get_rgba32_falpha(&shape->_stroke.paint.color(),
                                                  shape->_stroke.opacity *
                                                  SP_SCALE24_TO_FLOAT(style->opacity.value));
            } else {
                rgba = sp_color_get_rgba32_falpha(&shape->_stroke.paint.color(),
                                                  shape->_stroke.opacity);
            }
            nr_blit_pixblock_mask_rgba32(pb, &m, rgba);
            pb->empty = FALSE;
        } else if (shape->_stroke.paint.type() == NRArenaShape::Paint::SERVER) {
            if (shape->stroke_painter) {
                nr_arena_render_paintserver_fill(pb, area, shape->stroke_painter, shape->_stroke.opacity, &m);
            }
        }

        nr_pixblock_release(&m);
    }

    /* Just compose children into parent buffer */
    for (NRArenaItem *child = shape->markers; child != NULL; child = child->next) {
        unsigned int ret;
        ret = nr_arena_item_invoke_render(child, area, pb, flags);
        if (ret & NR_ARENA_ITEM_STATE_INVALID) return ret;
    }

    return item->state;
}

static guint
nr_arena_shape_clip(NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
    NRArenaShape *shape = NR_ARENA_SHAPE(item);
    if (!shape->curve) return item->state;

    if ( shape->delayed_shp || shape->fill_shp == NULL) { // we need a fill shape no matter what
        if ( nr_rect_l_test_intersect(area, &item->bbox) ) {
            NRGC   tempGC(NULL);
            tempGC.transform=shape->ctm;
            shape->delayed_shp = false;
            nr_arena_shape_update_fill(shape, &tempGC, &pb->visible_area, true);
        } else {
            return item->state;
        }
    }

    if ( shape->fill_shp ) {
        NRPixBlock m;

        /* fixme: We can OR in one step (Lauris) */
        nr_pixblock_setup_fast(&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);
        m.visible_area = pb->visible_area; 
        nr_pixblock_render_shape_mask_or(m,shape->fill_shp);

        for (int y = area->y0; y < area->y1; y++) {
            unsigned char *s, *d;
            s = NR_PIXBLOCK_PX(&m) + (y - area->y0) * m.rs;
            d = NR_PIXBLOCK_PX(pb) + (y - area->y0) * pb->rs;
            for (int x = area->x0; x < area->x1; x++) {
                *d = NR_A7_NORMALIZED(*s,*d);
                d ++;
                s ++;
            }
        }
        nr_pixblock_release(&m);
        pb->empty = FALSE;
    }

    return item->state;
}

static NRArenaItem *
nr_arena_shape_pick(NRArenaItem *item, NR::Point p, double delta, unsigned int /*sticky*/)
{
    NRArenaShape *shape = NR_ARENA_SHAPE(item);

    if (!shape->curve) return NULL;
    if (!shape->style) return NULL;
    if ( shape->delayed_shp ) {
        NRRectL  area, updateArea;
        area.x0=(int)floor(p[NR::X]);
        area.x1=(int)ceil(p[NR::X]);
        area.y0=(int)floor(p[NR::Y]);
        area.y1=(int)ceil(p[NR::Y]);
        int idelta = (int)ceil(delta) + 1;
        // njh: inset rect
        area.x0-=idelta;
        area.x1+=idelta;
        area.y0-=idelta;
        area.y1+=idelta;
        if ( nr_rect_l_test_intersect(&area, &item->bbox) ) {
            NRGC   tempGC(NULL);
            tempGC.transform=shape->ctm;
            updateArea = item->bbox;
            if (shape->cached_stroke)
                nr_rect_l_intersect (&updateArea, &updateArea, &shape->cached_sarea);

            shape->delayed_shp = false;
            nr_arena_shape_update_stroke(shape, &tempGC, &updateArea);
            nr_arena_shape_update_fill(shape, &tempGC, &updateArea);
            /*      NRRect bbox;
                    bbox.x0 = bbox.y0 = bbox.x1 = bbox.y1 = 0.0;
                    nr_arena_shape_add_bboxes(shape,bbox);
                    item->bbox.x0 = (gint32)(bbox.x0 - 1.0F);
                    item->bbox.y0 = (gint32)(bbox.y0 - 1.0F);
                    item->bbox.x1 = (gint32)(bbox.x1 + 1.0F);
                    item->bbox.y1 = (gint32)(bbox.y1 + 1.0F);
                    shape->approx_bbox=item->bbox;*/
        }
    }

    if (item->state & NR_ARENA_ITEM_STATE_RENDER) {
        if (shape->fill_shp && (shape->_fill.paint.type() != NRArenaShape::Paint::NONE)) {
            if (shape->fill_shp->PtWinding(p) > 0 ) return item;
        }
        if (shape->stroke_shp && (shape->_stroke.paint.type() != NRArenaShape::Paint::NONE)) {
            if (shape->stroke_shp->PtWinding(p) > 0 ) return item;
        }
        if (delta > 1e-3) {
            if (shape->fill_shp && (shape->_fill.paint.type() != NRArenaShape::Paint::NONE)) {
                if (distanceLessThanOrEqual(shape->fill_shp, p, delta)) return item;
            }
            if (shape->stroke_shp && (shape->_stroke.paint.type() != NRArenaShape::Paint::NONE)) {
                if (distanceLessThanOrEqual(shape->stroke_shp, p, delta)) return item;
            }
        }
    } else {
        NRBPath bp;
        bp.path = shape->curve->bpath;
        double dist = NR_HUGE;
        int wind = 0;
        nr_path_matrix_point_bbox_wind_distance(&bp, shape->ctm, p, NULL, &wind, &dist, NR_EPSILON);
        if (shape->_fill.paint.type() != NRArenaShape::Paint::NONE) {
            if (!shape->style->fill_rule.computed) {
                if (wind != 0) return item;
            } else {
                if (wind & 0x1) return item;
            }
        }
        if (shape->_stroke.paint.type() != NRArenaShape::Paint::NONE) {
            /* fixme: We do not take stroke width into account here (Lauris) */
            if (dist < delta) return item;
        }
    }

    return NULL;
}

/**
 *
 *  Requests a render of the shape, then if the shape is already a curve it
 *  unrefs the old curve; if the new curve is valid it creates a copy of the
 *  curve and adds it to the shape.  Finally, it requests an update of the
 *  arena for the shape.
 */
void nr_arena_shape_set_path(NRArenaShape *shape, SPCurve *curve,bool justTrans)
{
    g_return_if_fail(shape != NULL);
    g_return_if_fail(NR_IS_ARENA_SHAPE(shape));

    if ( justTrans == false ) {
        // dirty cached versions
        if ( shape->cached_fill ) {
            delete shape->cached_fill;
            shape->cached_fill=NULL;
        }
        if ( shape->cached_stroke ) {
            delete shape->cached_stroke;
            shape->cached_stroke=NULL;
        }
    }

    nr_arena_item_request_render(NR_ARENA_ITEM(shape));

    if (shape->curve) {
        sp_curve_unref(shape->curve);
        shape->curve = NULL;
    }

    if (curve) {
        shape->curve = curve;
        sp_curve_ref(curve);
    }

    nr_arena_item_request_update(NR_ARENA_ITEM(shape), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void NRArenaShape::setFill(SPPaintServer *server) {
    _fill.paint.set(server);
    _invalidateCachedFill();
}

void NRArenaShape::setFill(SPColor const &color) {
    _fill.paint.set(color);
    _invalidateCachedFill();
}

void NRArenaShape::setFillOpacity(double opacity) {
    _fill.opacity = opacity;
    _invalidateCachedFill();
}

void NRArenaShape::setFillRule(NRArenaShape::FillRule rule) {
    _fill.rule = rule;
    _invalidateCachedFill();
}

void NRArenaShape::setStroke(SPPaintServer *server) {
    _stroke.paint.set(server);
    _invalidateCachedStroke();
}

void NRArenaShape::setStroke(SPColor const &color) {
    _stroke.paint.set(color);
    _invalidateCachedStroke();
}

void NRArenaShape::setStrokeOpacity(double opacity) {
    _stroke.opacity = opacity;
    _invalidateCachedStroke();
}

void NRArenaShape::setStrokeWidth(double width) {
    _stroke.width = width;
    _invalidateCachedStroke();
}

void NRArenaShape::setMitreLimit(double limit) {
    _stroke.mitre_limit = limit;
    _invalidateCachedStroke();
}

void NRArenaShape::setLineCap(NRArenaShape::CapType cap) {
    _stroke.cap = cap;
    _invalidateCachedStroke();
}

void NRArenaShape::setLineJoin(NRArenaShape::JoinType join) {
    _stroke.join = join;
    _invalidateCachedStroke();
}

/** nr_arena_shape_set_style
 *
 * Unrefs any existing style and ref's to the given one, then requests an update of the arena
 */
void
nr_arena_shape_set_style(NRArenaShape *shape, SPStyle *style)
{
    g_return_if_fail(shape != NULL);
    g_return_if_fail(NR_IS_ARENA_SHAPE(shape));

    if (style) sp_style_ref(style);
    if (shape->style) sp_style_unref(shape->style);
    shape->style = style;

    switch (style->fill.type) {
        case SP_PAINT_TYPE_NONE: {
            shape->setFill(NULL);
            break;
        }
        case SP_PAINT_TYPE_COLOR: {
            shape->setFill(style->fill.value.color);
            break;
        }
        case SP_PAINT_TYPE_PAINTSERVER: {
            shape->setFill(style->fill.value.paint.server);
            break;
        }
        default: {
            g_assert_not_reached();
        }
    }
    shape->setFillOpacity(SP_SCALE24_TO_FLOAT(style->fill_opacity.value));
    switch (style->fill_rule.computed) {
        case SP_WIND_RULE_EVENODD: {
            shape->setFillRule(NRArenaShape::EVEN_ODD);
            break;
        }
        case SP_WIND_RULE_NONZERO: {
            shape->setFillRule(NRArenaShape::NONZERO);
            break;
        }
        default: {
            g_assert_not_reached();
        }
    }

    switch (style->stroke.type) {
        case SP_PAINT_TYPE_NONE: {
            shape->setStroke(NULL);
            break;
        }
        case SP_PAINT_TYPE_COLOR: {
            shape->setStroke(style->stroke.value.color);
            break;
        }
        case SP_PAINT_TYPE_PAINTSERVER: {
            shape->setStroke(style->stroke.value.paint.server);
            break;
        }
        default: {
            g_assert_not_reached();
        }
    }
    shape->setStrokeWidth(style->stroke_width.computed);
    shape->setStrokeOpacity(SP_SCALE24_TO_FLOAT(style->stroke_opacity.value));
    switch (style->stroke_linecap.computed) {
        case SP_STROKE_LINECAP_ROUND: {
            shape->setLineCap(NRArenaShape::ROUND_CAP);
            break;
        }
        case SP_STROKE_LINECAP_SQUARE: {
            shape->setLineCap(NRArenaShape::SQUARE_CAP);
            break;
        }
        case SP_STROKE_LINECAP_BUTT: {
            shape->setLineCap(NRArenaShape::BUTT_CAP);
            break;
        }
        default: {
            g_assert_not_reached();
        }
    }
    switch (style->stroke_linejoin.computed) {
        case SP_STROKE_LINEJOIN_ROUND: {
            shape->setLineJoin(NRArenaShape::ROUND_JOIN);
            break;
        }
        case SP_STROKE_LINEJOIN_BEVEL: {
            shape->setLineJoin(NRArenaShape::BEVEL_JOIN);
            break;
        }
        case SP_STROKE_LINEJOIN_MITER: {
            shape->setLineJoin(NRArenaShape::MITRE_JOIN);
            break;
        }
        default: {
            g_assert_not_reached();
        }
    }
    shape->setMitreLimit(style->stroke_miterlimit.value);

    nr_arena_item_request_update(shape, NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void
nr_arena_shape_set_paintbox(NRArenaShape *shape, NRRect const *pbox)
{
    g_return_if_fail(shape != NULL);
    g_return_if_fail(NR_IS_ARENA_SHAPE(shape));
    g_return_if_fail(pbox != NULL);

    if ((pbox->x0 < pbox->x1) && (pbox->y0 < pbox->y1)) {
        shape->paintbox = *pbox;
    } else {
        /* fixme: We kill warning, although not sure what to do here (Lauris) */
        shape->paintbox.x0 = shape->paintbox.y0 = 0.0F;
        shape->paintbox.x1 = shape->paintbox.y1 = 256.0F;
    }

    nr_arena_item_request_update(shape, NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void NRArenaShape::setPaintBox(NR::Rect const &pbox)
{
    paintbox.x0 = pbox.min()[NR::X];
    paintbox.y0 = pbox.min()[NR::Y];
    paintbox.x1 = pbox.max()[NR::X];
    paintbox.y1 = pbox.max()[NR::Y];

    nr_arena_item_request_update(this, NR_ARENA_ITEM_STATE_ALL, FALSE);
}

static void
shape_run_A8_OR(raster_info &dest,void */*data*/,int st,float vst,int en,float ven)
{
    if ( st >= en ) return;
    if ( vst < 0 ) vst=0;
    if ( vst > 1 ) vst=1;
    if ( ven < 0 ) ven=0;
    if ( ven > 1 ) ven=1;
    float   sv=vst;
    float   dv=ven-vst;
    int     len=en-st;
    unsigned char*   d=(unsigned char*)dest.buffer;
    d+=(st-dest.startPix);
    if ( fabs(dv) < 0.001 ) {
        if ( vst > 0.999 ) {
            /* Simple copy */
            while (len > 0) {
                d[0] = 255;
                d += 1;
                len -= 1;
            }
        } else {
            sv*=256;
            unsigned int c0_24=(int)sv;
            c0_24&=0xFF;
            while (len > 0) {
                unsigned int da;
                /* Draw */
                da = NR_A7(c0_24,d[0]);
                d[0] = NR_PREMUL_SINGLE(da);
                d += 1;
                len -= 1;
            }
        }
    } else {
        if ( en <= st+1 ) {
            sv=0.5*(vst+ven);
            sv*=256;
            unsigned int c0_24=(int)sv;
            c0_24&=0xFF;
            unsigned int da;
            /* Draw */
            da = NR_A7(c0_24,d[0]);
            d[0] = NR_PREMUL_SINGLE(da);
        } else {
            dv/=len;
            sv+=0.5*dv; // correction trapezoidale
            sv*=16777216;
            dv*=16777216;
            int c0_24 = static_cast<int>(CLAMP(sv, 0, 16777216));
            int s0_24 = static_cast<int>(dv);
            while (len > 0) {
                unsigned int ca, da;
                /* Draw */
                ca = c0_24 >> 16;
                if ( ca > 255 ) ca=255;
                da = NR_A7(ca,d[0]);
                d[0] = NR_PREMUL_SINGLE(da);
                d += 1;
                c0_24 += s0_24;
                c0_24 = CLAMP(c0_24, 0, 16777216);
                len -= 1;
            }
        }
    }
}

void nr_pixblock_render_shape_mask_or(NRPixBlock &m,Shape* theS)
{
    theS->CalcBBox();
    float l = theS->leftX, r = theS->rightX, t = theS->topY, b = theS->bottomY;
    int    il,ir,it,ib;
    il=(int)floor(l);
    ir=(int)ceil(r);
    it=(int)floor(t);
    ib=(int)ceil(b);

    if ( il >= m.area.x1 || ir <= m.area.x0 || it >= m.area.y1 || ib <= m.area.y0 ) return;
    if ( il < m.area.x0 ) il=m.area.x0;
    if ( it < m.area.y0 ) it=m.area.y0;
    if ( ir > m.area.x1 ) ir=m.area.x1;
    if ( ib > m.area.y1 ) ib=m.area.y1;

    /* This is the FloatLigne version.  See svn (prior to Apr 2006) for versions using BitLigne or direct BitLigne. */
    int    curPt;
    float  curY;
    theS->BeginQuickRaster(curY, curPt);

    FloatLigne *theI = new FloatLigne();
    IntLigne *theIL = new IntLigne();

    theS->DirectQuickScan(curY, curPt, (float) it, true, 1.0);

    char *mdata = (char*)m.data.px;
    if ( m.size == NR_PIXBLOCK_SIZE_TINY ) mdata=(char*)m.data.p;
    uint32_t *ligStart = ((uint32_t*)(mdata + ((il - m.area.x0) + m.rs * (it - m.area.y0))));
    for (int y = it; y < ib; y++) {
        theI->Reset();
        theS->QuickScan(curY, curPt, ((float)(y+1)), theI, 1.0);
        theI->Flatten();
        theIL->Copy(theI);

        raster_info  dest;
        dest.startPix=il;
        dest.endPix=ir;
        dest.sth=il;
        dest.stv=y;
        dest.buffer=ligStart;
        theIL->Raster(dest, NULL, shape_run_A8_OR);
        ligStart=((uint32_t*)(((char*)ligStart)+m.rs));
    }
    theS->EndQuickRaster();
    delete theI;
    delete theIL;
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

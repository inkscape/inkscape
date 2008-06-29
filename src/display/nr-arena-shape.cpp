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



#include <display/canvas-arena.h>
#include <display/nr-arena.h>
#include <display/nr-arena-shape.h>
#include "display/curve.h"
#include <libnr/n-art-bpath.h>
#include <libnr/nr-path.h>
#include <libnr/nr-pixops.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-blit.h>
#include <libnr/nr-convert2geom.h>
#include <2geom/pathvector.h>
#include <2geom/curves.h>
#include <livarot/Path.h>
#include <livarot/float-line.h>
#include <livarot/int-line.h>
#include <style.h>
#include "prefs-utils.h"
#include "inkscape-cairo.h"
#include "helper/geom.h"
#include "sp-filter.h"
#include "sp-filter-reference.h"
#include "display/nr-filter.h"
#include <typeinfo>
#include <cairo.h>

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
static unsigned int nr_arena_shape_render(cairo_t *ct, NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags);
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

/**
 * Initializes the arena shape, setting all parameters to null, 0, false,
 * or other defaults
 */
static void
nr_arena_shape_init(NRArenaShape *shape)
{
    shape->curve = NULL;
    shape->style = NULL;
    shape->paintbox.x0 = shape->paintbox.y0 = 0.0F;
    shape->paintbox.x1 = shape->paintbox.y1 = 256.0F;

    shape->ctm.set_identity();
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
    shape->cached_fctm.set_identity();
    shape->cached_sctm.set_identity();

    shape->markers = NULL;

    shape->last_pick = NULL;
    shape->repick_after = 0;
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
    if (shape->curve) shape->curve->unref();

    ((NRObjectClass *) shape_parent_class)->finalize(object);
}

/**
 * Retrieves the markers from the item
 */
static NRArenaItem *
nr_arena_shape_children(NRArenaItem *item)
{
    NRArenaShape *shape = (NRArenaShape *) item;

    return shape->markers;
}

/**
 * Attaches child to item, and if ref is not NULL, sets it and ref->next as
 * the prev and next items.  If ref is NULL, then it sets the item's markers
 * as the next items.
 */
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

/**
 * Removes child from the shape.  If there are no prev items in 
 * the child, it sets items' markers to the next item in the child.
 */
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

/**
 * Detaches child from item, and if there are no previous items in child, it 
 * sets item's markers to the child.  It then attaches the child back onto the item.
 * If ref is null, it sets the markers to be the next item, otherwise it uses
 * the next/prev items in ref.
 */
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
void nr_arena_shape_add_bboxes(NRArenaShape* shape, Geom::Rect &bbox);

/**
 * Updates the arena shape 'item' and all of its children, including the markers.
 */
static guint
nr_arena_shape_update(NRArenaItem *item, NRRectL *area, NRGC *gc, guint state, guint reset)
{
    Geom::Rect boundingbox;

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
                boundingbox = bounds_exact_transformed(shape->curve->get_pathvector(), to_2geom(gc->transform));
                item->bbox.x0 = (gint32)(boundingbox[0][0] - 1.0F);
                item->bbox.y0 = (gint32)(boundingbox[1][0] - 1.0F);
                item->bbox.x1 = (gint32)(boundingbox[0][1] + 1.9999F);
                item->bbox.y1 = (gint32)(boundingbox[1][1] + 1.9999F);
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
    boundingbox[0][0] = boundingbox[1][0] = NR_HUGE;
    boundingbox[0][1] = boundingbox[1][1] = -NR_HUGE;

    bool outline = (NR_ARENA_ITEM(shape)->arena->rendermode == Inkscape::RENDERMODE_OUTLINE);

    if (shape->curve) {
        boundingbox = bounds_exact_transformed(shape->curve->get_pathvector(), to_2geom(gc->transform));

        if (shape->_stroke.paint.type() != NRArenaShape::Paint::NONE || outline) {
            float width, scale;
            scale = NR::expansion(gc->transform);
            width = MAX(0.125, shape->_stroke.width * scale);
            if ( fabs(shape->_stroke.width * scale) > 0.01 ) { // sinon c'est 0=oon veut pas de bord
                boundingbox.expandBy(width);
            }
            // those pesky miters, now
            float miterMax=width*shape->_stroke.mitre_limit;
            if ( miterMax > 0.01 ) {
                // grunt mode. we should compute the various miters instead (one for each point on the curve)
                boundingbox.expandBy(miterMax);
            }
        }
    } else {
    }
    shape->approx_bbox.x0 = (gint32)(boundingbox[0][0] - 1.0F);
    shape->approx_bbox.y0 = (gint32)(boundingbox[1][0] - 1.0F);
    shape->approx_bbox.x1 = (gint32)(boundingbox[0][1] + 1.9999F);
    shape->approx_bbox.y1 = (gint32)(boundingbox[1][1] + 1.9999F);
    if ( area && nr_rect_l_test_intersect(area, &shape->approx_bbox) ) shape->delayed_shp=false;

    /* Release state data */
    if (TRUE || !NR::transform_equalp(gc->transform, shape->ctm, NR_EPSILON)) {
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

    if (!shape->curve || 
        !shape->style ||
        shape->curve->is_empty() ||
        (( shape->_fill.paint.type() == NRArenaShape::Paint::NONE ) &&
         ( shape->_stroke.paint.type() == NRArenaShape::Paint::NONE && !outline) ))
    {
        item->bbox = shape->approx_bbox;
        return NR_ARENA_ITEM_STATE_ALL;
    }

    /* Build state data */
    if ( shape->delayed_shp ) {
        item->bbox=shape->approx_bbox;
    } else {
        nr_arena_shape_update_stroke(shape, gc, area);
        nr_arena_shape_update_fill(shape, gc, area);

        boundingbox[0][0] = boundingbox[0][1] = boundingbox[1][0] = boundingbox[1][1] = 0.0;
        nr_arena_shape_add_bboxes(shape, boundingbox);

        shape->approx_bbox.x0 = (gint32)(boundingbox[0][0] - 1.0F);
        shape->approx_bbox.y0 = (gint32)(boundingbox[1][0] - 1.0F);
        shape->approx_bbox.x1 = (gint32)(boundingbox[0][1] + 1.9999F);
        shape->approx_bbox.y1 = (gint32)(boundingbox[1][1] + 1.9999F);
    }

    if (boundingbox.isEmpty())
        return NR_ARENA_ITEM_STATE_ALL;

    item->bbox.x0 = (gint32)(boundingbox[0][0] - 1.0F);
    item->bbox.y0 = (gint32)(boundingbox[1][0] - 1.0F);
    item->bbox.x1 = (gint32)(boundingbox[0][1] + 1.0F);
    item->bbox.y1 = (gint32)(boundingbox[1][1] + 1.0F);
    nr_arena_request_render_rect(item->arena, &item->bbox);

    item->render_opacity = TRUE;
    if ( shape->_fill.paint.type() == NRArenaShape::Paint::SERVER ) {
        if (gc && gc->parent) {
            shape->fill_painter = sp_paint_server_painter_new(shape->_fill.paint.server(),
                                                              gc->transform, gc->parent->transform,
                                                              &shape->paintbox);
        }
        item->render_opacity = FALSE;
    }
    if ( shape->_stroke.paint.type() == NRArenaShape::Paint::SERVER ) {
        if (gc && gc->parent) {
            shape->stroke_painter = sp_paint_server_painter_new(shape->_stroke.paint.server(),
                                                                gc->transform, gc->parent->transform,
                                                                &shape->paintbox);
        }
        item->render_opacity = FALSE;
    }
    if (  (shape->_fill.paint.type() != NRArenaShape::Paint::NONE && 
           shape->_stroke.paint.type() != NRArenaShape::Paint::NONE)
          || (shape->markers)
        )
    {
        // don't merge item opacity with paint opacity if there is a stroke on the fill, or markers on stroke
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

/* returns true if the pathvector has a region that needs fill.
 * is for optimizing purposes, so should be fast and can falsely return true. 
 * CANNOT falsely return false. */
static bool has_inner_area(Geom::PathVector const & pv) {
    // return false for the cases where there is surely no region to be filled
    if (pv.empty())
        return false;

    if ( (pv.size() == 1) && (pv.front().size() <= 1) ) {
        // vector has only one path with only one segment, see if that's a non-curve segment: that would mean no internal region
        Geom::Curve const & c = pv.front().front();
        if ( typeid(c) == typeid(Geom::LineSegment) )
            return false;
        if ( typeid(c) == typeid(Geom::HLineSegment) )
            return false;
        if ( typeid(c) == typeid(Geom::VLineSegment) )
            return false;
    }

    return true; //too costly to see if it has region to be filled, so return true.
}

/** force_shape is used for clipping paths, when we need the shape for clipping even if it's not filled */
void
nr_arena_shape_update_fill(NRArenaShape *shape, NRGC *gc, NRRectL *area, bool force_shape)
{
    if ((shape->_fill.paint.type() != NRArenaShape::Paint::NONE || force_shape) &&
//        ((shape->curve->get_length() > 2) || (SP_CURVE_BPATH(shape->curve)[1].code == NR_CURVETO)) ) {  // <-- this used to be the old code, i think it has to determine that the path has a sort of 'internal region' where fill would occur
          has_inner_area(shape->curve->get_pathvector()) ) {
        if (TRUE || !shape->fill_shp) {
            NR::Matrix  cached_to_new = NR::identity();
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
                    Geom::Matrix tempMat(to_2geom(gc->transform));
                    thePath->LoadPathVector(shape->curve->get_pathvector(), tempMat, true);
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

    float const scale = NR::expansion(gc->transform);

    bool outline = (NR_ARENA_ITEM(shape)->arena->rendermode == Inkscape::RENDERMODE_OUTLINE);

    if (outline) {
        // cairo does not need the livarot path for rendering
        return; 
    }

    // after switching normal stroke rendering to cairo too, optimize this: lower tolerance, disregard dashes 
    // (since it will only be used for picking, not for rendering)

    if (outline ||
        ((shape->_stroke.paint.type() != NRArenaShape::Paint::NONE) &&
         ( fabs(shape->_stroke.width * scale) > 0.01 ))) { // sinon c'est 0=oon veut pas de bord

        float style_width = MAX(0.125, shape->_stroke.width * scale);
        float width;
        if (outline) {
            width = 0.5; // 1 pixel wide, independent of zoom
        } else {
            width = style_width;
        }

        NR::Matrix  cached_to_new = NR::identity();

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
            if (0 != isometry && width != shape->cached_width) { 
                // if this happens without setting style, we have just switched to outline or back
                isometry = 0; 
            } 
        }

        if ( isometry == 0 ) {
            if ( shape->cached_stroke == NULL ) shape->cached_stroke=new Shape;
            shape->cached_stroke->Reset();
            Path*  thePath = new Path;
            Shape* theShape = new Shape;
            {
                Geom::Matrix   tempMat( to_2geom(gc->transform) );
                thePath->LoadPathVector(shape->curve->get_pathvector(), tempMat, true);
            }

            // add some padding to the rendering area, so clipped path does not go into a render area
            NRRectL padded_area = *area;
            padded_area.x0 -= (NR::ICoord)width;
            padded_area.x1 += (NR::ICoord)width;
            padded_area.y0 -= (NR::ICoord)width;
            padded_area.y1 += (NR::ICoord)width;
            if ((style->stroke_dash.n_dash && !outline) || is_inner_area(padded_area, NR_ARENA_ITEM(shape)->bbox)) {
                thePath->Convert((outline) ? 4.0 : 1.0);
                shape->cached_spartialy = false;
            }
            else {
                thePath->Convert(&padded_area, (outline) ? 4.0 : 1.0);
                shape->cached_spartialy = true;
            }

            if (style->stroke_dash.n_dash && !outline) {
                thePath->DashPolylineFromStyle(style, scale, 1.0);
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

            if (outline) {
                butt = butt_straight;
                join = join_straight;
            }

            thePath->Stroke(theShape, false, 0.5*width, join, butt,
                            0.5*width*shape->_stroke.mitre_limit);


            if (outline) {
                // speeds it up, but uses evenodd for the stroke shape (which does not matter for 1-pixel wide outline)
                shape->cached_stroke->Copy(theShape);
            } else {
                shape->cached_stroke->ConvertToShape(theShape, fill_nonZero);
            }

            shape->cached_width = width;

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
nr_arena_shape_add_bboxes(NRArenaShape* shape, Geom::Rect &bbox)
{
    /* TODO: are these two if's mutually exclusive? ( i.e. "shape->stroke_shp <=> !shape->fill_shp" )
     * if so, then this can be written much more compact ! */

    if ( shape->stroke_shp ) {
        Shape *larger = shape->stroke_shp;
        larger->CalcBBox();
        larger->leftX   = floor(larger->leftX);
        larger->rightX  = ceil(larger->rightX);
        larger->topY    = floor(larger->topY);
        larger->bottomY = ceil(larger->bottomY);
        Geom::Rect stroke_bbox( Geom::Interval(larger->leftX, larger->rightX),
                                Geom::Interval(larger->topY,  larger->bottomY) );
        bbox.unionWith(stroke_bbox);
    }

    if ( shape->fill_shp ) {
        Shape *larger = shape->fill_shp;
        larger->CalcBBox();
        larger->leftX   = floor(larger->leftX);
        larger->rightX  = ceil(larger->rightX);
        larger->topY    = floor(larger->topY);
        larger->bottomY = ceil(larger->bottomY);
        Geom::Rect fill_bbox( Geom::Interval(larger->leftX, larger->rightX),
                              Geom::Interval(larger->topY,  larger->bottomY) );
        bbox.unionWith(fill_bbox);
    }
}

// cairo outline rendering:
static unsigned int
cairo_arena_shape_render_outline(cairo_t *ct, NRArenaItem *item, NR::Maybe<NR::Rect> area)
{
    NRArenaShape *shape = NR_ARENA_SHAPE(item);

    if (!ct) 
        return item->state;

    guint32 rgba = NR_ARENA_ITEM(shape)->arena->outlinecolor;
    // FIXME: we use RGBA buffers but cairo writes BGRA (on i386), so we must cheat 
    // by setting color channels in the "wrong" order
    cairo_set_source_rgba(ct, SP_RGBA32_B_F(rgba), SP_RGBA32_G_F(rgba), SP_RGBA32_R_F(rgba), SP_RGBA32_A_F(rgba));

    cairo_set_line_width(ct, 0.5);
    cairo_set_tolerance(ct, 1.25); // low quality, but good enough for outline mode
    cairo_new_path(ct);

    feed_pathvector_to_cairo (ct, shape->curve->get_pathvector(), to_2geom(shape->ctm), area, true, 0);

    cairo_stroke(ct);

    return item->state;
}

// cairo stroke rendering (flat color only so far!):
// works on canvas, but wrongs the colors in nonpremul buffers: icons and png export
// (need to switch them to premul before this can be enabled)
void
cairo_arena_shape_render_stroke(NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
    NRArenaShape *shape = NR_ARENA_SHAPE(item);
    SPStyle const *style = shape->style;

    float const scale = NR::expansion(shape->ctm);

    if (fabs(shape->_stroke.width * scale) < 0.01)
        return;

    cairo_t *ct = nr_create_cairo_context (area, pb);

    if (!ct)
        return;

    guint32 rgba;
    if ( item->render_opacity ) {
        rgba = shape->_stroke.paint.color().toRGBA32( shape->_stroke.opacity *
                                                      SP_SCALE24_TO_FLOAT(style->opacity.value) );
    } else {
        rgba = shape->_stroke.paint.color().toRGBA32( shape->_stroke.opacity );
    }

    // FIXME: we use RGBA buffers but cairo writes BGRA (on i386), so we must cheat 
    // by setting color channels in the "wrong" order
    cairo_set_source_rgba(ct, SP_RGBA32_B_F(rgba), SP_RGBA32_G_F(rgba), SP_RGBA32_R_F(rgba), SP_RGBA32_A_F(rgba));

    float style_width = MAX(0.125, shape->_stroke.width * scale);
    cairo_set_line_width(ct, style_width);

    switch (shape->_stroke.cap) {
        case NRArenaShape::BUTT_CAP:
            cairo_set_line_cap(ct, CAIRO_LINE_CAP_BUTT);
            break;
        case NRArenaShape::ROUND_CAP:
            cairo_set_line_cap(ct, CAIRO_LINE_CAP_ROUND);
            break;
        case NRArenaShape::SQUARE_CAP:
            cairo_set_line_cap(ct, CAIRO_LINE_CAP_SQUARE);
            break;
    }
    switch (shape->_stroke.join) {
        case NRArenaShape::MITRE_JOIN:
            cairo_set_line_join(ct, CAIRO_LINE_JOIN_MITER);
            break;
        case NRArenaShape::ROUND_JOIN:
            cairo_set_line_join(ct, CAIRO_LINE_JOIN_ROUND);
            break;
        case NRArenaShape::BEVEL_JOIN:
            cairo_set_line_join(ct, CAIRO_LINE_JOIN_BEVEL);
            break;
    }

    cairo_set_miter_limit (ct, style->stroke_miterlimit.value);

    if (style->stroke_dash.n_dash) {
        NRVpathDash dash;
        dash.offset = style->stroke_dash.offset * scale;
        dash.n_dash = style->stroke_dash.n_dash;
        dash.dash = g_new(double, dash.n_dash);
        for (int i = 0; i < dash.n_dash; i++) {
            dash.dash[i] = style->stroke_dash.dash[i] * scale;
        }
        cairo_set_dash (ct, dash.dash, dash.n_dash, dash.offset);
        g_free(dash.dash);
    }

    cairo_set_tolerance(ct, 0.1);
    cairo_new_path(ct);

    feed_pathvector_to_cairo (ct, shape->curve->get_pathvector(), to_2geom(shape->ctm), area->upgrade(), true, style_width);

    cairo_stroke(ct);

    cairo_surface_t *cst = cairo_get_target(ct);
    cairo_destroy (ct);
    cairo_surface_finish (cst);
    cairo_surface_destroy (cst);

    pb->empty = FALSE;
}


/**
 * Renders the item.  Markers are just composed into the parent buffer.
 */
static unsigned int
nr_arena_shape_render(cairo_t *ct, NRArenaItem *item, NRRectL *area, NRPixBlock *pb, unsigned int flags)
{
    NRArenaShape *shape = NR_ARENA_SHAPE(item);

    if (!shape->curve) return item->state;
    if (!shape->style) return item->state;

    bool outline = (NR_ARENA_ITEM(shape)->arena->rendermode == Inkscape::RENDERMODE_OUTLINE);

    if (outline) { // cairo outline rendering

        pb->empty = FALSE;
        unsigned int ret = cairo_arena_shape_render_outline (ct, item, (&pb->area)->upgrade());
        if (ret & NR_ARENA_ITEM_STATE_INVALID) return ret;

    } else {

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
    if (shape->fill_shp) {
        NRPixBlock m;
        guint32 rgba;

        nr_pixblock_setup_fast(&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);

        // if memory allocation failed, abort render
        if (m.size != NR_PIXBLOCK_SIZE_TINY && m.data.px == NULL) {
            nr_pixblock_release (&m);
            return (item->state);
        }

        m.visible_area = pb->visible_area;
        nr_pixblock_render_shape_mask_or(m,shape->fill_shp);
        m.empty = FALSE;

        if (shape->_fill.paint.type() == NRArenaShape::Paint::NONE) {
            // do not render fill in any way
        } else if (shape->_fill.paint.type() == NRArenaShape::Paint::COLOR) {
            if ( item->render_opacity ) {
                rgba = shape->_fill.paint.color().toRGBA32( shape->_fill.opacity *
                                                            SP_SCALE24_TO_FLOAT(style->opacity.value) );
            } else {
                rgba = shape->_fill.paint.color().toRGBA32( shape->_fill.opacity );
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

    if (shape->stroke_shp && shape->_stroke.paint.type() == NRArenaShape::Paint::COLOR) {

        // cairo_arena_shape_render_stroke(item, area, pb);

        guint32 rgba;
        NRPixBlock m;

        nr_pixblock_setup_fast(&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);

        // if memory allocation failed, abort render
        if (m.size != NR_PIXBLOCK_SIZE_TINY && m.data.px == NULL) {
            nr_pixblock_release (&m);
            return (item->state);
        }

        m.visible_area = pb->visible_area;
        nr_pixblock_render_shape_mask_or(m, shape->stroke_shp);
        m.empty = FALSE;

            if ( item->render_opacity ) {
                rgba = shape->_stroke.paint.color().toRGBA32( shape->_stroke.opacity *
                                                              SP_SCALE24_TO_FLOAT(style->opacity.value) );
            } else {
                rgba = shape->_stroke.paint.color().toRGBA32( shape->_stroke.opacity );
            }
            nr_blit_pixblock_mask_rgba32(pb, &m, rgba);
            pb->empty = FALSE;

        nr_pixblock_release(&m);

    } else if (shape->stroke_shp && shape->_stroke.paint.type() == NRArenaShape::Paint::SERVER) {

        NRPixBlock m;

        nr_pixblock_setup_fast(&m, NR_PIXBLOCK_MODE_A8, area->x0, area->y0, area->x1, area->y1, TRUE);

        // if memory allocation failed, abort render
        if (m.size != NR_PIXBLOCK_SIZE_TINY && m.data.px == NULL) {
            nr_pixblock_release (&m);
            return (item->state);
        }

        m.visible_area = pb->visible_area; 
        nr_pixblock_render_shape_mask_or(m, shape->stroke_shp);
        m.empty = FALSE;

        if (shape->stroke_painter) {
            nr_arena_render_paintserver_fill(pb, area, shape->stroke_painter, shape->_stroke.opacity, &m);
        }

        nr_pixblock_release(&m);
    }

    } // non-cairo non-outline branch

    /* Render markers into parent buffer */
    for (NRArenaItem *child = shape->markers; child != NULL; child = child->next) {
        unsigned int ret = nr_arena_item_invoke_render(ct, child, area, pb, flags);
        if (ret & NR_ARENA_ITEM_STATE_INVALID) return ret;
    }

    return item->state;
}


// cairo clipping: this basically works except for the stride-must-be-divisible-by-4 cairo bug;
// reenable this when the bug is fixed and remove the rest of this function
// TODO
#if defined(DEADCODE) && !defined(DEADCODE)
static guint
cairo_arena_shape_clip(NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
    NRArenaShape *shape = NR_ARENA_SHAPE(item);
    if (!shape->curve) return item->state;

        cairo_t *ct = nr_create_cairo_context (area, pb);

        if (!ct)
            return item->state;

        cairo_set_source_rgba(ct, 0, 0, 0, 1);

        cairo_new_path(ct);

        feed_pathvector_to_cairo (ct, shape->curve->get_pathvector(), to_2geom(shape->ctm), (area)->upgrade(), false, 0);

        cairo_fill(ct);

        cairo_surface_t *cst = cairo_get_target(ct);
        cairo_destroy (ct);
        cairo_surface_finish (cst);
        cairo_surface_destroy (cst);

        pb->empty = FALSE;

        return item->state;
}
#endif //defined(DEADCODE) && !defined(DEADCODE)


static guint
nr_arena_shape_clip(NRArenaItem *item, NRRectL *area, NRPixBlock *pb)
{
    //return cairo_arena_shape_clip(item, area, pb);

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

        // if memory allocation failed, abort 
        if (m.size != NR_PIXBLOCK_SIZE_TINY && m.data.px == NULL) {
            nr_pixblock_release (&m);
            return (item->state);
        }

        m.visible_area = pb->visible_area; 
        nr_pixblock_render_shape_mask_or(m,shape->fill_shp);

        for (int y = area->y0; y < area->y1; y++) {
            unsigned char *s, *d;
            s = NR_PIXBLOCK_PX(&m) + (y - area->y0) * m.rs;
            d = NR_PIXBLOCK_PX(pb) + (y - area->y0) * pb->rs;
            for (int x = area->x0; x < area->x1; x++) {
                *d = NR_COMPOSEA_111(*s, *d);
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

    if (shape->repick_after > 0)
        shape->repick_after--;

    if (shape->repick_after > 0) // we are a slow, huge path. skip this pick, returning what was returned last time
        return shape->last_pick;

    if (!shape->curve) return NULL;
    if (!shape->style) return NULL;

    bool outline = (NR_ARENA_ITEM(shape)->arena->rendermode == Inkscape::RENDERMODE_OUTLINE);

    if (SP_SCALE24_TO_FLOAT(shape->style->opacity.value) == 0 && !outline) 
        // fully transparent, no pick unless outline mode
        return NULL;

    GTimeVal tstart, tfinish;
    g_get_current_time (&tstart);

    double width;
    if (outline) {
        width = 0.5;
    } else if (shape->_stroke.paint.type() != NRArenaShape::Paint::NONE && shape->_stroke.opacity > 1e-3) {
        float const scale = NR::expansion(shape->ctm);
        width = MAX(0.125, shape->_stroke.width * scale) / 2;
    } else {
        width = 0;
    }

    double dist = NR_HUGE;
    int wind = 0;
    bool needfill = (shape->_fill.paint.type() != NRArenaShape::Paint::NONE 
             && shape->_fill.opacity > 1e-3 && !outline);

    if (item->arena->canvasarena) {
        Geom::Rect viewbox = to_2geom(item->arena->canvasarena->item.canvas->getViewbox());
        viewbox.expandBy (width);
        pathv_matrix_point_bbox_wind_distance(shape->curve->get_pathvector(), to_2geom(shape->ctm), to_2geom(p), NULL, needfill? &wind : NULL, &dist, 0.5, &viewbox);
    } else {
        pathv_matrix_point_bbox_wind_distance(shape->curve->get_pathvector(), to_2geom(shape->ctm), to_2geom(p), NULL, needfill? &wind : NULL, &dist, 0.5, NULL);
    }

    g_get_current_time (&tfinish);
    glong this_pick = (tfinish.tv_sec - tstart.tv_sec) * 1000000 + (tfinish.tv_usec - tstart.tv_usec);
    //g_print ("pick time %lu\n", this_pick);

    if (this_pick > 10000) { // slow picking, remember to skip several new picks
        shape->repick_after = this_pick / 5000;
    }

    // covered by fill?
    if (needfill) {
        if (!shape->style->fill_rule.computed) {
            if (wind != 0) {
                shape->last_pick = item;
                return item;
            }
        } else {
            if (wind & 0x1) {
                shape->last_pick = item;
                return item;
            }
        }
    }

    // close to the edge, as defined by strokewidth and delta?
    // this ignores dashing (as if the stroke is solid) and always works as if caps are round
    if (needfill || width > 0) { // if either fill or stroke visible,
        if ((dist - width) < delta) {
            shape->last_pick = item;
            return item;
        }
    }

    // if not picked on the shape itself, try its markers
    for (NRArenaItem *child = shape->markers; child != NULL; child = child->next) {
        NRArenaItem *ret = nr_arena_item_invoke_pick(child, p, delta, 0);
        if (ret) {
            shape->last_pick = item;
            return item;
        }
    }

    shape->last_pick = NULL;
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
        shape->curve->unref();
        shape->curve = NULL;
    }

    if (curve) {
        shape->curve = curve;
        curve->ref();
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

    if ( style->fill.isPaintserver() ) {
        shape->setFill(style->getFillPaintServer());
    } else if ( style->fill.isColor() ) {
        shape->setFill(style->fill.value.color);
    } else if ( style->fill.isNone() ) {
        shape->setFill(NULL);
    } else {
        g_assert_not_reached();
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

    if ( style->stroke.isPaintserver() ) {
        shape->setStroke(style->getStrokePaintServer());
    } else if ( style->stroke.isColor() ) {
        shape->setStroke(style->stroke.value.color);
    } else if ( style->stroke.isNone() ) {
        shape->setStroke(NULL);
    } else {
        g_assert_not_reached();
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

    //if shape has a filter
    if (style->filter.set && style->getFilter()) {
        if (!shape->filter) {
            int primitives = sp_filter_primitive_count(SP_FILTER(style->getFilter()));
            shape->filter = new NR::Filter(primitives);
        }
        sp_filter_build_renderer(SP_FILTER(style->getFilter()), shape->filter);
    } else {
        //no filter set for this shape
        delete shape->filter;
        shape->filter = NULL;
    }

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
                /* Draw */
                d[0] = NR_COMPOSEA_111(c0_24,d[0]);
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
            /* Draw */
            d[0] = NR_COMPOSEA_111(c0_24,d[0]);
        } else {
            dv/=len;
            sv+=0.5*dv; // correction trapezoidale
            sv*=16777216;
            dv*=16777216;
            int c0_24 = static_cast<int>(CLAMP(sv, 0, 16777216));
            int s0_24 = static_cast<int>(dv);
            while (len > 0) {
                unsigned int ca;
                /* Draw */
                ca = c0_24 >> 16;
                if ( ca > 255 ) ca=255;
                d[0] = NR_COMPOSEA_111(ca,d[0]);
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

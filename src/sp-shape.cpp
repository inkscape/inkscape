#define __SP_SHAPE_C__

/*
 * Base class for shapes, including <path> element
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2004 John Cliff
 * Copyright (C) 2007-2008 Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <libnr/n-art-bpath.h>
#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-translate-ops.h>
#include <libnr/nr-scale-matrix-ops.h>

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#include "macros.h"
#include "display/nr-arena-shape.h"
#include "print.h"
#include "document.h"
#include "style.h"
#include "marker.h"
#include "sp-path.h"
#include "prefs-utils.h"
#include "attributes.h"

#include "live_effects/lpeobject.h"
#include "uri.h"
#include "extract-uri.h"
#include "uri-references.h"
#include "bad-uri-exception.h"
#include "xml/repr.h"

#include "util/mathfns.h" // for triangle_area()

#define noSHAPE_VERBOSE

static void sp_shape_class_init (SPShapeClass *klass);
static void sp_shape_init (SPShape *shape);
static void sp_shape_finalize (GObject *object);

static void sp_shape_build (SPObject * object, SPDocument * document, Inkscape::XML::Node * repr);
static void sp_shape_release (SPObject *object);

static void sp_shape_set(SPObject *object, unsigned key, gchar const *value);
static void sp_shape_update (SPObject *object, SPCtx *ctx, unsigned int flags);
static void sp_shape_modified (SPObject *object, unsigned int flags);
static Inkscape::XML::Node *sp_shape_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

static void sp_shape_bbox(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags);
void sp_shape_print (SPItem * item, SPPrintContext * ctx);
static NRArenaItem *sp_shape_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_shape_hide (SPItem *item, unsigned int key);
static void sp_shape_snappoints (SPItem const *item, SnapPointsIter p);

static void sp_shape_update_marker_view (SPShape *shape, NRArenaItem *ai);

static SPLPEItemClass *parent_class;

/**
 * Registers the SPShape class with Gdk and returns its type number.
 */
GType
sp_shape_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPShapeClass),
			NULL, NULL,
			(GClassInitFunc) sp_shape_class_init,
			NULL, NULL,
			sizeof (SPShape),
			16,
			(GInstanceInitFunc) sp_shape_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_LPE_ITEM, "SPShape", &info, (GTypeFlags)0);
	}
	return type;
}

/**
 * Initializes a SPShapeClass object.  Establishes the function pointers to the class'
 * member routines in the class vtable, and sets pointers to parent classes.
 */
static void
sp_shape_class_init (SPShapeClass *klass)
{
    GObjectClass *gobject_class;
	SPObjectClass *sp_object_class;
	SPItemClass * item_class;
    SPLPEItemClass * lpe_item_class;

    gobject_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;

	parent_class = (SPLPEItemClass *)g_type_class_peek_parent (klass);

    gobject_class->finalize = sp_shape_finalize;

	sp_object_class->build = sp_shape_build;
	sp_object_class->release = sp_shape_release;
    sp_object_class->set = sp_shape_set;
	sp_object_class->update = sp_shape_update;
	sp_object_class->modified = sp_shape_modified;
    sp_object_class->write = sp_shape_write;

	item_class->bbox = sp_shape_bbox;
	item_class->print = sp_shape_print;
	item_class->show = sp_shape_show;
	item_class->hide = sp_shape_hide;
    item_class->snappoints = sp_shape_snappoints;
    lpe_item_class->update_patheffect = NULL;

    klass->set_shape = NULL;
}

/**
 * Initializes an SPShape object.
 */
static void
sp_shape_init (SPShape *shape)
{
    for ( int i = 0 ; i < SP_MARKER_LOC_QTY ; i++ ) {
        new (&shape->release_connect[i]) sigc::connection();
        new (&shape->modified_connect[i]) sigc::connection();
    }
}

static void
sp_shape_finalize (GObject *object)
{
    SPShape *shape=(SPShape *)object;

    for ( int i = 0 ; i < SP_MARKER_LOC_QTY ; i++ ) {
        shape->release_connect[i].disconnect();
        shape->release_connect[i].~connection();
        shape->modified_connect[i].disconnect();
        shape->modified_connect[i].~connection();
    }

    if (((GObjectClass *) (parent_class))->finalize) {
        (* ((GObjectClass *) (parent_class))->finalize)(object);
    }
}

/**
 * Virtual build callback for SPMarker.
 *
 * This is to be invoked immediately after creation of an SPShape.
 *
 * \see sp_object_build()
 */
static void
sp_shape_build (SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) (parent_class))->build) {
       (*((SPObjectClass *) (parent_class))->build) (object, document, repr);
    }
}

/**
 * Removes, releases and unrefs all children of object
 *
 * This is the inverse of sp_shape_build().  It must be invoked as soon
 * as the shape is removed from the tree, even if it is still referenced
 * by other objects.  This routine also disconnects/unrefs markers and
 * curves attached to it.
 *
 * \see sp_object_release()
 */
static void
sp_shape_release (SPObject *object)
{
	SPItem *item;
	SPShape *shape;
	SPItemView *v;
	int i;

	item = (SPItem *) object;
	shape = (SPShape *) object;

	for (i=SP_MARKER_LOC_START; i<SP_MARKER_LOC_QTY; i++) {
	  if (shape->marker[i]) {
	    sp_signal_disconnect_by_data (shape->marker[i], object);
	    for (v = item->display; v != NULL; v = v->next) {
	      sp_marker_hide ((SPMarker *) shape->marker[i], NR_ARENA_ITEM_GET_KEY (v->arenaitem) + i);
	    }
	    shape->marker[i] = sp_object_hunref (shape->marker[i], object);
	  }
	}
	if (shape->curve) {
		shape->curve = sp_curve_unref (shape->curve);
	}
    
	if (((SPObjectClass *) parent_class)->release) {
	  ((SPObjectClass *) parent_class)->release (object);
	}
}



static void
sp_shape_set(SPObject *object, unsigned int key, gchar const *value)
{
    if (((SPObjectClass *) parent_class)->set) {
        ((SPObjectClass *) parent_class)->set(object, key, value);
    }
}

static Inkscape::XML::Node *
sp_shape_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    if (((SPObjectClass *)(parent_class))->write) {
        ((SPObjectClass *)(parent_class))->write(object, repr, flags);
    }

    return repr;
}

/** 
 * Updates the shape when its attributes have changed.  Also establishes
 * marker objects to match the style settings.  
 */
static void
sp_shape_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
    SPItem *item = (SPItem *) object;
    SPShape *shape = (SPShape *) object;

	if (((SPObjectClass *) (parent_class))->update) {
	  (* ((SPObjectClass *) (parent_class))->update) (object, ctx, flags);
	}

	/* This stanza checks that an object's marker style agrees with
	 * the marker objects it has allocated.  sp_shape_set_marker ensures
	 * that the appropriate marker objects are present (or absent) to
	 * match the style.
	 */
	/* TODO:  It would be nice if this could be done at an earlier level */
	for (int i = 0 ; i < SP_MARKER_LOC_QTY ; i++) {
	    sp_shape_set_marker (object, i, object->style->marker[i].value);
	  }

	if (flags & (SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
		SPStyle *style;
		style = SP_OBJECT_STYLE (object);
		if (style->stroke_width.unit == SP_CSS_UNIT_PERCENT) {
			SPItemCtx *ictx = (SPItemCtx *) ctx;
			double const aw = 1.0 / NR::expansion(ictx->i2vp);
			style->stroke_width.computed = style->stroke_width.value * aw;
			for (SPItemView *v = ((SPItem *) (shape))->display; v != NULL; v = v->next) {
				nr_arena_shape_set_style ((NRArenaShape *) v->arenaitem, style);
			}
		}
	}

	if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG)) {
		/* This is suboptimal, because changing parent style schedules recalculation */
		/* But on the other hand - how can we know that parent does not tie style and transform */
                NR::Maybe<NR::Rect> paintbox = SP_ITEM(object)->getBounds(NR::identity());
		for (SPItemView *v = SP_ITEM (shape)->display; v != NULL; v = v->next) {
                    NRArenaShape * const s = NR_ARENA_SHAPE(v->arenaitem);
                    if (flags & SP_OBJECT_MODIFIED_FLAG) {
                        nr_arena_shape_set_path(s, shape->curve, (flags & SP_OBJECT_USER_MODIFIED_FLAG_B));
                    }
                    if (paintbox) {
                        s->setPaintBox(*paintbox);
                    }
		}
	}

        if (sp_shape_has_markers (shape)) {

            /* Dimension marker views */
            for (SPItemView *v = item->display; v != NULL; v = v->next) {

                if (!v->arenaitem->key) {
		    /* Get enough keys for all, start, mid and end marker types,
		    ** and set this view's arenaitem key to the first of these keys.
		    */
		    NR_ARENA_ITEM_SET_KEY (
                        v->arenaitem,
                        sp_item_display_key_new (SP_MARKER_LOC_QTY)
                        );
                }

                for (int i = 0 ; i < SP_MARKER_LOC_QTY ; i++) {
                    if (shape->marker[i]) {
                        sp_marker_show_dimension ((SPMarker *) shape->marker[i],
                                                  NR_ARENA_ITEM_GET_KEY (v->arenaitem) + i - SP_MARKER_LOC,
                                                  sp_shape_number_of_markers (shape, i));
                    }
		}
            }

            /* Update marker views */
            for (SPItemView *v = item->display; v != NULL; v = v->next) {
                sp_shape_update_marker_view (shape, v->arenaitem);
            }
	}
}


/**
* Works out whether a marker of a given type is required at a particular
* point on a shape.
*
* \param shape Shape of interest.
* \param m Marker type (e.g. SP_MARKER_LOC_START)
* \param bp Path segment.
* \return 1 if a marker is required here, otherwise 0.
*/
bool
sp_shape_marker_required(SPShape const *shape, int const m, NArtBpath *bp)
{
    if (shape->marker[m] == NULL) {
        return false;
    }

    if (bp == SP_CURVE_BPATH(shape->curve))
        return m == SP_MARKER_LOC_START;
    else if (bp[1].code == NR_END)
        return m == SP_MARKER_LOC_END;
    else
        return m == SP_MARKER_LOC_MID;
}

static bool
is_moveto(NRPathcode const c)
{
    return c == NR_MOVETO || c == NR_MOVETO_OPEN;
}

/** 
 * Helper function that advances a subpath's bpath to the first subpath
 * by checking for moveto segments.
 *
 * \pre The bpath[] containing bp begins with a moveto. 
 */
static NArtBpath const *
first_seg_in_subpath(NArtBpath const *bp)
{
    while (!is_moveto(bp->code)) {
        --bp;
    }
    return bp;
}

/**
 * Advances the bpath to the last segment in the subpath.
 */
static NArtBpath const *
last_seg_in_subpath(NArtBpath const *bp)
{
    for(;;) {
        ++bp;
        switch (bp->code) {
            case NR_MOVETO:
            case NR_MOVETO_OPEN:
            case NR_END:
                --bp;
                return bp;

            default: continue;
        }
    }
}


/* A subpath begins with a moveto and ends immediately before the next moveto or NR_END.
 * (`moveto' here means either NR_MOVETO or NR_MOVETO_OPEN.)  I'm assuming that non-empty
 * paths always begin with a moveto.
 *
 * The control points of the subpath are the control points of the path elements of the subpath.
 *
 * As usual, the control points of a moveto or NR_LINETO are {c(3)}, and
 * the control points of a NR_CURVETO are {c(1), c(2), c(3)}.
 * (It follows from the definition that NR_END isn't part of a subpath.)
 *
 * The initial control point is bpath[bi0].c(3).
 *
 * Reference: http://www.w3.org/TR/SVG11/painting.html#MarkerElement, the `orient' attribute.
 * Reference for behaviour of zero-length segments:
 * http://www.w3.org/TR/SVG11/implnote.html#PathElementImplementationNotes
 */

static double const no_tangent = 128.0;  /* arbitrarily-chosen value outside the range of atan2,
                                          * i.e. outside of [-pi, pi]. This value is incremented by
                                          * 1 and checked using > to be safe from floating-point
                                          * equality comparison madness.*/

/**
 * Helper function to calculate the outgoing tangent of a path 
 * ( atan2(other - p0) )
 * \pre The bpath[] containing bp0 begins with a moveto. 
 */
static double
outgoing_tangent(NArtBpath const *bp0)
{
    /* See notes in comment block above. */

    g_assert(bp0->code != NR_END);
    NR::Point const &p0 = bp0->c(3);
    NR::Point other;
    for (NArtBpath const *bp = bp0;;) {
        ++bp;
        switch (bp->code) {
            case NR_LINETO:
                other = bp->c(3);
                if (other != p0) {
                    goto found;
                }
                break;

            case NR_CURVETO:
                for (unsigned ci = 1; ci <= 3; ++ci) {
                    other = bp->c(ci);
                    if (other != p0) {
                        goto found;
                    }
                }
                break;

            case NR_MOVETO_OPEN:
            case NR_END:
            case NR_MOVETO:
                bp = first_seg_in_subpath(bp0);
                if (bp == bp0) {
                    /* Gone right around the subpath without finding any different point since the
                     * initial moveto. */
                    return no_tangent + 1;
                }
                if (bp->code != NR_MOVETO) {
                    /* Open subpath. */
                    return no_tangent + 1;
                }
                other = bp->c(3);
                if (other != p0) {
                    goto found;
                }
                break;
        }

        if (bp == bp0) {
            /* Back where we started, so zero-length subpath. */
            return no_tangent + 1;

            /* Note: this test must come after we've looked at element bp, in case bp0 is a curve:
             * we must look at c(1) and c(2).  (E.g. single-curve subpath.)
             */
        }
    }

found:
    return atan2( other - p0 );
}

/**
 * Helper function to calculate the incoming tangent of a path
 * ( atan2(p0 - other) )
 * 
 * \pre The bpath[] containing bp0 begins with a moveto. 
 */
static double
incoming_tangent(NArtBpath const *bp0)
{
    /* See notes in comment block before outgoing_tangent. */

    g_assert(bp0->code != NR_END);
    NR::Point const &p0 = bp0->c(3);
    NR::Point other;
    for (NArtBpath const *bp = bp0;;) {
        switch (bp->code) {
            case NR_LINETO:
                other = bp->c(3);
                if (other != p0) {
                    goto found;
                }
                --bp;
                break;

            case NR_CURVETO:
                for (unsigned ci = 3; ci != 0; --ci) {
                    other = bp->c(ci);
                    if (other != p0) {
                        goto found;
                    }
                }
                --bp;
                break;

            case NR_MOVETO:
            case NR_MOVETO_OPEN:
                other = bp->c(3);
                if (other != p0) {
                    goto found;
                }
                if (bp->code != NR_MOVETO) {
                    /* Open subpath. */
                    return no_tangent + 1;
                }
                bp = last_seg_in_subpath(bp0);
                break;

            default: /* includes NR_END */
                g_error("Found invalid path code %u in middle of path.", bp->code);
                return no_tangent + 1;
        }

        if (bp == bp0) {
            /* Back where we started from: zero-length subpath. */
            return no_tangent + 1;
        }
    }

found:
    return atan2( p0 - other );
}


/**
 * Calculate the transform required to get a marker's path object in the
 * right place for particular path segment on a shape.  You should
 * call sp_shape_marker_required first to see if a marker is required
 * at this point.
 *
 * \see sp_shape_marker_required.
 *
 * \param shape Shape which the marker is for.
 * \param m Marker type (e.g. SP_MARKER_LOC_START)
 * \param bp Path segment which the arrow is for.
 * \return Transform matrix.
 */
NR::Matrix
sp_shape_marker_get_transform(SPShape const *shape, NArtBpath const *bp)
{
    g_return_val_if_fail(( is_moveto(SP_CURVE_BPATH(shape->curve)[0].code)
                           && ( 0 < shape->curve->end )
                           && ( SP_CURVE_BPATH(shape->curve)[shape->curve->end].code == NR_END ) ),
                         NR::Matrix(NR::translate(bp->c(3))));
    double const angle1 = incoming_tangent(bp);
    double const angle2 = outgoing_tangent(bp);

    double ret_angle;
    if (angle1 > no_tangent) {
        /* First vertex of an open subpath. */
        ret_angle = ( angle2 > no_tangent
                      ? 0.
                      : angle2 );
    } else if (angle2 > no_tangent) {
        /* Last vertex of an open subpath. */
        ret_angle = angle1;
    } else {
        ret_angle = .5 * (angle1 + angle2);

        if ( fabs( angle2 - angle1 ) > M_PI ) {
            /* ret_angle is in the middle of the larger of the two sectors between angle1 and
             * angle2, so flip it by 180degrees to force it to the middle of the smaller sector.
             *
             * (Imagine a circle with rays drawn at angle1 and angle2 from the centre of the
             * circle.  Those two rays divide the circle into two sectors.)
             */
            ret_angle += M_PI;
        }
    }

    return NR::Matrix(NR::rotate(ret_angle)) * NR::translate(bp->c(3));
}

/**
 * Updates the instances (views) of a given marker in a shape.
 * Marker views have to be scaled already.  The transformation
 * is retrieved and then shown by calling sp_marker_show_instance.
 */
static void
sp_shape_update_marker_view (SPShape *shape, NRArenaItem *ai)
{
	SPStyle *style = ((SPObject *) shape)->style;

        for (int i = SP_MARKER_LOC_START; i < SP_MARKER_LOC_QTY; i++) {
            if (shape->marker[i] == NULL) {
                continue;
            }

            int n = 0;

            for (NArtBpath *bp = SP_CURVE_BPATH(shape->curve); bp->code != NR_END; bp++) {
                if (sp_shape_marker_required (shape, i, bp)) {
                    NR::Matrix const m(sp_shape_marker_get_transform(shape, bp));
                    sp_marker_show_instance ((SPMarker* ) shape->marker[i], ai,
                                             NR_ARENA_ITEM_GET_KEY(ai) + i, n, m,
                                             style->stroke_width.computed);
                    n++;
                }
            }
	}
}

/**
 * Sets modified flag for all sub-item views.
 */
static void
sp_shape_modified (SPObject *object, unsigned int flags)
{
	SPShape *shape = SP_SHAPE (object);

	if (((SPObjectClass *) (parent_class))->modified) {
	  (* ((SPObjectClass *) (parent_class))->modified) (object, flags);
	}

	if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
		for (SPItemView *v = SP_ITEM (shape)->display; v != NULL; v = v->next) {
			nr_arena_shape_set_style (NR_ARENA_SHAPE (v->arenaitem), object->style);
		}
	}
}

/**
 * Calculates the bounding box for item, storing it into bbox.
 * This also includes the bounding boxes of any markers included in the shape.
 */
static void sp_shape_bbox(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags)
{
    SPShape const *shape = SP_SHAPE (item);

    if (shape->curve) {

        NRRect  cbbox;
        NRBPath bp;

        bp.path = SP_CURVE_BPATH (shape->curve);

        cbbox.x0 = cbbox.y0 = NR_HUGE;
        cbbox.x1 = cbbox.y1 = -NR_HUGE;

        nr_path_matrix_bbox_union(&bp, transform, &cbbox);

        if ((SPItem::BBoxType) flags != SPItem::GEOMETRIC_BBOX) {
            
            SPStyle* style=SP_OBJECT_STYLE (item);
            if (!style->stroke.isNone()) {
                double const scale = expansion(transform);
                if ( fabs(style->stroke_width.computed * scale) > 0.01 ) { // sinon c'est 0=oon veut pas de bord
                    double const width = MAX(0.125, style->stroke_width.computed * scale);
                    if ( fabs(cbbox.x1-cbbox.x0) > -0.00001 && fabs(cbbox.y1-cbbox.y0) > -0.00001 ) {
                        cbbox.x0-=0.5*width;
                        cbbox.x1+=0.5*width;
                        cbbox.y0-=0.5*width;
                        cbbox.y1+=0.5*width;
                    }
                }
            }

            // Union with bboxes of the markers, if any
            if (sp_shape_has_markers (shape)) {
                for (NArtBpath* bp = SP_CURVE_BPATH(shape->curve); bp->code != NR_END; bp++) {
                    for (int m = SP_MARKER_LOC_START; m < SP_MARKER_LOC_QTY; m++) {
                        if (sp_shape_marker_required (shape, m, bp)) {

                            SPMarker* marker = SP_MARKER (shape->marker[m]);
                            SPItem* marker_item = sp_item_first_item_child (SP_OBJECT (shape->marker[m]));

                            NR::Matrix tr(sp_shape_marker_get_transform(shape, bp));

                            if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
                                tr = NR::scale(style->stroke_width.computed) * tr;
                            }

                            // total marker transform
                            tr = marker_item->transform * marker->c2p * tr * transform;

                            // get bbox of the marker with that transform
                            NRRect marker_bbox;
                            sp_item_invoke_bbox (marker_item, &marker_bbox, tr, true);
                            // union it with the shape bbox
                            nr_rect_d_union (&cbbox, &cbbox, &marker_bbox);
                        }
                    }
                }
            }
        }

        // copy our bbox to the variable we're given
        *bbox = cbbox;
    }
}

/**
 * Prepares shape for printing.  Handles printing of comments for printing
 * debugging, sizes the item to fit into the document width/height,
 * applies print fill/stroke, sets transforms for markers, and adds
 * comment labels.
 */
void
sp_shape_print (SPItem *item, SPPrintContext *ctx)
{
	NRRect pbox, dbox, bbox;

	SPShape *shape = SP_SHAPE(item);

	if (!shape->curve) return;

        gint add_comments = prefs_get_int_attribute_limited ("printing.debug", "add-label-comments", 0, 0, 1);
        if (add_comments) {
            gchar * comment = g_strdup_printf("begin '%s'",
                                              SP_OBJECT(item)->defaultLabel());
            sp_print_comment(ctx, comment);
            g_free(comment);
        }

	/* fixme: Think (Lauris) */
	sp_item_invoke_bbox(item, &pbox, NR::identity(), TRUE);
	dbox.x0 = 0.0;
	dbox.y0 = 0.0;
	dbox.x1 = sp_document_width (SP_OBJECT_DOCUMENT (item));
	dbox.y1 = sp_document_height (SP_OBJECT_DOCUMENT (item));
	sp_item_bbox_desktop (item, &bbox);
	NR::Matrix const i2d = sp_item_i2d_affine(item);

        SPStyle* style = SP_OBJECT_STYLE (item);

	if (!style->fill.isNone()) {
		NRBPath bp;
		bp.path = SP_CURVE_BPATH(shape->curve);
		sp_print_fill (ctx, &bp, &i2d, style, &pbox, &dbox, &bbox);
	}

	if (!style->stroke.isNone()) {
		NRBPath bp;
		bp.path = SP_CURVE_BPATH(shape->curve);
		sp_print_stroke (ctx, &bp, &i2d, style, &pbox, &dbox, &bbox);
	}

        for (NArtBpath* bp = SP_CURVE_BPATH(shape->curve); bp->code != NR_END; bp++) {
            for (int m = SP_MARKER_LOC_START; m < SP_MARKER_LOC_QTY; m++) {
                if (sp_shape_marker_required (shape, m, bp)) {

                    SPMarker* marker = SP_MARKER (shape->marker[m]);
                    SPItem* marker_item = sp_item_first_item_child (SP_OBJECT (shape->marker[m]));

                    NR::Matrix tr(sp_shape_marker_get_transform(shape, bp));

                    if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
                        tr = NR::scale(style->stroke_width.computed) * tr;
                    }

                    tr = marker_item->transform * marker->c2p * tr;

                    NR::Matrix old_tr = marker_item->transform;
                    marker_item->transform = tr;
                    sp_item_invoke_print (marker_item, ctx);
                    marker_item->transform = old_tr;
                }
            }
        }

        if (add_comments) {
            gchar * comment = g_strdup_printf("end '%s'",
                                              SP_OBJECT(item)->defaultLabel());
            sp_print_comment(ctx, comment);
            g_free(comment);
        }
}

/**
 * Sets style, path, and paintbox.  Updates marker views, including dimensions.
 */
static NRArenaItem *
sp_shape_show (SPItem *item, NRArena *arena, unsigned int /*key*/, unsigned int /*flags*/)
{
	SPObject *object = SP_OBJECT(item);
	SPShape *shape = SP_SHAPE(item);

	NRArenaItem *arenaitem = NRArenaShape::create(arena);
        NRArenaShape * const s = NR_ARENA_SHAPE(arenaitem);
	nr_arena_shape_set_style(s, object->style);
	nr_arena_shape_set_path(s, shape->curve, false);
        NR::Maybe<NR::Rect> paintbox = item->getBounds(NR::identity());
        if (paintbox) {
            s->setPaintBox(*paintbox);
        }

        if (sp_shape_has_markers (shape)) {

            /* Dimension the marker views */
            if (!arenaitem->key) {
                NR_ARENA_ITEM_SET_KEY (arenaitem, sp_item_display_key_new (SP_MARKER_LOC_QTY));
            }

            for (int i = 0; i < SP_MARKER_LOC_QTY; i++) {
                if (shape->marker[i]) {
                    sp_marker_show_dimension ((SPMarker *) shape->marker[i],
                                              NR_ARENA_ITEM_GET_KEY (arenaitem) + i - SP_MARKER_LOC,
                                              sp_shape_number_of_markers (shape, i));
		}
            }


            /* Update marker views */
            sp_shape_update_marker_view (shape, arenaitem);
	}

	return arenaitem;
}

/**
 * Hides/removes marker views from the shape.
 */
static void
sp_shape_hide (SPItem *item, unsigned int key)
{
	SPShape *shape;
	SPItemView *v;
	int i;

	shape = (SPShape *) item;

	for (i=0; i<SP_MARKER_LOC_QTY; i++) {
	  if (shape->marker[i]) {
	    for (v = item->display; v != NULL; v = v->next) {
                if (key == v->key) {
	      sp_marker_hide ((SPMarker *) shape->marker[i],
                                    NR_ARENA_ITEM_GET_KEY (v->arenaitem) + i);
                }
	    }
	  }
	}

	if (((SPItemClass *) parent_class)->hide) {
	  ((SPItemClass *) parent_class)->hide (item, key);
	}
}

/**
* \param shape Shape.
* \return TRUE if the shape has any markers, or FALSE if not.
*/
int
sp_shape_has_markers (SPShape const *shape)
{
    /* Note, we're ignoring 'marker' settings, which technically should apply for
       all three settings.  This should be fixed later such that if 'marker' is
       specified, then all three should appear. */

    return (
        shape->curve &&
        (shape->marker[SP_MARKER_LOC_START] ||
         shape->marker[SP_MARKER_LOC_MID] ||
         shape->marker[SP_MARKER_LOC_END])
        );
}


/**
* \param shape Shape.
* \param type Marker type (e.g. SP_MARKER_LOC_START)
* \return Number of markers that the shape has of this type.
*/
int
sp_shape_number_of_markers (SPShape *shape, int type)
{
    int n = 0;
    for (NArtBpath* bp = SP_CURVE_BPATH(shape->curve); bp->code != NR_END; bp++) {
        if (sp_shape_marker_required (shape, type, bp)) {
            n++;
        }
    }

    return n;
}

/**
 * Checks if the given marker is used in the shape, and if so, it
 * releases it by calling sp_marker_hide.  Also detaches signals
 * and unrefs the marker from the shape.
 */
static void
sp_shape_marker_release (SPObject *marker, SPShape *shape)
{
	SPItem *item;
	int i;

	item = (SPItem *) shape;

	for (i = SP_MARKER_LOC_START; i < SP_MARKER_LOC_QTY; i++) {
	  if (marker == shape->marker[i]) {
	    SPItemView *v;
	    /* Hide marker */
	    for (v = item->display; v != NULL; v = v->next) {
	      sp_marker_hide ((SPMarker *) (shape->marker[i]), NR_ARENA_ITEM_GET_KEY (v->arenaitem) + i);
	      /* fixme: Do we need explicit remove here? (Lauris) */
	      /* nr_arena_item_set_mask (v->arenaitem, NULL); */
	    }
	    /* Detach marker */
	    sp_signal_disconnect_by_data (shape->marker[i], item);
	    shape->marker[i] = sp_object_hunref (shape->marker[i], item);
	  }
	}
}

/**
 * No-op.  Exists for handling 'modified' messages
 */
static void
sp_shape_marker_modified (SPObject */*marker*/, guint /*flags*/, SPItem */*item*/)
{
	/* I think mask does update automagically */
	/* g_warning ("Item %s mask %s modified", SP_OBJECT_ID (item), SP_OBJECT_ID (mask)); */
}

/**
 * Adds a new marker to shape object at the location indicated by key.  value 
 * must be a valid URI reference resolvable from the shape object (i.e., present
 * in the document <defs>).  If the shape object already has a marker
 * registered at the given position, it is removed first.  Then the
 * new marker is hrefed and its signals connected.
 */
void
sp_shape_set_marker (SPObject *object, unsigned int key, const gchar *value)
{
    SPItem *item = (SPItem *) object;
    SPShape *shape = (SPShape *) object;

    if (key < SP_MARKER_LOC_START || key > SP_MARKER_LOC_END) {
        return;
    }

    SPObject *mrk = sp_css_uri_reference_resolve (SP_OBJECT_DOCUMENT (object), value);
    if (mrk != shape->marker[key]) {
        if (shape->marker[key]) {
            SPItemView *v;

            /* Detach marker */
            shape->release_connect[key].disconnect();
            shape->modified_connect[key].disconnect();

            /* Hide marker */
            for (v = item->display; v != NULL; v = v->next) {
                sp_marker_hide ((SPMarker *) (shape->marker[key]),
                                NR_ARENA_ITEM_GET_KEY (v->arenaitem) + key);
                /* fixme: Do we need explicit remove here? (Lauris) */
                /* nr_arena_item_set_mask (v->arenaitem, NULL); */
            }

            /* Unref marker */
            shape->marker[key] = sp_object_hunref (shape->marker[key], object);
        }
        if (SP_IS_MARKER (mrk)) {
            shape->marker[key] = sp_object_href (mrk, object);
            shape->release_connect[key] = mrk->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_shape_marker_release), shape));
            shape->modified_connect[key] = mrk->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_shape_marker_modified), shape));
        }
    }
}



/* Shape section */

/**
 * Calls any registered handlers for the set_shape action
 */
void
sp_shape_set_shape (SPShape *shape)
{
	g_return_if_fail (shape != NULL);
	g_return_if_fail (SP_IS_SHAPE (shape));

	if (SP_SHAPE_CLASS (G_OBJECT_GET_CLASS (shape))->set_shape) {
	  SP_SHAPE_CLASS (G_OBJECT_GET_CLASS (shape))->set_shape (shape);
	}
}

/**
 * Adds a curve to the shape.  If owner is specified, a reference
 * will be made, otherwise the curve will be copied into the shape.
 * Any existing curve in the shape will be unreferenced first.
 * This routine also triggers a request to update the display.
 */
void
sp_shape_set_curve (SPShape *shape, SPCurve *curve, unsigned int owner)
{
	if (shape->curve) {
		shape->curve = sp_curve_unref (shape->curve);
	}
	if (curve) {
		if (owner) {
			shape->curve = sp_curve_ref (curve);
		} else {
			shape->curve = sp_curve_copy (curve);
		}
	}
        SP_OBJECT(shape)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Return duplicate of curve (if any exists) or NULL if there is no curve
 */
SPCurve *
sp_shape_get_curve (SPShape *shape)
{
	if (shape->curve) {
		return sp_curve_copy (shape->curve);
	}
	return NULL;
}

/**
 * Same as sp_shape_set_curve but without updating the display
 */
void
sp_shape_set_curve_insync (SPShape *shape, SPCurve *curve, unsigned int owner)
{
	if (shape->curve) {
		shape->curve = sp_curve_unref (shape->curve);
	}
	if (curve) {
		if (owner) {
			shape->curve = sp_curve_ref (curve);
		} else {
			shape->curve = sp_curve_copy (curve);
		}
	}
}

/**
 * Return all nodes in a path that are to be considered for snapping
 */
static void sp_shape_snappoints(SPItem const *item, SnapPointsIter p)
{
    g_assert(item != NULL);
    g_assert(SP_IS_SHAPE(item));

    SPShape const *shape = SP_SHAPE(item);
    if (shape->curve == NULL) {
        return;
    }
    
    NR::Matrix const i2d (sp_item_i2d_affine (item));
    NArtBpath const *b = SP_CURVE_BPATH(shape->curve);    
    
    // Cycle through the nodes in the concatenated subpaths
    while (b->code != NR_END) {
        NR::Point pos = b->c(3) * i2d; // this is the current node
        
        // NR_MOVETO Indicates the start of a closed subpath, see nr-path-code.h
        // If we're looking at a closed subpath, then we can skip this first 
        // point of the subpath because it's coincident with the last point.  
        if (b->code != NR_MOVETO) {
            if (b->code == NR_MOVETO_OPEN || b->code == NR_LINETO || b[1].code == NR_LINETO || b[1].code == NR_END) {
                // end points of a line segment are always considered for snapping
                *p = pos; 
            } else {        
                // g_assert(b->code == NR_CURVETO);
                NR::Point ppos, npos;
                ppos = b->code == NR_CURVETO ? b->c(2) * i2d : pos; // backward handle 
                npos = b[1].code == NR_CURVETO ? b[1].c(1) * i2d : pos; // forward handle            
                // Determine whether a node is at a smooth part of the path, by 
                // calculating a measure for the collinearity of the handles
                bool c1 = fabs (Inkscape::Util::triangle_area (pos, ppos, npos)) < 1; // points are (almost) collinear
                bool c2 = NR::L2(pos - ppos) < 1e-6 || NR::L2(pos - npos) < 1e-6; // endnode, or a node with a retracted handle
                if (!(c1 & !c2)) {
                    *p = pos; // only return non-smooth nodes ("cusps")
                }
            }
        }
        
        b++;
    }
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

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

#include <libnr/nr-matrix-fns.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-matrix-translate-ops.h>
#include <libnr/nr-scale-matrix-ops.h>
#include <2geom/rect.h>
#include <2geom/transforms.h>
#include <2geom/pathvector.h>
#include "helper/geom.h"
#include "helper/geom-nodetype.h"

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#include "macros.h"
#include "display/nr-arena-shape.h"
#include "display/curve.h"
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
static Inkscape::XML::Node *sp_shape_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static void sp_shape_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags);
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
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    SPObjectClass *sp_object_class = SP_OBJECT_CLASS(klass);
    SPItemClass * item_class = SP_ITEM_CLASS(klass);
    SPLPEItemClass * lpe_item_class = SP_LPE_ITEM_CLASS(klass);

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
		shape->curve = shape->curve->unref();
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
sp_shape_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    if (((SPObjectClass *)(parent_class))->write) {
        ((SPObjectClass *)(parent_class))->write(object, doc, repr, flags);
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
        boost::optional<NR::Rect> paintbox = SP_ITEM(object)->getBounds(NR::identity(), SPItem::GEOMETRIC_BBOX);
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
 * Calculate the transform required to get a marker's path object in the
 * right place for particular path segment on a shape.
 *
 * \see sp_shape_marker_update_marker_view.
 *
 * From SVG spec:
 * The axes of the temporary new user coordinate system are aligned according to the orient attribute on the 'marker'
 * element and the slope of the curve at the given vertex. (Note: if there is a discontinuity at a vertex, the slope
 * is the average of the slopes of the two segments of the curve that join at the given vertex. If a slope cannot be
 * determined, the slope is assumed to be zero.)
 *
 * Reference: http://www.w3.org/TR/SVG11/painting.html#MarkerElement, the `orient' attribute.
 * Reference for behaviour of zero-length segments:
 * http://www.w3.org/TR/SVG11/implnote.html#PathElementImplementationNotes
 */
Geom::Matrix
sp_shape_marker_get_transform(Geom::Curve const & c1, Geom::Curve const & c2)
{
    Geom::Point p = c1.pointAt(1);
    Geom::Curve * c1_reverse = c1.reverse();
    Geom::Point tang1 = - c1_reverse->unitTangentAt(0);
    delete c1_reverse;
    Geom::Point tang2 = c2.unitTangentAt(0);

    double const angle1 = Geom::atan2(tang1);
    double const angle2 = Geom::atan2(tang2);

    double ret_angle;
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

    return Geom::Rotate(ret_angle) * Geom::Translate(p);
}
Geom::Matrix
sp_shape_marker_get_transform_at_start(Geom::Curve const & c)
{
    Geom::Point p = c.pointAt(0);
    Geom::Matrix ret = Geom::Translate(p);

    if ( !c.isDegenerate() ) {
        Geom::Point tang = c.unitTangentAt(0);
        double const angle = Geom::atan2(tang);
        ret = Geom::Rotate(angle) * Geom::Translate(p);
    } else {
        /* FIXME: the svg spec says to search for a better alternative than zero angle directionality:
         * http://www.w3.org/TR/SVG11/implnote.html#PathElementImplementationNotes */
    }

    return ret;
}
Geom::Matrix
sp_shape_marker_get_transform_at_end(Geom::Curve const & c)
{
    Geom::Point p = c.pointAt(1);
    Geom::Matrix ret = Geom::Translate(p);

    if ( !c.isDegenerate() ) {
        Geom::Curve * c_reverse = c.reverse();
        Geom::Point tang = - c_reverse->unitTangentAt(0);
        delete c_reverse;
        double const angle = Geom::atan2(tang);
        ret = Geom::Rotate(angle) * Geom::Translate(p);
    } else {
        /* FIXME: the svg spec says to search for a better alternative than zero angle directionality:
         * http://www.w3.org/TR/SVG11/implnote.html#PathElementImplementationNotes */
    }

    return ret;
}

/**
 * Updates the instances (views) of a given marker in a shape.
 * Marker views have to be scaled already.  The transformation
 * is retrieved and then shown by calling sp_marker_show_instance.
 *
 * TODO: correctly handle the 'marker' attribute.
 * "Using the marker property from a style sheet is equivalent to using all three (start, mid, end)."
 * See painting-marker-03-f.svg in SVG 1.1 Full test suite.
 */
static void
sp_shape_update_marker_view (SPShape *shape, NRArenaItem *ai)
{
    SPStyle *style = ((SPObject *) shape)->style;

    // position arguments to sp_marker_show_instance, basically counts the amount of markers.
    int start_pos = 0;
    int mid_pos = 0;
    int end_pos = 0;

    Geom::PathVector const & pathv = shape->curve->get_pathvector();
    for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
        if ( shape->marker[SP_MARKER_LOC_START] ) {
            Geom::Matrix const m (sp_shape_marker_get_transform_at_start(path_it->front()));
            sp_marker_show_instance ((SPMarker* ) shape->marker[SP_MARKER_LOC_START], ai,
                                     NR_ARENA_ITEM_GET_KEY(ai) + SP_MARKER_LOC_START, start_pos, m,
                                     style->stroke_width.computed);
             start_pos++;
        }

        if ( shape->marker[SP_MARKER_LOC_MID] && (path_it->size_default() > 1) ) {
            Geom::Path::const_iterator curve_it1 = path_it->begin();      // incoming curve
            Geom::Path::const_iterator curve_it2 = ++(path_it->begin());  // outgoing curve
            while (curve_it2 != path_it->end_default())
            {
                /* Put marker between curve_it1 and curve_it2.
                 * Loop to end_default (so including closing segment), because when a path is closed,
                 * there should be a midpoint marker between last segment and closing straight line segment
                 */
                Geom::Matrix const m (sp_shape_marker_get_transform(*curve_it1, *curve_it2));
                sp_marker_show_instance ((SPMarker* ) shape->marker[SP_MARKER_LOC_MID], ai,
                                         NR_ARENA_ITEM_GET_KEY(ai) + SP_MARKER_LOC_MID, mid_pos, m,
                                         style->stroke_width.computed);
                mid_pos++;

                ++curve_it1;
                ++curve_it2;
            }
        }

        if ( shape->marker[SP_MARKER_LOC_END] ) {
            /* Get reference to last curve in the path.
             * For moveto-only path, this returns the "closing line segment". */
            unsigned int index = path_it->size_default();
            if (index > 0) {
                index--;
            }
            Geom::Curve const &lastcurve = (*path_it)[index];

            Geom::Matrix const m = sp_shape_marker_get_transform_at_end(lastcurve);
            sp_marker_show_instance ((SPMarker* ) shape->marker[SP_MARKER_LOC_END], ai,
                                     NR_ARENA_ITEM_GET_KEY(ai) + SP_MARKER_LOC_END, end_pos, m,
                                     style->stroke_width.computed);
            end_pos++;
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
static void sp_shape_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags)
{
    SPShape const *shape = SP_SHAPE (item);

    if (shape->curve) {

        NRRect  cbbox;

        Geom::Rect geombbox = bounds_exact_transformed(shape->curve->get_pathvector(), transform);
        cbbox.x0 = geombbox[0][0];
        cbbox.y0 = geombbox[1][0];
        cbbox.x1 = geombbox[0][1];
        cbbox.y1 = geombbox[1][1];

        if ((SPItem::BBoxType) flags != SPItem::GEOMETRIC_BBOX) {
            
            SPStyle* style=SP_OBJECT_STYLE (item);
            if (!style->stroke.isNone()) {
                double const scale = transform.descrim();
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
                /* TODO: make code prettier: lots of variables can be taken out of the loop! */
                Geom::PathVector const & pathv = shape->curve->get_pathvector();
                for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
                    if ( shape->marker[SP_MARKER_LOC_START] ) {
                        SPMarker* marker = SP_MARKER (shape->marker[SP_MARKER_LOC_START]);
                        SPItem* marker_item = sp_item_first_item_child (SP_OBJECT (shape->marker[SP_MARKER_LOC_START]));

                        Geom::Matrix tr(sp_shape_marker_get_transform_at_start(path_it->front()));

                        if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
                            tr = Geom::Scale(style->stroke_width.computed) * tr;
                        }

                        // total marker transform
                        tr = marker_item->transform * marker->c2p * tr * transform;

                        // get bbox of the marker with that transform
                        NRRect marker_bbox;
                        sp_item_invoke_bbox (marker_item, &marker_bbox, from_2geom(tr), true);
                        // union it with the shape bbox
                        nr_rect_d_union (&cbbox, &cbbox, &marker_bbox);
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

                            // total marker transform
                            tr = marker_item->transform * marker->c2p * tr * transform;

                            // get bbox of the marker with that transform
                            NRRect marker_bbox;
                            sp_item_invoke_bbox (marker_item, &marker_bbox, from_2geom(tr), true);
                            // union it with the shape bbox
                            nr_rect_d_union (&cbbox, &cbbox, &marker_bbox);

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
    Geom::Matrix const i2d(sp_item_i2d_affine(item));

        SPStyle* style = SP_OBJECT_STYLE (item);

    if (!style->fill.isNone()) {
        sp_print_fill (ctx, shape->curve->get_pathvector(), &i2d, style, &pbox, &dbox, &bbox);
    }

    if (!style->stroke.isNone()) {
        sp_print_stroke (ctx, shape->curve->get_pathvector(), &i2d, style, &pbox, &dbox, &bbox);
    }

    /* TODO: make code prettier: lots of variables can be taken out of the loop! */
    Geom::PathVector const & pathv = shape->curve->get_pathvector();
    for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
        if ( shape->marker[SP_MARKER_LOC_START] ) {
            SPMarker* marker = SP_MARKER (shape->marker[SP_MARKER_LOC_START]);
            SPItem* marker_item = sp_item_first_item_child (SP_OBJECT (shape->marker[SP_MARKER_LOC_START]));

            Geom::Matrix tr(sp_shape_marker_get_transform_at_start(path_it->front()));

            if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
                tr = Geom::Scale(style->stroke_width.computed) * tr;
            }

            tr = marker_item->transform * marker->c2p * tr;

            NR::Matrix old_tr = marker_item->transform;
            marker_item->transform = tr;
            sp_item_invoke_print (marker_item, ctx);
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

                tr = marker_item->transform * marker->c2p * tr;

                NR::Matrix old_tr = marker_item->transform;
                marker_item->transform = tr;
                sp_item_invoke_print (marker_item, ctx);
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

            tr = marker_item->transform * marker->c2p * tr;

            NR::Matrix old_tr = marker_item->transform;
            marker_item->transform = tr;
            sp_item_invoke_print (marker_item, ctx);
            marker_item->transform = old_tr;
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
        boost::optional<NR::Rect> paintbox = item->getBounds(NR::identity());
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
    Geom::PathVector const & pathv = shape->curve->get_pathvector();

    switch(type) {
        case SP_MARKER_LOC_START:
            return shape->marker[SP_MARKER_LOC_START] ? pathv.size() : 0;

        case SP_MARKER_LOC_MID:
        {
            if ( shape->marker[SP_MARKER_LOC_MID] ) {
            guint n = 0;
                for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
                    n += path_it->size();
                    n += path_it->closed() ? 1 : 0;
                }
                return n;
            } else {
                return 0;
            }
        }

        case SP_MARKER_LOC_END:
        {
            if ( shape->marker[SP_MARKER_LOC_END] ) {
                guint n = 0;
                for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
                    if (!path_it->empty()) {
                        n++;
                    }
                }
                return n;
            } else {
                return 0;
            }
        }

        default:
            return 0;
    }
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
		shape->curve = shape->curve->unref();
	}
	if (curve) {
		if (owner) {
			shape->curve = curve->ref();
		} else {
			shape->curve = curve->copy();
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
		return shape->curve->copy();
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
		shape->curve = shape->curve->unref();
	}
	if (curve) {
		if (owner) {
			shape->curve = curve->ref();
		} else {
			shape->curve = curve->copy();
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

    Geom::PathVector const &pathv = shape->curve->get_pathvector();
    if (pathv.empty())
        return;

    Geom::Matrix const i2d (sp_item_i2d_affine (item));

    for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
        *p = from_2geom(path_it->initialPoint() * i2d);

        Geom::Path::const_iterator curve_it1 = path_it->begin();      // incoming curve
        Geom::Path::const_iterator curve_it2 = ++(path_it->begin());  // outgoing curve
        while (curve_it2 != path_it->end_closed())
        {
            /* Test whether to add the node between curve_it1 and curve_it2.
             * Loop to end_closed (so always including closing segment), the last node to be added
             * is the node between the closing segment and the segment before that one. Regardless
             * of the path being closed. If the path is closed, the final point was already added by
             * adding the initial point. */

            Geom::NodeType nodetype = Geom::get_nodetype(*curve_it1, *curve_it2);

            // Only add cusp nodes. TODO: Shouldn't this be a preference instead?
            if (nodetype == Geom::NODE_NONE) {
                *p = from_2geom(curve_it1->finalPoint() * i2d);
            }
            if (nodetype == Geom::NODE_CUSP) {
                *p = from_2geom(curve_it1->finalPoint() * i2d);
            }

            ++curve_it1;
            ++curve_it2;
        }
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

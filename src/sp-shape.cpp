/*
 * Base class for shapes, including <path> element
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2004 John Cliff
 * Copyright (C) 2007-2008 Johan Engelen
 * Copyright (C) 2010      Jon A. Cruz <jon@joncruz.org>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <2geom/rect.h>
#include <2geom/transforms.h>
#include <2geom/pathvector.h>
#include <2geom/path-intersection.h>
#include <2geom/exception.h>
#include "helper/geom.h"
#include "helper/geom-nodetype.h"

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#include "macros.h"
#include "display/drawing-shape.h"
#include "display/curve.h"
#include "print.h"
#include "document.h"
#include "style.h"
#include "marker.h"
#include "sp-path.h"
#include "preferences.h"
#include "attributes.h"

#include "live_effects/lpeobject.h"
#include "uri.h"
#include "extract-uri.h"
#include "uri-references.h"
#include "bad-uri-exception.h"
#include "xml/repr.h"

#include "util/mathfns.h" // for triangle_area()

#include "splivarot.h" // for bounding box calculation

#define noSHAPE_VERBOSE

void sp_shape_print (SPItem * item, SPPrintContext * ctx);

SPLPEItemClass * SPShapeClass::parent_class = 0;

/**
 * Registers the SPShape class with Gdk and returns its type number.
 */
GType SPShape::getType(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof (SPShapeClass),
            NULL, NULL,
            (GClassInitFunc) SPShapeClass::sp_shape_class_init,
            NULL, NULL,
            sizeof (SPShape),
            16,
            (GInstanceInitFunc) sp_shape_init,
            NULL,    /* value_table */
        };
        type = g_type_register_static (SP_TYPE_LPE_ITEM, "SPShape", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * Initializes a SPShapeClass object.  Establishes the function pointers to the class'
 * member routines in the class vtable, and sets pointers to parent classes.
 */
void SPShapeClass::sp_shape_class_init(SPShapeClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    SPObjectClass *sp_object_class = SP_OBJECT_CLASS(klass);
    SPItemClass * item_class = SP_ITEM_CLASS(klass);
    SPLPEItemClass * lpe_item_class = SP_LPE_ITEM_CLASS(klass);

    parent_class = (SPLPEItemClass *)g_type_class_peek_parent (klass);

    gobject_class->finalize = SPShape::sp_shape_finalize;

    sp_object_class->build = SPShape::sp_shape_build;
    sp_object_class->release = SPShape::sp_shape_release;
    sp_object_class->set = SPShape::sp_shape_set;
    sp_object_class->update = SPShape::sp_shape_update;
    sp_object_class->modified = SPShape::sp_shape_modified;
    sp_object_class->write = SPShape::sp_shape_write;

    item_class->bbox = SPShape::sp_shape_bbox;
    item_class->print = sp_shape_print;
    item_class->show = SPShape::sp_shape_show;
    item_class->hide = SPShape::sp_shape_hide;
    item_class->snappoints = SPShape::sp_shape_snappoints;
    lpe_item_class->update_patheffect = NULL;

    klass->set_shape = NULL;
}

/**
 * Initializes an SPShape object.
 */
void SPShape::sp_shape_init(SPShape *shape)
{
    for ( int i = 0 ; i < SP_MARKER_LOC_QTY ; i++ ) {
        new (&shape->_release_connect[i]) sigc::connection();
        new (&shape->_modified_connect[i]) sigc::connection();
        shape->_marker[i] = NULL;
    }
    shape->_curve = NULL;
    shape->_curve_before_lpe = NULL;
}

void SPShape::sp_shape_finalize(GObject *object)
{
    SPShape *shape=(SPShape *)object;

    for ( int i = 0 ; i < SP_MARKER_LOC_QTY ; i++ ) {
        shape->_release_connect[i].disconnect();
        shape->_release_connect[i].~connection();
        shape->_modified_connect[i].disconnect();
        shape->_modified_connect[i].~connection();
    }

    if (((GObjectClass *) (SPShapeClass::parent_class))->finalize) {
        (* ((GObjectClass *) (SPShapeClass::parent_class))->finalize)(object);
    }
}

/**
 * Virtual build callback for SPMarker.
 *
 * This is to be invoked immediately after creation of an SPShape.
 *
 * \see sp_object_build()
 */
void SPShape::sp_shape_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) (SPShapeClass::parent_class))->build) {
       (*((SPObjectClass *) (SPShapeClass::parent_class))->build) (object, document, repr);
    }

    for (int i = 0 ; i < SP_MARKER_LOC_QTY ; i++) {
        sp_shape_set_marker (object, i, object->style->marker[i].value);
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
void SPShape::sp_shape_release(SPObject *object)
{
    SPItem *item;
    SPShape *shape;
    SPItemView *v;
    int i;

    item = (SPItem *) object;
    shape = (SPShape *) object;

    for (i = 0; i < SP_MARKER_LOC_QTY; i++) {
        if (shape->_marker[i]) {
            for (v = item->display; v != NULL; v = v->next) {
              sp_marker_hide ((SPMarker *) shape->_marker[i], v->arenaitem->key() + i);
            }
            shape->_release_connect[i].disconnect();
            shape->_modified_connect[i].disconnect();
            shape->_marker[i] = sp_object_hunref (shape->_marker[i], object);
        }
    }
    if (shape->_curve) {
        shape->_curve = shape->_curve->unref();
    }
    if (shape->_curve_before_lpe) {
        shape->_curve_before_lpe = shape->_curve_before_lpe->unref();
    }

    if (((SPObjectClass *) SPShapeClass::parent_class)->release) {
      ((SPObjectClass *) SPShapeClass::parent_class)->release (object);
    }
}



void SPShape::sp_shape_set(SPObject *object, unsigned int key, gchar const *value)
{
    if (((SPObjectClass *) SPShapeClass::parent_class)->set) {
        ((SPObjectClass *) SPShapeClass::parent_class)->set(object, key, value);
    }
}

Inkscape::XML::Node * SPShape::sp_shape_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    if (((SPObjectClass *)(SPShapeClass::parent_class))->write) {
        ((SPObjectClass *)(SPShapeClass::parent_class))->write(object, doc, repr, flags);
    }

    return repr;
}

/**
 * Updates the shape when its attributes have changed.  Also establishes
 * marker objects to match the style settings.
 */
void SPShape::sp_shape_update(SPObject *object, SPCtx *ctx, unsigned int flags)
{
    SPShape *shape = (SPShape *) object;

    if (((SPObjectClass *) (SPShapeClass::parent_class))->update) {
        (* ((SPObjectClass *) (SPShapeClass::parent_class))->update) (object, ctx, flags);
    }

    /* This stanza checks that an object's marker style agrees with
     * the marker objects it has allocated.  sp_shape_set_marker ensures
     * that the appropriate marker objects are present (or absent) to
     * match the style.
     */
    for (int i = 0 ; i < SP_MARKER_LOC_QTY ; i++) {
        sp_shape_set_marker (object, i, object->style->marker[i].value);
      }

    if (flags & (SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        SPStyle *style = object->style;
        if (style->stroke_width.unit == SP_CSS_UNIT_PERCENT) {
            SPItemCtx *ictx = (SPItemCtx *) ctx;
            double const aw = 1.0 / ictx->i2vp.descrim();
            style->stroke_width.computed = style->stroke_width.value * aw;
            for (SPItemView *v = ((SPItem *) (shape))->display; v != NULL; v = v->next) {
                Inkscape::DrawingShape *sh = dynamic_cast<Inkscape::DrawingShape *>(v->arenaitem);
                sh->setStyle(style);
            }
        }
    }

    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_PARENT_MODIFIED_FLAG)) {
        /* This is suboptimal, because changing parent style schedules recalculation */
        /* But on the other hand - how can we know that parent does not tie style and transform */
        for (SPItemView *v = shape->display; v != NULL; v = v->next) {
            Inkscape::DrawingShape *sh = dynamic_cast<Inkscape::DrawingShape *>(v->arenaitem);
            if (flags & SP_OBJECT_MODIFIED_FLAG) {
                sh->setPath(shape->_curve);
            }
        }
    }

    if (shape->hasMarkers ()) {
        /* Dimension marker views */
        for (SPItemView *v = shape->display; v != NULL; v = v->next) {
            if (!v->arenaitem->key()) {
                v->arenaitem->setKey(SPItem::display_key_new (SP_MARKER_LOC_QTY));
            }
            for (int i = 0 ; i < SP_MARKER_LOC_QTY ; i++) {
                if (shape->_marker[i]) {
                    sp_marker_show_dimension ((SPMarker *) shape->_marker[i],
                                              v->arenaitem->key() + i,
                                              shape->numberOfMarkers (i));
                }
            }
        }

        /* Update marker views */
        for (SPItemView *v = shape->display; v != NULL; v = v->next) {
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
Geom::Affine sp_shape_marker_get_transform(Geom::Curve const & c1, Geom::Curve const & c2)
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

Geom::Affine sp_shape_marker_get_transform_at_start(Geom::Curve const & c)
{
    Geom::Point p = c.pointAt(0);
    Geom::Affine ret = Geom::Translate(p);

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

Geom::Affine sp_shape_marker_get_transform_at_end(Geom::Curve const & c)
{
    Geom::Point p = c.pointAt(1);
    Geom::Affine ret = Geom::Translate(p);

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
 * @todo figure out what to do when both 'marker' and for instance 'marker-end' are set.
 */
void SPShape::sp_shape_update_marker_view(SPShape *shape, Inkscape::DrawingItem *ai)
{
    SPStyle *style = ((SPObject *) shape)->style;

    // position arguments to sp_marker_show_instance, basically counts the amount of markers.
    int counter[4] = {0};

    if (!shape->_curve) return;
    Geom::PathVector const & pathv = shape->_curve->get_pathvector();
    if (pathv.empty()) return;

    // the first vertex should get a start marker, the last an end marker, and all the others a mid marker
    // see bug 456148

    // START marker
    {
        Geom::Affine const m (sp_shape_marker_get_transform_at_start(pathv.begin()->front()));
        for (int i = 0; i < 2; i++) {  // SP_MARKER_LOC and SP_MARKER_LOC_START
            if ( shape->_marker[i] ) {
                sp_marker_show_instance ((SPMarker* ) shape->_marker[i], ai,
                                         ai->key() + i, counter[i], m,
                                         style->stroke_width.computed);
                 counter[i]++;
            }
        }
    }

    // MID marker
    if (shape->_marker[SP_MARKER_LOC_MID] || shape->_marker[SP_MARKER_LOC]) {
        for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
            // START position
            if ( path_it != pathv.begin() 
                 && ! ((path_it == (pathv.end()-1)) && (path_it->size_default() == 0)) ) // if this is the last path and it is a moveto-only, don't draw mid marker there
            {
                Geom::Affine const m (sp_shape_marker_get_transform_at_start(path_it->front()));
                for (int i = 0; i < 3; i += 2) {  // SP_MARKER_LOC and SP_MARKER_LOC_MID
                    if ( shape->_marker[i] ) {
                        sp_marker_show_instance ((SPMarker* ) shape->_marker[i], ai,
                                                 ai->key() + i, counter[i], m,
                                                 style->stroke_width.computed);
                         counter[i]++;
                    }
                }
            }
            // MID position
            if ( path_it->size_default() > 1) {
                Geom::Path::const_iterator curve_it1 = path_it->begin();      // incoming curve
                Geom::Path::const_iterator curve_it2 = ++(path_it->begin());  // outgoing curve
                while (curve_it2 != path_it->end_default())
                {
                    /* Put marker between curve_it1 and curve_it2.
                     * Loop to end_default (so including closing segment), because when a path is closed,
                     * there should be a midpoint marker between last segment and closing straight line segment
                     */
                    Geom::Affine const m (sp_shape_marker_get_transform(*curve_it1, *curve_it2));
                    for (int i = 0; i < 3; i += 2) {  // SP_MARKER_LOC and SP_MARKER_LOC_MID
                        if (shape->_marker[i]) {
                            sp_marker_show_instance ((SPMarker* ) shape->_marker[i], ai,
                                                     ai->key() + i, counter[i], m,
                                                     style->stroke_width.computed);
                            counter[i]++;
                        }
                    }

                    ++curve_it1;
                    ++curve_it2;
                }
            }
            // END position
            if ( path_it != (pathv.end()-1) && !path_it->empty()) {
                Geom::Curve const &lastcurve = path_it->back_default();
                Geom::Affine const m = sp_shape_marker_get_transform_at_end(lastcurve);
                for (int i = 0; i < 3; i += 2) {  // SP_MARKER_LOC and SP_MARKER_LOC_MID
                    if (shape->_marker[i]) {
                        sp_marker_show_instance ((SPMarker* ) shape->_marker[i], ai,
                                                 ai->key() + i, counter[i], m,
                                                 style->stroke_width.computed);
                        counter[i]++;
                    }
                }
            }
        }
    }

    // END marker
    if ( shape->_marker[SP_MARKER_LOC_END] || shape->_marker[SP_MARKER_LOC] ) {
        /* Get reference to last curve in the path.
         * For moveto-only path, this returns the "closing line segment". */
        Geom::Path const &path_last = pathv.back();
        unsigned int index = path_last.size_default();
        if (index > 0) {
            index--;
        }
        Geom::Curve const &lastcurve = path_last[index];
        Geom::Affine const m = sp_shape_marker_get_transform_at_end(lastcurve);

        for (int i = 0; i < 4; i += 3) {  // SP_MARKER_LOC and SP_MARKER_LOC_END
            if (shape->_marker[i]) {
                sp_marker_show_instance ((SPMarker* ) shape->_marker[i], ai,
                                         ai->key() + i, counter[i], m,
                                         style->stroke_width.computed);
                counter[i]++;
            }
        }
    }
}

/**
 * Sets modified flag for all sub-item views.
 */
void SPShape::sp_shape_modified(SPObject *object, unsigned int flags)
{
    SPShape *shape = SP_SHAPE (object);

    if (((SPObjectClass *) (SPShapeClass::parent_class))->modified) {
      (* ((SPObjectClass *) (SPShapeClass::parent_class))->modified) (object, flags);
    }

    if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
        for (SPItemView *v = shape->display; v != NULL; v = v->next) {
            Inkscape::DrawingShape *sh = dynamic_cast<Inkscape::DrawingShape *>(v->arenaitem);
            sh->setStyle(object->style);
        }
    }
}

/**
 * Calculates the bounding box for item, storing it into bbox.
 * This also includes the bounding boxes of any markers included in the shape.
 */
Geom::OptRect SPShape::sp_shape_bbox(SPItem const *item, Geom::Affine const &transform, SPItem::BBoxType bboxtype)
{
    SPShape const *shape = SP_SHAPE (item);
    Geom::OptRect bbox;

    if (!shape->_curve) return bbox;
    bbox = bounds_exact_transformed(shape->_curve->get_pathvector(), transform);
    if (!bbox) return bbox;

    if (bboxtype == SPItem::VISUAL_BBOX) {
        // convert the stroke to a path and calculate that path's geometric bbox
        SPStyle* style = item->style;
        if (!style->stroke.isNone()) {
            Geom::PathVector *pathv = item_outline(item, true);  // calculate bbox_only
            if (pathv) {
                bbox |= bounds_exact_transformed(*pathv, transform);
                delete pathv;
            }
        }
        // Union with bboxes of the markers, if any
        if ( shape->hasMarkers()  && !shape->_curve->get_pathvector().empty() ) {
            /** \todo make code prettier! */
            Geom::PathVector const & pathv = shape->_curve->get_pathvector();
            // START marker
            for (unsigned i = 0; i < 2; i++) { // SP_MARKER_LOC and SP_MARKER_LOC_START
                if ( shape->_marker[i] ) {
                    SPMarker* marker = SP_MARKER (shape->_marker[i]);
                    SPItem* marker_item = sp_item_first_item_child( marker );

                    if (marker_item) {
                        Geom::Affine tr(sp_shape_marker_get_transform_at_start(pathv.begin()->front()));
                        if (!marker->orient_auto) {
                            Geom::Point transl = tr.translation();
                            tr = Geom::Rotate::from_degrees(marker->orient) * Geom::Translate(transl);
                        }
                        if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
                            tr = Geom::Scale(style->stroke_width.computed) * tr;
                        }

                        // total marker transform
                        tr = marker_item->transform * marker->c2p * tr * transform;

                        // get bbox of the marker with that transform
                        bbox |= marker_item->visualBounds(tr);
                    }
                }
            }
            // MID marker
            for (unsigned i = 0; i < 3; i += 2) { // SP_MARKER_LOC and SP_MARKER_LOC_MID
                if ( !shape->_marker[i] ) continue;
                SPMarker* marker = SP_MARKER (shape->_marker[i]);
                SPItem* marker_item = sp_item_first_item_child( marker );
                if ( !marker_item ) continue;

                for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
                    // START position
                    if ( path_it != pathv.begin() 
                         && ! ((path_it == (pathv.end()-1)) && (path_it->size_default() == 0)) ) // if this is the last path and it is a moveto-only, there is no mid marker there
                    {
                        Geom::Affine tr(sp_shape_marker_get_transform_at_start(path_it->front()));
                        if (!marker->orient_auto) {
                            Geom::Point transl = tr.translation();
                            tr = Geom::Rotate::from_degrees(marker->orient) * Geom::Translate(transl);
                        }
                        if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
                            tr = Geom::Scale(style->stroke_width.computed) * tr;
                        }
                        tr = marker_item->transform * marker->c2p * tr * transform;
                        bbox |= marker_item->visualBounds(tr);
                    }
                    // MID position
                    if ( path_it->size_default() > 1) {
                        Geom::Path::const_iterator curve_it1 = path_it->begin();      // incoming curve
                        Geom::Path::const_iterator curve_it2 = ++(path_it->begin());  // outgoing curve
                        while (curve_it2 != path_it->end_default())
                        {
                            /* Put marker between curve_it1 and curve_it2.
                             * Loop to end_default (so including closing segment), because when a path is closed,
                             * there should be a midpoint marker between last segment and closing straight line segment */

                            SPMarker* marker = SP_MARKER (shape->_marker[i]);
                            SPItem* marker_item = sp_item_first_item_child( marker );

                            if (marker_item) {
                                Geom::Affine tr(sp_shape_marker_get_transform(*curve_it1, *curve_it2));
                                if (!marker->orient_auto) {
                                    Geom::Point transl = tr.translation();
                                    tr = Geom::Rotate::from_degrees(marker->orient) * Geom::Translate(transl);
                                }
                                if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
                                    tr = Geom::Scale(style->stroke_width.computed) * tr;
                                }
                                tr = marker_item->transform * marker->c2p * tr * transform;
                                bbox |= marker_item->visualBounds(tr);
                            }

                            ++curve_it1;
                            ++curve_it2;
                        }
                    }
                    // END position
                    if ( path_it != (pathv.end()-1) && !path_it->empty()) {
                        Geom::Curve const &lastcurve = path_it->back_default();
                        Geom::Affine tr = sp_shape_marker_get_transform_at_end(lastcurve);
                        if (!marker->orient_auto) {
                            Geom::Point transl = tr.translation();
                            tr = Geom::Rotate::from_degrees(marker->orient) * Geom::Translate(transl);
                        }
                        if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
                            tr = Geom::Scale(style->stroke_width.computed) * tr;
                        }
                        tr = marker_item->transform * marker->c2p * tr * transform;
                        bbox |= marker_item->visualBounds(tr);
                    }
                }
            }
            // END marker
            for (unsigned i = 0; i < 4; i += 3) { // SP_MARKER_LOC and SP_MARKER_LOC_END
                if ( shape->_marker[i] ) {
                    SPMarker* marker = SP_MARKER (shape->_marker[i]);
                    SPItem* marker_item = sp_item_first_item_child( marker );

                    if (marker_item) {
                        /* Get reference to last curve in the path.
                         * For moveto-only path, this returns the "closing line segment". */
                        Geom::Path const &path_last = pathv.back();
                        unsigned int index = path_last.size_default();
                        if (index > 0) {
                            index--;
                        }
                        Geom::Curve const &lastcurve = path_last[index];

                        Geom::Affine tr = sp_shape_marker_get_transform_at_end(lastcurve);
                        if (!marker->orient_auto) {
                            Geom::Point transl = tr.translation();
                            tr = Geom::Rotate::from_degrees(marker->orient) * Geom::Translate(transl);
                        }
                        if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
                            tr = Geom::Scale(style->stroke_width.computed) * tr;
                        }

                        // total marker transform
                        tr = marker_item->transform * marker->c2p * tr * transform;

                        // get bbox of the marker with that transform
                        bbox |= marker_item->visualBounds(tr);
                    }
                }
            }
        }
    }
    return bbox;
}

static void
sp_shape_print_invoke_marker_printing(SPObject *obj, Geom::Affine tr, SPStyle const *style, SPPrintContext *ctx)
{
    SPMarker *marker = SP_MARKER(obj);
    if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
        tr = Geom::Scale(style->stroke_width.computed) * tr;
    }

    SPItem* marker_item = sp_item_first_item_child( marker );
    if (marker_item) {
        tr = marker_item->transform * marker->c2p * tr;

        Geom::Affine old_tr = marker_item->transform;
        marker_item->transform = tr;
        marker_item->invoke_print (ctx);
        marker_item->transform = old_tr;
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
    Geom::OptRect pbox, dbox, bbox;

    SPShape *shape = SP_SHAPE(item);

    if (!shape->_curve) return;

    Geom::PathVector const & pathv = shape->_curve->get_pathvector();
    if (pathv.empty()) return;

        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        gint add_comments = prefs->getBool("/printing/debug/add-label-comments");
        if (add_comments) {
            gchar * comment = g_strdup_printf("begin '%s'",
                                              item->defaultLabel());
            sp_print_comment(ctx, comment);
            g_free(comment);
        }

    /* fixme: Think (Lauris) */
    pbox = item->geometricBounds();
    bbox = item->desktopVisualBounds();
    dbox = Geom::Rect::from_xywh(Geom::Point(0,0), item->document->getDimensions());
    Geom::Affine const i2dt(item->i2dt_affine());

    SPStyle* style = item->style;

    if (!style->fill.isNone()) {
        sp_print_fill (ctx, pathv, i2dt, style, pbox, dbox, bbox);
    }

    if (!style->stroke.isNone()) {
        sp_print_stroke (ctx, pathv, i2dt, style, pbox, dbox, bbox);
    }

    /** \todo make code prettier */
    // START marker
    for (int i = 0; i < 2; i++) {  // SP_MARKER_LOC and SP_MARKER_LOC_START    
        if ( shape->_marker[i] ) {
            Geom::Affine tr(sp_shape_marker_get_transform_at_start(pathv.begin()->front()));
            sp_shape_print_invoke_marker_printing(shape->_marker[i], tr, style, ctx);
        }
    }
    // MID marker
    for (int i = 0; i < 3; i += 2) {  // SP_MARKER_LOC and SP_MARKER_LOC_MID
        if (shape->_marker[i]) {
            for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
                // START position
                if ( path_it != pathv.begin() 
                     && ! ((path_it == (pathv.end()-1)) && (path_it->size_default() == 0)) ) // if this is the last path and it is a moveto-only, there is no mid marker there
                {
                    Geom::Affine tr(sp_shape_marker_get_transform_at_start(path_it->front()));
                    sp_shape_print_invoke_marker_printing(shape->_marker[i], tr, style, ctx);
                }
                // MID position
                if ( path_it->size_default() > 1) {
                    Geom::Path::const_iterator curve_it1 = path_it->begin();      // incoming curve
                    Geom::Path::const_iterator curve_it2 = ++(path_it->begin());  // outgoing curve
                    while (curve_it2 != path_it->end_default())
                    {
                        /* Put marker between curve_it1 and curve_it2.
                         * Loop to end_default (so including closing segment), because when a path is closed,
                         * there should be a midpoint marker between last segment and closing straight line segment */
                        Geom::Affine tr(sp_shape_marker_get_transform(*curve_it1, *curve_it2));

                        sp_shape_print_invoke_marker_printing(shape->_marker[i], tr, style, ctx);

                        ++curve_it1;
                        ++curve_it2;
                    }
                }
                if ( path_it != (pathv.end()-1) && !path_it->empty()) {
                    Geom::Curve const &lastcurve = path_it->back_default();
                    Geom::Affine tr = sp_shape_marker_get_transform_at_end(lastcurve);
                    sp_shape_print_invoke_marker_printing(shape->_marker[i], tr, style, ctx);
                }
            }
        }
    }
    // END marker
    if ( shape->_marker[SP_MARKER_LOC_END] || shape->_marker[SP_MARKER_LOC]) {
        /* Get reference to last curve in the path.
         * For moveto-only path, this returns the "closing line segment". */
        Geom::Path const &path_last = pathv.back();
        unsigned int index = path_last.size_default();
        if (index > 0) {
            index--;
        }
        Geom::Curve const &lastcurve = path_last[index];

        Geom::Affine tr = sp_shape_marker_get_transform_at_end(lastcurve);

        for (int i = 0; i < 4; i += 3) {  // SP_MARKER_LOC and SP_MARKER_LOC_END
            if (shape->_marker[i]) {
                sp_shape_print_invoke_marker_printing(shape->_marker[i], tr, style, ctx);
            }
        }
    }

        if (add_comments) {
            gchar * comment = g_strdup_printf("end '%s'",
                                              item->defaultLabel());
            sp_print_comment(ctx, comment);
            g_free(comment);
        }
}

/**
 * Sets style, path, and paintbox.  Updates marker views, including dimensions.
 */
Inkscape::DrawingItem * SPShape::sp_shape_show(SPItem *item, Inkscape::Drawing &drawing, unsigned int /*key*/, unsigned int /*flags*/)
{
    SPObject *object = item;
    SPShape *shape = SP_SHAPE(item);

    Inkscape::DrawingShape *s = new Inkscape::DrawingShape(drawing);
    s->setStyle(object->style);
    s->setPath(shape->_curve);

    /* This stanza checks that an object's marker style agrees with
     * the marker objects it has allocated.  sp_shape_set_marker ensures
     * that the appropriate marker objects are present (or absent) to
     * match the style.
     */
    for (int i = 0 ; i < SP_MARKER_LOC_QTY ; i++) {
        sp_shape_set_marker (object, i, object->style->marker[i].value);
      }

    if (shape->hasMarkers ()) {

        /* provide key and dimension the marker views */
        if (!s->key()) {
            s->setKey(SPItem::display_key_new (SP_MARKER_LOC_QTY));
        }

        for (int i = 0; i < SP_MARKER_LOC_QTY; i++) {
            if (shape->_marker[i]) {
                sp_marker_show_dimension ((SPMarker *) shape->_marker[i],
                                          s->key() + i,
                                          shape->numberOfMarkers (i));
            }
        }

        /* Update marker views */
        sp_shape_update_marker_view (shape, s);
    }

    return s;
}

/**
 * Hides/removes marker views from the shape.
 */
void SPShape::sp_shape_hide(SPItem *item, unsigned int key)
{
    SPShape *shape;
    SPItemView *v;
    int i;

    shape = (SPShape *) item;

    for (i=0; i<SP_MARKER_LOC_QTY; i++) {
      if (shape->_marker[i]) {
        for (v = item->display; v != NULL; v = v->next) {
                if (key == v->key) {
          sp_marker_hide ((SPMarker *) shape->_marker[i],
                                    v->arenaitem->key() + i);
                }
        }
      }
    }

    if (((SPItemClass *) SPShapeClass::parent_class)->hide) {
      ((SPItemClass *) SPShapeClass::parent_class)->hide (item, key);
    }
}

/**
* \param shape Shape.
* \return TRUE if the shape has any markers, or FALSE if not.
*/
int SPShape::hasMarkers() const
{
    /* Note, we're ignoring 'marker' settings, which technically should apply for
       all three settings.  This should be fixed later such that if 'marker' is
       specified, then all three should appear. */

    return (
        this->_curve &&
        (this->_marker[SP_MARKER_LOC] ||
         this->_marker[SP_MARKER_LOC_START] ||
         this->_marker[SP_MARKER_LOC_MID] ||
         this->_marker[SP_MARKER_LOC_END])
        );
}


/**
* \param shape Shape.
* \param type Marker type (e.g. SP_MARKER_LOC_START)
* \return Number of markers that the shape has of this type.
*/
int SPShape::numberOfMarkers(int type)
{
    Geom::PathVector const & pathv = this->_curve->get_pathvector();
    if (pathv.size() == 0) {
        return 0;
    }

    switch(type) {
        case SP_MARKER_LOC:
        {
            if ( this->_marker[SP_MARKER_LOC] ) {
                guint n = 0;
                for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
                    n += path_it->size_default() + 1;
                }
                return n;
            } else {
                return 0;
            }
        }
        case SP_MARKER_LOC_START:
            // there is only a start marker on the first path of a pathvector
            return this->_marker[SP_MARKER_LOC_START] ? 1 : 0;

        case SP_MARKER_LOC_MID:
        {
            if ( this->_marker[SP_MARKER_LOC_MID] ) {
                guint n = 0;
                for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
                    n += path_it->size_default() + 1;
                }
                return n - 2; // minus the start and end marker.
            } else {
                return 0;
            }
        }

        case SP_MARKER_LOC_END:
        {
            // there is only an end marker on the last path of a pathvector
            return this->_marker[SP_MARKER_LOC_END] ? 1 : 0;
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

    for (i = 0; i < SP_MARKER_LOC_QTY; i++) {
        if (marker == shape->_marker[i]) {
            SPItemView *v;
            /* Hide marker */
            for (v = item->display; v != NULL; v = v->next) {
              sp_marker_hide ((SPMarker *) (shape->_marker[i]), v->arenaitem->key() + i);
            }
            /* Detach marker */
            shape->_release_connect[i].disconnect();
            shape->_modified_connect[i].disconnect();
            shape->_marker[i] = sp_object_hunref (shape->_marker[i], item);
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
    /* g_warning ("Item %s mask %s modified", item->getId(), mask->getId()); */
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

    if (key > SP_MARKER_LOC_END) {
        return;
    }

    SPObject *mrk = sp_css_uri_reference_resolve(object->document, value);
    if (mrk != shape->_marker[key]) {
        if (shape->_marker[key]) {
            SPItemView *v;

            /* Detach marker */
            shape->_release_connect[key].disconnect();
            shape->_modified_connect[key].disconnect();

            /* Hide marker */
            for (v = item->display; v != NULL; v = v->next) {
                sp_marker_hide ((SPMarker *) (shape->_marker[key]),
                                v->arenaitem->key() + key);
            }

            /* Unref marker */
            shape->_marker[key] = sp_object_hunref (shape->_marker[key], object);
        }
        if (SP_IS_MARKER (mrk)) {
            shape->_marker[key] = sp_object_href (mrk, object);
            shape->_release_connect[key] = mrk->connectRelease(sigc::bind<1>(sigc::ptr_fun(&sp_shape_marker_release), shape));
            shape->_modified_connect[key] = mrk->connectModified(sigc::bind<2>(sigc::ptr_fun(&sp_shape_marker_modified), shape));
        }
    }
}



/* Shape section */

/**
 * Calls any registered handlers for the set_shape action
 */
void SPShape::setShape()
{
    if (SP_SHAPE_CLASS (G_OBJECT_GET_CLASS (this))->set_shape) {
      SP_SHAPE_CLASS (G_OBJECT_GET_CLASS (this))->set_shape (this);
    }
}

/**
 * Adds a curve to the shape.  If owner is specified, a reference
 * will be made, otherwise the curve will be copied into the shape.
 * Any existing curve in the shape will be unreferenced first.
 * This routine also triggers a request to update the display.
 */
void SPShape::setCurve(SPCurve *new_curve, unsigned int owner)
{
    if (_curve) {
        _curve = _curve->unref();
    }
    if (new_curve) {
        if (owner) {
            _curve = new_curve->ref();
        } else {
            _curve = new_curve->copy();
        }
    }
    this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Sets _curve_before_lpe to refer to the curve.
 */
void
SPShape::setCurveBeforeLPE (SPCurve *new_curve)
{
    if (_curve_before_lpe) {
        _curve_before_lpe = _curve_before_lpe->unref();
    }
    if (new_curve) {
        _curve_before_lpe = new_curve->ref();
    }
}

/**
 * Return duplicate of curve (if any exists) or NULL if there is no curve
 */
SPCurve * SPShape::getCurve() const
{
    if (_curve) {
        return _curve->copy();
    }
    return NULL;
}

/**
 * Return duplicate of curve *before* LPE (if any exists) or NULL if there is no curve
 */
SPCurve * SPShape::getCurveBeforeLPE() const
{
    if (sp_lpe_item_has_path_effect(this)) {
        if (_curve_before_lpe) {
            return this->_curve_before_lpe->copy();
        }
    } else {
        if (_curve) {
            return _curve->copy();
        }
    }
    return NULL;
}

/**
 * Same as sp_shape_set_curve but without updating the display
 */
void SPShape::setCurveInsync(SPCurve *new_curve, unsigned int owner)
{
    if (_curve) {
        _curve = _curve->unref();
    }
    if (new_curve) {
        if (owner) {
            _curve = new_curve->ref();
        } else {
            _curve = new_curve->copy();
        }
    }
}

/**
 * Return all nodes in a path that are to be considered for snapping
 */
void SPShape::sp_shape_snappoints(SPItem const *item, std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs)
{
    g_assert(item != NULL);
    g_assert(SP_IS_SHAPE(item));

    SPShape const *shape = SP_SHAPE(item);
    if (shape->_curve == NULL) {
        return;
    }

    Geom::PathVector const &pathv = shape->_curve->get_pathvector();
    if (pathv.empty())
        return;

    Geom::Affine const i2dt (item->i2dt_affine ());

    if (snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_OBJECT_MIDPOINT)) {
        Geom::OptRect bbox = item->desktopVisualBounds();
        if (bbox) {
            p.push_back(Inkscape::SnapCandidatePoint(bbox->midpoint(), Inkscape::SNAPSOURCE_OBJECT_MIDPOINT, Inkscape::SNAPTARGET_OBJECT_MIDPOINT));
        }
    }

    for(Geom::PathVector::const_iterator path_it = pathv.begin(); path_it != pathv.end(); ++path_it) {
        if (snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_NODE_CUSP)) {
            // Add the first point of the path
            p.push_back(Inkscape::SnapCandidatePoint(path_it->initialPoint() * i2dt, Inkscape::SNAPSOURCE_NODE_CUSP, Inkscape::SNAPTARGET_NODE_CUSP));
        }

        Geom::Path::const_iterator curve_it1 = path_it->begin();      // incoming curve
        Geom::Path::const_iterator curve_it2 = ++(path_it->begin());  // outgoing curve
        while (curve_it1 != path_it->end_default())
        {
            // For each path: consider midpoints of line segments for snapping
            if (snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_LINE_MIDPOINT)) {
                if (Geom::LineSegment const* line_segment = dynamic_cast<Geom::LineSegment const*>(&(*curve_it1))) {
                    p.push_back(Inkscape::SnapCandidatePoint(Geom::middle_point(*line_segment) * i2dt, Inkscape::SNAPSOURCE_LINE_MIDPOINT, Inkscape::SNAPTARGET_LINE_MIDPOINT));
                }
            }

            if (curve_it2 == path_it->end_default()) { // Test will only pass for the last iteration of the while loop
                if (snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_NODE_CUSP) && !path_it->closed()) {
                    // Add the last point of the path, but only for open paths
                    // (for closed paths the first and last point will coincide)
                    p.push_back(Inkscape::SnapCandidatePoint((*curve_it1).finalPoint() * i2dt, Inkscape::SNAPSOURCE_NODE_CUSP, Inkscape::SNAPTARGET_NODE_CUSP));
                }
            } else {
                /* Test whether to add the node between curve_it1 and curve_it2.
                 * Loop to end_default (so only iterating through the stroked part); */

                Geom::NodeType nodetype = Geom::get_nodetype(*curve_it1, *curve_it2);

                bool c1 = snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_NODE_CUSP) && (nodetype == Geom::NODE_CUSP || nodetype == Geom::NODE_NONE);
                bool c2 = snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_NODE_SMOOTH) && (nodetype == Geom::NODE_SMOOTH || nodetype == Geom::NODE_SYMM);

                if (c1 || c2) {
                    Inkscape::SnapSourceType sst;
                    Inkscape::SnapTargetType stt;
                    switch (nodetype) {
                    case Geom::NODE_CUSP:
                        sst = Inkscape::SNAPSOURCE_NODE_CUSP;
                        stt = Inkscape::SNAPTARGET_NODE_CUSP;
                        break;
                    case Geom::NODE_SMOOTH:
                    case Geom::NODE_SYMM:
                        sst = Inkscape::SNAPSOURCE_NODE_SMOOTH;
                        stt = Inkscape::SNAPTARGET_NODE_SMOOTH;
                        break;
                    default:
                        sst = Inkscape::SNAPSOURCE_UNDEFINED;
                        stt = Inkscape::SNAPTARGET_UNDEFINED;
                        break;
                    }
                    p.push_back(Inkscape::SnapCandidatePoint(curve_it1->finalPoint() * i2dt, sst, stt));
                }
            }

            ++curve_it1;
            ++curve_it2;
        }

        // Find the internal intersections of each path and consider these for snapping
        // (using "Method 1" as described in Inkscape::ObjectSnapper::_collectNodes())
        if (snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_PATH_INTERSECTION)) {
            Geom::Crossings cs;
            try {
                cs = self_crossings(*path_it);
                if (!cs.empty()) { // There might be multiple intersections...
                    for (Geom::Crossings::const_iterator i = cs.begin(); i != cs.end(); ++i) {
                        Geom::Point p_ix = (*path_it).pointAt((*i).ta);
                        p.push_back(Inkscape::SnapCandidatePoint(p_ix * i2dt, Inkscape::SNAPSOURCE_PATH_INTERSECTION, Inkscape::SNAPTARGET_PATH_INTERSECTION));
                    }
                }
            } catch (Geom::RangeError &e) {
                // do nothing
                // The exception could be Geom::InfiniteSolutions: then no snappoints should be added
            }

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

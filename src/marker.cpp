#define __MARKER_C__

/*
 * SVG <marker> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Bryce Harrington <bryce@bryceharrington.org>
 *
 * Copyright (C) 1999-2003 Lauris Kaplinski
 *               2004-2006 Bryce Harrington
 *               2008      Johan Engelen
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <string>
#include "config.h"


#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-matrix-ops.h"
#include "libnr/nr-matrix-translate-ops.h"
#include "libnr/nr-scale-matrix-ops.h"
#include "libnr/nr-translate-matrix-ops.h"
#include "libnr/nr-convert2geom.h"
#include <2geom/matrix.h>
#include "svg/svg.h"
#include "display/nr-arena-group.h"
#include "xml/repr.h"
#include "attributes.h"
#include "marker.h"
#include "document.h"
#include "document-private.h"

struct SPMarkerView {
	SPMarkerView *next;
	unsigned int key;
	unsigned int size;
	NRArenaItem *items[1];
};

static void sp_marker_class_init (SPMarkerClass *klass);
static void sp_marker_init (SPMarker *marker);

static void sp_marker_build (SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_marker_release (SPObject *object);
static void sp_marker_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_marker_update (SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_marker_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static NRArenaItem *sp_marker_private_show (SPItem *item, NRArena *arena, unsigned int key, unsigned int flags);
static void sp_marker_private_hide (SPItem *item, unsigned int key);
static void sp_marker_bbox(SPItem const *item, NRRect *bbox, Geom::Matrix const &transform, unsigned const flags);
static void sp_marker_print (SPItem *item, SPPrintContext *ctx);

static void sp_marker_view_remove (SPMarker *marker, SPMarkerView *view, unsigned int destroyitems);

static SPGroupClass *parent_class;

/**
 * Registers the SPMarker class with Gdk and returns its type number.
 */
GType
sp_marker_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPMarkerClass),
			NULL, NULL,
			(GClassInitFunc) sp_marker_class_init,
			NULL, NULL,
			sizeof (SPMarker),
			16,
			(GInstanceInitFunc) sp_marker_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_GROUP, "SPMarker", &info, (GTypeFlags)0);
	}
	return type;
}

/**
 * Initializes a SPMarkerClass object.  Establishes the function pointers to the class'
 * member routines in the class vtable, and sets pointers to parent classes.
 */
static void
sp_marker_class_init (SPMarkerClass *klass)
{
	GObjectClass *object_class;
	SPObjectClass *sp_object_class;
	SPItemClass *sp_item_class;

	object_class = G_OBJECT_CLASS (klass);
	sp_object_class = (SPObjectClass *) klass;
	sp_item_class = (SPItemClass *) klass;

	parent_class = (SPGroupClass *)g_type_class_ref (SP_TYPE_GROUP);

	sp_object_class->build = sp_marker_build;
	sp_object_class->release = sp_marker_release;
	sp_object_class->set = sp_marker_set;
	sp_object_class->update = sp_marker_update;
	sp_object_class->write = sp_marker_write;

	sp_item_class->show = sp_marker_private_show;
	sp_item_class->hide = sp_marker_private_hide;
	sp_item_class->bbox = sp_marker_bbox;
	sp_item_class->print = sp_marker_print;
}

/**
 * Initializes an SPMarker object.  This notes the marker's viewBox is
 * not set and initializes the marker's c2p identity matrix.
 */
static void
sp_marker_init (SPMarker *marker)
{
	marker->viewBox_set = FALSE;

	marker->c2p.setIdentity();
}

/**
 * Virtual build callback for SPMarker.
 *
 * This is to be invoked immediately after creation of an SPMarker.  This
 * method fills an SPMarker object with its SVG attributes, and calls the
 * parent class' build routine to attach the object to its document and
 * repr.  The result will be creation of the whole document tree.
 *
 * \see sp_object_build()
 */
static void
sp_marker_build (SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
	SPGroup *group;
	SPMarker *marker;

	group = (SPGroup *) object;
	marker = (SPMarker *) object;

	sp_object_read_attr (object, "markerUnits");
	sp_object_read_attr (object, "refX");
	sp_object_read_attr (object, "refY");
	sp_object_read_attr (object, "markerWidth");
	sp_object_read_attr (object, "markerHeight");
	sp_object_read_attr (object, "orient");
	sp_object_read_attr (object, "viewBox");
	sp_object_read_attr (object, "preserveAspectRatio");

	if (((SPObjectClass *) parent_class)->build)
		((SPObjectClass *) parent_class)->build (object, document, repr);
}

/**
 * Removes, releases and unrefs all children of object
 *
 * This is the inverse of sp_marker_build().  It must be invoked as soon
 * as the marker is removed from the tree, even if it is still referenced
 * by other objects.  It hides and removes any views of the marker, then
 * calls the parent classes' release function to deregister the object
 * and release its SPRepr bindings.  The result will be the destruction
 * of the entire document tree.
 *
 * \see sp_object_release()
 */
static void
sp_marker_release (SPObject *object)
{
	SPMarker *marker;

	marker = (SPMarker *) object;

	while (marker->views) {
		/* Destroy all NRArenaitems etc. */
		/* Parent class ::hide method */
		((SPItemClass *) parent_class)->hide ((SPItem *) marker, marker->views->key);
		sp_marker_view_remove (marker, marker->views, TRUE);
	}

	if (((SPObjectClass *) parent_class)->release)
		((SPObjectClass *) parent_class)->release (object);
}

/**
 * Sets an attribute, 'key', of a marker object to 'value'.  Supported
 * attributes that can be set with this routine include:
 *
 *     SP_ATTR_MARKERUNITS
 *     SP_ATTR_REFX
 *     SP_ATTR_REFY
 *     SP_ATTR_MARKERWIDTH
 *     SP_ATTR_MARKERHEIGHT
 *     SP_ATTR_ORIENT
 *     SP_ATTR_VIEWBOX
 *     SP_ATTR_PRESERVEASPECTRATIO
 */
static void
sp_marker_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPItem *item;
	SPMarker *marker;

	item = SP_ITEM (object);
	marker = SP_MARKER (object);

	switch (key) {
	case SP_ATTR_MARKERUNITS:
		marker->markerUnits_set = FALSE;
		marker->markerUnits = SP_MARKER_UNITS_STROKEWIDTH;
		if (value) {
			if (!strcmp (value, "strokeWidth")) {
				marker->markerUnits_set = TRUE;
			} else if (!strcmp (value, "userSpaceOnUse")) {
				marker->markerUnits = SP_MARKER_UNITS_USERSPACEONUSE;
				marker->markerUnits_set = TRUE;
			}
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
		break;
	case SP_ATTR_REFX:
	        marker->refX.readOrUnset(value);
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_REFY:
	        marker->refY.readOrUnset(value);
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_MARKERWIDTH:
	        marker->markerWidth.readOrUnset(value, SVGLength::NONE, 3.0, 3.0);
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_MARKERHEIGHT:
	        marker->markerHeight.readOrUnset(value, SVGLength::NONE, 3.0, 3.0);
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_ORIENT:
		marker->orient_set = FALSE;
		marker->orient_auto = FALSE;
		marker->orient = 0.0;
		if (value) {
			if (!strcmp (value, "auto")) {
				marker->orient_auto = TRUE;
				marker->orient_set = TRUE;
			} else if (sp_svg_number_read_f (value, &marker->orient)) {
				marker->orient_set = TRUE;
			}
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_VIEWBOX:
		marker->viewBox_set = FALSE;
		if (value) {
			double x, y, width, height;
			char *eptr;
			/* fixme: We have to take original item affine into account */
			/* fixme: Think (Lauris) */
			eptr = (gchar *) value;
			x = g_ascii_strtod (eptr, &eptr);
			while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
			y = g_ascii_strtod (eptr, &eptr);
			while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
			width = g_ascii_strtod (eptr, &eptr);
			while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
			height = g_ascii_strtod (eptr, &eptr);
			while (*eptr && ((*eptr == ',') || (*eptr == ' '))) eptr++;
			if ((width > 0) && (height > 0)) {
				/* Set viewbox */
				marker->viewBox.x0 = x;
				marker->viewBox.y0 = y;
				marker->viewBox.x1 = x + width;
				marker->viewBox.y1 = y + height;
				marker->viewBox_set = TRUE;
			}
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
		break;
	case SP_ATTR_PRESERVEASPECTRATIO:
		/* Do setup before, so we can use break to escape */
		marker->aspect_set = FALSE;
		marker->aspect_align = SP_ASPECT_NONE;
		marker->aspect_clip = SP_ASPECT_MEET;
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
		if (value) {
			int len;
			gchar c[256];
			const gchar *p, *e;
			unsigned int align, clip;
			p = value;
			while (*p && *p == 32) p += 1;
			if (!*p) break;
			e = p;
			while (*e && *e != 32) e += 1;
			len = e - p;
			if (len > 8) break;
			memcpy (c, value, len);
			c[len] = 0;
			/* Now the actual part */
			if (!strcmp (c, "none")) {
				align = SP_ASPECT_NONE;
			} else if (!strcmp (c, "xMinYMin")) {
				align = SP_ASPECT_XMIN_YMIN;
			} else if (!strcmp (c, "xMidYMin")) {
				align = SP_ASPECT_XMID_YMIN;
			} else if (!strcmp (c, "xMaxYMin")) {
				align = SP_ASPECT_XMAX_YMIN;
			} else if (!strcmp (c, "xMinYMid")) {
				align = SP_ASPECT_XMIN_YMID;
			} else if (!strcmp (c, "xMidYMid")) {
				align = SP_ASPECT_XMID_YMID;
			} else if (!strcmp (c, "xMaxYMin")) {
				align = SP_ASPECT_XMAX_YMID;
			} else if (!strcmp (c, "xMinYMax")) {
				align = SP_ASPECT_XMIN_YMAX;
			} else if (!strcmp (c, "xMidYMax")) {
				align = SP_ASPECT_XMID_YMAX;
			} else if (!strcmp (c, "xMaxYMax")) {
				align = SP_ASPECT_XMAX_YMAX;
			} else {
				break;
			}
			clip = SP_ASPECT_MEET;
			while (*e && *e == 32) e += 1;
			if (e) {
				if (!strcmp (e, "meet")) {
					clip = SP_ASPECT_MEET;
				} else if (!strcmp (e, "slice")) {
					clip = SP_ASPECT_SLICE;
				} else {
					break;
				}
			}
			marker->aspect_set = TRUE;
			marker->aspect_align = align;
			marker->aspect_clip = clip;
		}
		break;
	default:
		if (((SPObjectClass *) parent_class)->set)
			((SPObjectClass *) parent_class)->set (object, key, value);
		break;
	}
}

/**
 * Updates <marker> when its attributes have changed.  Takes care of setting up
 * transformations and viewBoxes.
 */
static void
sp_marker_update (SPObject *object, SPCtx *ctx, guint flags)
{
	SPItem *item;
	SPMarker *marker;
	SPItemCtx rctx;
	NRRect *vb;
	double x, y, width, height;
	SPMarkerView *v;

	item = SP_ITEM (object);
	marker = SP_MARKER (object);

	/* fixme: We have to set up clip here too */

	/* Copy parent context */
	rctx.ctx = *ctx;
	/* Initialize tranformations */
	rctx.i2doc = NR::identity();
	rctx.i2vp = NR::identity();
	/* Set up viewport */
	rctx.vp.x0 = 0.0;
	rctx.vp.y0 = 0.0;
	rctx.vp.x1 = marker->markerWidth.computed;
	rctx.vp.y1 = marker->markerHeight.computed;

	/* Start with identity transform */
	marker->c2p.setIdentity();

	/* Viewbox is always present, either implicitly or explicitly */
	if (marker->viewBox_set) {
		vb = &marker->viewBox;
	} else {
		vb = &rctx.vp;
	}
	/* Now set up viewbox transformation */
	/* Determine actual viewbox in viewport coordinates */
	if (marker->aspect_align == SP_ASPECT_NONE) {
		x = 0.0;
		y = 0.0;
		width = rctx.vp.x1 - rctx.vp.x0;
		height = rctx.vp.y1 - rctx.vp.y0;
	} else {
		double scalex, scaley, scale;
		/* Things are getting interesting */
		scalex = (rctx.vp.x1 - rctx.vp.x0) / (vb->x1 - vb->x0);
		scaley = (rctx.vp.y1 - rctx.vp.y0) / (vb->y1 - vb->y0);
		scale = (marker->aspect_clip == SP_ASPECT_MEET) ? MIN (scalex, scaley) : MAX (scalex, scaley);
		width = (vb->x1 - vb->x0) * scale;
		height = (vb->y1 - vb->y0) * scale;
		/* Now place viewbox to requested position */
		switch (marker->aspect_align) {
		case SP_ASPECT_XMIN_YMIN:
			x = 0.0;
			y = 0.0;
			break;
		case SP_ASPECT_XMID_YMIN:
			x = 0.5 * ((rctx.vp.x1 - rctx.vp.x0) - width);
			y = 0.0;
			break;
		case SP_ASPECT_XMAX_YMIN:
			x = 1.0 * ((rctx.vp.x1 - rctx.vp.x0) - width);
			y = 0.0;
			break;
		case SP_ASPECT_XMIN_YMID:
			x = 0.0;
			y = 0.5 * ((rctx.vp.y1 - rctx.vp.y0) - height);
			break;
		case SP_ASPECT_XMID_YMID:
			x = 0.5 * ((rctx.vp.x1 - rctx.vp.x0) - width);
			y = 0.5 * ((rctx.vp.y1 - rctx.vp.y0) - height);
			break;
		case SP_ASPECT_XMAX_YMID:
			x = 1.0 * ((rctx.vp.x1 - rctx.vp.x0) - width);
			y = 0.5 * ((rctx.vp.y1 - rctx.vp.y0) - height);
			break;
		case SP_ASPECT_XMIN_YMAX:
			x = 0.0;
			y = 1.0 * ((rctx.vp.y1 - rctx.vp.y0) - height);
			break;
		case SP_ASPECT_XMID_YMAX:
			x = 0.5 * ((rctx.vp.x1 - rctx.vp.x0) - width);
			y = 1.0 * ((rctx.vp.y1 - rctx.vp.y0) - height);
			break;
		case SP_ASPECT_XMAX_YMAX:
			x = 1.0 * ((rctx.vp.x1 - rctx.vp.x0) - width);
			y = 1.0 * ((rctx.vp.y1 - rctx.vp.y0) - height);
			break;
		default:
			x = 0.0;
			y = 0.0;
			break;
		}
	}

    {
        Geom::Matrix q;
	    /* Compose additional transformation from scale and position */
	    q[0] = width / (vb->x1 - vb->x0);
	    q[1] = 0.0;
	    q[2] = 0.0;
	    q[3] = height / (vb->y1 - vb->y0);
	    q[4] = -vb->x0 * q[0] + x;
	    q[5] = -vb->y0 * q[3] + y;
	    /* Append viewbox transformation */
	    marker->c2p = q * marker->c2p;
    }

	/* Append reference translation */
	/* fixme: lala (Lauris) */
        marker->c2p = Geom::Translate(-marker->refX.computed, -marker->refY.computed) * marker->c2p;

	rctx.i2doc = marker->c2p * rctx.i2doc;

	/* If viewBox is set reinitialize child viewport */
	/* Otherwise it already correct */
	if (marker->viewBox_set) {
            rctx.vp.x0 = marker->viewBox.x0;
            rctx.vp.y0 = marker->viewBox.y0;
            rctx.vp.x1 = marker->viewBox.x1;
            rctx.vp.y1 = marker->viewBox.y1;
            rctx.i2vp = Geom::identity();
	}

	/* And invoke parent method */
	if (((SPObjectClass *) (parent_class))->update)
		((SPObjectClass *) (parent_class))->update (object, (SPCtx *) &rctx, flags);

	/* As last step set additional transform of arena group */
	for (v = marker->views; v != NULL; v = v->next) {
            for (unsigned i = 0 ; i < v->size ; i++) {
                if (v->items[i]) {
                    NR::Matrix tmp = from_2geom(marker->c2p);
                    nr_arena_group_set_child_transform(NR_ARENA_GROUP(v->items[i]), &tmp);
                }
            }
	}
}

/**
 * Writes the object's properties into its repr object.
 */
static Inkscape::XML::Node *
sp_marker_write (SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
	SPMarker *marker;

	marker = SP_MARKER (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = xml_doc->createElement("svg:marker");
	}

	if (marker->markerUnits_set) {
		if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
			repr->setAttribute("markerUnits", "strokeWidth");
		} else {
			repr->setAttribute("markerUnits", "userSpaceOnUse");
		}
	} else {
		repr->setAttribute("markerUnits", NULL);
	}
	if (marker->refX._set) {
		sp_repr_set_svg_double(repr, "refX", marker->refX.computed);
	} else {
		repr->setAttribute("refX", NULL);
	}
	if (marker->refY._set) {
		sp_repr_set_svg_double (repr, "refY", marker->refY.computed);
	} else {
		repr->setAttribute("refY", NULL);
	}
	if (marker->markerWidth._set) {
		sp_repr_set_svg_double (repr, "markerWidth", marker->markerWidth.computed);
	} else {
		repr->setAttribute("markerWidth", NULL);
	}
	if (marker->markerHeight._set) {
		sp_repr_set_svg_double (repr, "markerHeight", marker->markerHeight.computed);
	} else {
		repr->setAttribute("markerHeight", NULL);
	}
	if (marker->orient_set) {
		if (marker->orient_auto) {
			repr->setAttribute("orient", "auto");
		} else {
			sp_repr_set_css_double(repr, "orient", marker->orient);
		}
	} else {
		repr->setAttribute("orient", NULL);
	}
	/* fixme: */
	repr->setAttribute("viewBox", object->repr->attribute("viewBox"));
	repr->setAttribute("preserveAspectRatio", object->repr->attribute("preserveAspectRatio"));

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, xml_doc, repr, flags);

	return repr;
}

/**
 * This routine is disabled to break propagation.
 */
static NRArenaItem *
sp_marker_private_show (SPItem */*item*/, NRArena */*arena*/, unsigned int /*key*/, unsigned int /*flags*/)
{
    /* Break propagation */
    return NULL;
}

/**
 * This routine is disabled to break propagation.
 */
static void
sp_marker_private_hide (SPItem */*item*/, unsigned int /*key*/)
{
    /* Break propagation */
}

/**
 * This routine is disabled to break propagation.
 */
static void
sp_marker_bbox(SPItem const *, NRRect *, Geom::Matrix const &, unsigned const)
{
	/* Break propagation */
}

/**
 * This routine is disabled to break propagation.
 */
static void
sp_marker_print (SPItem */*item*/, SPPrintContext */*ctx*/)
{
    /* Break propagation */
}

/* fixme: Remove link if zero-sized (Lauris) */

/**
 * Removes any SPMarkerViews that a marker has with a specific key.
 * Set up the NRArenaItem array's size in the specified SPMarker's SPMarkerView.
 * This is called from sp_shape_update() for shapes that have markers.  It
 * removes the old view of the marker and establishes a new one, registering
 * it with the marker's list of views for future updates.
 *
 * \param marker Marker to create views in.
 * \param key Key to give each SPMarkerView.
 * \param size Number of NRArenaItems to put in the SPMarkerView.
 */
void
sp_marker_show_dimension (SPMarker *marker, unsigned int key, unsigned int size)
{
	SPMarkerView *view;
	unsigned int i;

	for (view = marker->views; view != NULL; view = view->next) {
		if (view->key == key) break;
	}
	if (view && (view->size != size)) {
		/* Free old view and allocate new */
		/* Parent class ::hide method */
		((SPItemClass *) parent_class)->hide ((SPItem *) marker, key);
		sp_marker_view_remove (marker, view, TRUE);
		view = NULL;
	}
	if (!view) {
		view = (SPMarkerView *)g_malloc (sizeof (SPMarkerView) + (size) * sizeof (NRArenaItem *));
		for (i = 0; i < size; i++) view->items[i] = NULL;
		view->next = marker->views;
		marker->views = view;
		view->key = key;
		view->size = size;
	}
}

/**
 * Shows an instance of a marker.  This is called during sp_shape_update_marker_view()
 * show and transform a child item in the arena for all views with the given key.
 */
NRArenaItem *
sp_marker_show_instance ( SPMarker *marker, NRArenaItem *parent,
                          unsigned int key, unsigned int pos,
                          Geom::Matrix const &base, float linewidth)
{
    for (SPMarkerView *v = marker->views; v != NULL; v = v->next) {
        if (v->key == key) {
            if (pos >= v->size) {
                return NULL;
            }
            if (!v->items[pos]) {
                /* Parent class ::show method */
                v->items[pos] = ((SPItemClass *) parent_class)->show ((SPItem *) marker,
                                                                      parent->arena, key,
                                                                      SP_ITEM_REFERENCE_FLAGS);
                if (v->items[pos]) {
                    /* fixme: Position (Lauris) */
                    nr_arena_item_add_child (parent, v->items[pos], NULL);
                    /* nr_arena_item_unref (v->items[pos]); */
                    NR::Matrix tmp = from_2geom(marker->c2p);
                    nr_arena_group_set_child_transform((NRArenaGroup *) v->items[pos], &tmp);
                }
            }
            if (v->items[pos]) {
                Geom::Matrix m;
                if (marker->orient_auto) {
                    m = base;
                } else {
                    /* fixme: Orient units (Lauris) */
                    m = Geom::Matrix(Geom::Rotate::from_degrees(marker->orient));
                    m *= Geom::Translate(base[4], base[5]); // TODO: this was NR::get_translation() originally; should it be extracted into a new 2geom function?
                }
                if (marker->markerUnits == SP_MARKER_UNITS_STROKEWIDTH) {
                    m = Geom::Scale(linewidth) * m;
                }
                
                nr_arena_item_set_transform(v->items[pos], m);
            }
            return v->items[pos];
        }
    }

    return NULL;
}

/**
 * Hides/removes all views of the given marker that have key 'key'.
 * This replaces SPItem implementation because we have our own views
 * \param key SPMarkerView key to hide.
 */
void
sp_marker_hide (SPMarker *marker, unsigned int key)
{
	SPMarkerView *v;

	v = marker->views;
	while (v != NULL) {
		SPMarkerView *next;
		next = v->next;
		if (v->key == key) {
			/* Parent class ::hide method */
			((SPItemClass *) parent_class)->hide ((SPItem *) marker, key);
			sp_marker_view_remove (marker, v, TRUE);
			return;
		}
		v = next;
	}
}

/**
 * Removes a given view.  Also will destroy sub-items in the view if destroyitems
 * is set to a non-zero value.
 */
static void
sp_marker_view_remove (SPMarker *marker, SPMarkerView *view, unsigned int destroyitems)
{
	unsigned int i;
	if (view == marker->views) {
		marker->views = view->next;
	} else {
		SPMarkerView *v;
		for (v = marker->views; v->next != view; v = v->next) if (!v->next) return;
		v->next = view->next;
	}
	if (destroyitems) {
		for (i = 0; i < view->size; i++) {
			/* We have to walk through the whole array because there may be hidden items */
			if (view->items[i]) nr_arena_item_unref (view->items[i]);
		}
	}
	g_free (view);
}

const gchar *
generate_marker (GSList *reprs, NR::Rect bounds, SPDocument *document, Geom::Matrix /*transform*/, Geom::Matrix move)
{
    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(document);
    Inkscape::XML::Node *defsrepr = SP_OBJECT_REPR (SP_DOCUMENT_DEFS (document));

    Inkscape::XML::Node *repr = xml_doc->createElement("svg:marker");

    // Uncommenting this will make the marker fixed-size independent of stroke width.
    // Commented out for consistency with standard markers which scale when you change
    // stroke width:
    //repr->setAttribute("markerUnits", "userSpaceOnUse");

    sp_repr_set_svg_double(repr, "markerWidth", bounds.extent(NR::X));
    sp_repr_set_svg_double(repr, "markerHeight", bounds.extent(NR::Y));

    repr->setAttribute("orient", "auto");

    defsrepr->appendChild(repr);
    const gchar *mark_id = repr->attribute("id");
    SPObject *mark_object = document->getObjectById(mark_id);

    for (GSList *i = reprs; i != NULL; i = i->next) {
            Inkscape::XML::Node *node = (Inkscape::XML::Node *)(i->data);
        SPItem *copy = SP_ITEM(mark_object->appendChildRepr(node));

        Geom::Matrix dup_transform;
        if (!sp_svg_transform_read (node->attribute("transform"), &dup_transform))
            dup_transform = Geom::identity();
        dup_transform *= move;

        sp_item_write_transform(copy, SP_OBJECT_REPR(copy), dup_transform);
    }

    Inkscape::GC::release(repr);
    return mark_id;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

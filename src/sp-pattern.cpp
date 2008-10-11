#define __SP_PATTERN_C__

/*
 * SVG <pattern> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>
#include <libnr/nr-matrix-ops.h>
#include "libnr/nr-matrix-fns.h"
#include <2geom/transforms.h>
#include "macros.h"
#include "svg/svg.h"
#include "display/nr-arena.h"
#include "display/nr-arena-group.h"
#include "attributes.h"
#include "document-private.h"
#include "uri.h"
#include "sp-pattern.h"
#include "xml/repr.h"

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

/*
 * Pattern
 */

class SPPatPainter;

struct SPPatPainter {
	SPPainter painter;
	SPPattern *pat;

	NR::Matrix ps2px;
	NR::Matrix px2ps;
	NR::Matrix pcs2px;

	NRArena *arena;
	unsigned int dkey;
	NRArenaItem *root;
	
	bool         use_cached_tile;
	NR::Matrix     ca2pa;
	NR::Matrix     pa2ca;
	NRRectL      cached_bbox;
	NRPixBlock   cached_tile;
};

static void sp_pattern_class_init (SPPatternClass *klass);
static void sp_pattern_init (SPPattern *gr);

static void sp_pattern_build (SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_pattern_release (SPObject *object);
static void sp_pattern_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_pattern_child_added (SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
static void sp_pattern_update (SPObject *object, SPCtx *ctx, unsigned int flags);
static void sp_pattern_modified (SPObject *object, unsigned int flags);

static void pattern_ref_changed(SPObject *old_ref, SPObject *ref, SPPattern *pat);
static void pattern_ref_modified (SPObject *ref, guint flags, SPPattern *pattern);

static SPPainter *sp_pattern_painter_new (SPPaintServer *ps, NR::Matrix const &full_transform, NR::Matrix const &parent_transform, const NRRect *bbox);
static void sp_pattern_painter_free (SPPaintServer *ps, SPPainter *painter);

static SPPaintServerClass * pattern_parent_class;

GType
sp_pattern_get_type (void)
{
	static GType pattern_type = 0;
	if (!pattern_type) {
		GTypeInfo pattern_info = {
			sizeof (SPPatternClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_pattern_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPPattern),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_pattern_init,
			NULL,	/* value_table */
		};
		pattern_type = g_type_register_static (SP_TYPE_PAINT_SERVER, "SPPattern", &pattern_info, (GTypeFlags)0);
	}
	return pattern_type;
}

static void
sp_pattern_class_init (SPPatternClass *klass)
{
	SPObjectClass *sp_object_class;
	SPPaintServerClass *ps_class;

	sp_object_class = (SPObjectClass *) klass;
	ps_class = (SPPaintServerClass *) klass;

	pattern_parent_class = (SPPaintServerClass*)g_type_class_ref (SP_TYPE_PAINT_SERVER);

	sp_object_class->build = sp_pattern_build;
	sp_object_class->release = sp_pattern_release;
	sp_object_class->set = sp_pattern_set;
	sp_object_class->child_added = sp_pattern_child_added;
	sp_object_class->update = sp_pattern_update;
	sp_object_class->modified = sp_pattern_modified;

	// do we need _write? seems to work without it

	ps_class->painter_new = sp_pattern_painter_new;
	ps_class->painter_free = sp_pattern_painter_free;
}

static void
sp_pattern_init (SPPattern *pat)
{
	pat->ref = new SPPatternReference(SP_OBJECT(pat));
	pat->ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(pattern_ref_changed), pat));

	pat->patternUnits = SP_PATTERN_UNITS_OBJECTBOUNDINGBOX;
	pat->patternUnits_set = FALSE;

	pat->patternContentUnits = SP_PATTERN_UNITS_USERSPACEONUSE;
	pat->patternContentUnits_set = FALSE;

	pat->patternTransform = NR::identity();
	pat->patternTransform_set = FALSE;

	pat->x.unset();
	pat->y.unset();
	pat->width.unset();
	pat->height.unset();

	pat->viewBox_set = FALSE;

	new (&pat->modified_connection) sigc::connection();
}

static void
sp_pattern_build (SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
	if (((SPObjectClass *) pattern_parent_class)->build)
		(* ((SPObjectClass *) pattern_parent_class)->build) (object, document, repr);

	sp_object_read_attr (object, "patternUnits");
	sp_object_read_attr (object, "patternContentUnits");
	sp_object_read_attr (object, "patternTransform");
	sp_object_read_attr (object, "x");
	sp_object_read_attr (object, "y");
	sp_object_read_attr (object, "width");
	sp_object_read_attr (object, "height");
	sp_object_read_attr (object, "viewBox");
	sp_object_read_attr (object, "xlink:href");

	/* Register ourselves */
	sp_document_add_resource (document, "pattern", object);
}

static void
sp_pattern_release (SPObject *object)
{
	SPPattern *pat;

	pat = (SPPattern *) object;

	if (SP_OBJECT_DOCUMENT (object)) {
		/* Unregister ourselves */
		sp_document_remove_resource (SP_OBJECT_DOCUMENT (object), "pattern", SP_OBJECT (object));
	}

	if (pat->ref) {
		pat->modified_connection.disconnect();
		pat->ref->detach();
		delete pat->ref;
		pat->ref = NULL;
	}

	pat->modified_connection.~connection();

	if (((SPObjectClass *) pattern_parent_class)->release)
		((SPObjectClass *) pattern_parent_class)->release (object);
}

static void
sp_pattern_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPPattern *pat = SP_PATTERN (object);

	switch (key) {
	case SP_ATTR_PATTERNUNITS:
		if (value) {
			if (!strcmp (value, "userSpaceOnUse")) {
				pat->patternUnits = SP_PATTERN_UNITS_USERSPACEONUSE;
			} else {
				pat->patternUnits = SP_PATTERN_UNITS_OBJECTBOUNDINGBOX;
			}
			pat->patternUnits_set = TRUE;
		} else {
			pat->patternUnits_set = FALSE;
		}
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_PATTERNCONTENTUNITS:
		if (value) {
			if (!strcmp (value, "userSpaceOnUse")) {
				pat->patternContentUnits = SP_PATTERN_UNITS_USERSPACEONUSE;
			} else {
				pat->patternContentUnits = SP_PATTERN_UNITS_OBJECTBOUNDINGBOX;
			}
			pat->patternContentUnits_set = TRUE;
		} else {
			pat->patternContentUnits_set = FALSE;
		}
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_PATTERNTRANSFORM: {
		NR::Matrix t;
		if (value && sp_svg_transform_read (value, &t)) {
			pat->patternTransform = t;
			pat->patternTransform_set = TRUE;
		} else {
			pat->patternTransform = NR::identity();
			pat->patternTransform_set = FALSE;
		}
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	}
	case SP_ATTR_X:
	        pat->x.readOrUnset(value);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_Y:
	        pat->y.readOrUnset(value);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_WIDTH:
	        pat->width.readOrUnset(value);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_HEIGHT:
	        pat->height.readOrUnset(value);
		object->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_VIEWBOX: {
		/* fixme: Think (Lauris) */
		double x, y, width, height;
		char *eptr;

		if (value) {
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
				pat->viewBox.x0 = x;
				pat->viewBox.y0 = y;
				pat->viewBox.x1 = x + width;
				pat->viewBox.y1 = y + height;
				pat->viewBox_set = TRUE;
			} else {
				pat->viewBox_set = FALSE;
			}
		} else {
			pat->viewBox_set = FALSE;
		}
		object->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
		break;
	}
	case SP_ATTR_XLINK_HREF:
		if ( value && pat->href && ( strcmp(value, pat->href) == 0 ) ) {
			/* Href unchanged, do nothing. */
		} else {
			g_free(pat->href);
			pat->href = NULL;
			if (value) {
				// First, set the href field; it's only used in the "unchanged" check above.
				pat->href = g_strdup(value);
				// Now do the attaching, which emits the changed signal.
				if (value) {
					try {
						pat->ref->attach(Inkscape::URI(value));
					} catch (Inkscape::BadURIException &e) {
						g_warning("%s", e.what());
						pat->ref->detach();
					}
				} else {
					pat->ref->detach();
				}
			}
		}
		break;
	default:
		if (((SPObjectClass *) pattern_parent_class)->set)
			((SPObjectClass *) pattern_parent_class)->set (object, key, value);
		break;
	}
}

static void
sp_pattern_child_added (SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
	SPPattern *pat = SP_PATTERN (object);

	if (((SPObjectClass *) (pattern_parent_class))->child_added)
		(* ((SPObjectClass *) (pattern_parent_class))->child_added) (object, child, ref);

	SPObject *ochild = sp_object_get_child_by_repr(object, child);
	if (SP_IS_ITEM (ochild)) {

		SPPaintServer *ps = SP_PAINT_SERVER (pat);
		unsigned position = sp_item_pos_in_parent(SP_ITEM(ochild));

		for (SPPainter *p = ps->painters; p != NULL; p = p->next) {

			SPPatPainter *pp = (SPPatPainter *) p;
			NRArenaItem *ai = sp_item_invoke_show (SP_ITEM (ochild), pp->arena, pp->dkey, SP_ITEM_REFERENCE_FLAGS);

			if (ai) {
				nr_arena_item_add_child (pp->root, ai, NULL);
				nr_arena_item_set_order (ai, position);
				nr_arena_item_unref (ai);
			}
		}
	}
}

/* TODO: do we need a ::remove_child handler? */

/* fixme: We need ::order_changed handler too (Lauris) */

GSList *
pattern_getchildren (SPPattern *pat)
{
	GSList *l = NULL;

	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (sp_object_first_child(SP_OBJECT(pat_i))) { // find the first one with children
			for (SPObject *child = sp_object_first_child(SP_OBJECT (pat)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
				l = g_slist_prepend (l, child);
			}
			break; // do not go further up the chain if children are found
		}
	}

	return l;
}

static void
sp_pattern_update (SPObject *object, SPCtx *ctx, unsigned int flags)
{
	SPPattern *pat = SP_PATTERN (object);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	GSList *l = pattern_getchildren (pat);
	l = g_slist_reverse (l);

	while (l) {
		SPObject *child = SP_OBJECT (l->data);
		sp_object_ref (child, NULL);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			child->updateDisplay(ctx, flags);
		}
		sp_object_unref (child, NULL);
	}
}

static void
sp_pattern_modified (SPObject *object, guint flags)
{
	SPPattern *pat = SP_PATTERN (object);

	if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	GSList *l = pattern_getchildren (pat);
	l = g_slist_reverse (l);

	while (l) {
		SPObject *child = SP_OBJECT (l->data);
		sp_object_ref (child, NULL);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			child->emitModified(flags);
		}
		sp_object_unref (child, NULL);
	}
}

/**
Gets called when the pattern is reattached to another <pattern>
*/
static void
pattern_ref_changed(SPObject *old_ref, SPObject *ref, SPPattern *pat)
{
	if (old_ref) {
		pat->modified_connection.disconnect();
	}
	if (SP_IS_PATTERN (ref)) {
		pat->modified_connection = ref->connectModified(sigc::bind<2>(sigc::ptr_fun(&pattern_ref_modified), pat));
	}

	pattern_ref_modified (ref, 0, pat);
}

/**
Gets called when the referenced <pattern> is changed
*/
static void
pattern_ref_modified (SPObject */*ref*/, guint /*flags*/, SPPattern *pattern)
{
	if (SP_IS_OBJECT (pattern))
		SP_OBJECT (pattern)->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

guint
pattern_users (SPPattern *pattern)
{
	return SP_OBJECT (pattern)->hrefcount;
}

SPPattern *
pattern_chain (SPPattern *pattern)
{
	SPDocument *document = SP_OBJECT_DOCUMENT (pattern);
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(document);
	Inkscape::XML::Node *defsrepr = SP_OBJECT_REPR (SP_DOCUMENT_DEFS (document));

	Inkscape::XML::Node *repr = xml_doc->createElement("svg:pattern");
	repr->setAttribute("inkscape:collect", "always");
	gchar *parent_ref = g_strconcat ("#", SP_OBJECT_REPR(pattern)->attribute("id"), NULL);
	repr->setAttribute("xlink:href",  parent_ref);
	g_free (parent_ref);

	defsrepr->addChild(repr, NULL);
	const gchar *child_id = repr->attribute("id");
	SPObject *child = document->getObjectById(child_id);
	g_assert (SP_IS_PATTERN (child));

	return SP_PATTERN (child);
}

SPPattern *
sp_pattern_clone_if_necessary (SPItem *item, SPPattern *pattern, const gchar *property)
{
	if (pattern_users(pattern) > 1) {
		pattern = pattern_chain (pattern);
		gchar *href = g_strconcat ("url(#", SP_OBJECT_REPR (pattern)->attribute("id"), ")", NULL);

		SPCSSAttr *css = sp_repr_css_attr_new ();
		sp_repr_css_set_property (css, property, href);
		sp_repr_css_change_recursive (SP_OBJECT_REPR (item), css, "style");
	}
	return pattern;
}

void
sp_pattern_transform_multiply (SPPattern *pattern, NR::Matrix postmul, bool set)
{
	// this formula is for a different interpretation of pattern transforms as described in (*) in sp-pattern.cpp
	// for it to work, we also need    sp_object_read_attr (SP_OBJECT (item), "transform");
	//pattern->patternTransform = premul * item->transform * pattern->patternTransform * item->transform.inverse() * postmul;

	// otherwise the formula is much simpler
	if (set) {
		pattern->patternTransform = postmul;
	} else {
		pattern->patternTransform = pattern_patternTransform(pattern) * postmul;
	}
	pattern->patternTransform_set = TRUE;

	gchar *c=sp_svg_transform_write(pattern->patternTransform);
	SP_OBJECT_REPR(pattern)->setAttribute("patternTransform", c);
	g_free(c);
}

const gchar *
pattern_tile (GSList *reprs, NR::Rect bounds, SPDocument *document, NR::Matrix transform, NR::Matrix move)
{
	Inkscape::XML::Document *xml_doc = sp_document_repr_doc(document);
	Inkscape::XML::Node *defsrepr = SP_OBJECT_REPR (SP_DOCUMENT_DEFS (document));

	Inkscape::XML::Node *repr = xml_doc->createElement("svg:pattern");
	repr->setAttribute("patternUnits", "userSpaceOnUse");
	sp_repr_set_svg_double(repr, "width", bounds.extent(NR::X));
	sp_repr_set_svg_double(repr, "height", bounds.extent(NR::Y));

	gchar *t=sp_svg_transform_write(transform);
	repr->setAttribute("patternTransform", t);
	g_free(t);

	defsrepr->appendChild(repr);
	const gchar *pat_id = repr->attribute("id");
	SPObject *pat_object = document->getObjectById(pat_id);

	for (GSList *i = reprs; i != NULL; i = i->next) {
	        Inkscape::XML::Node *node = (Inkscape::XML::Node *)(i->data);
		SPItem *copy = SP_ITEM(pat_object->appendChildRepr(node));

		NR::Matrix dup_transform;
		if (!sp_svg_transform_read (node->attribute("transform"), &dup_transform))
			dup_transform = NR::identity();
		dup_transform *= move;

		sp_item_write_transform(copy, SP_OBJECT_REPR(copy), dup_transform);
	}

	Inkscape::GC::release(repr);
	return pat_id;
}

SPPattern *
pattern_getroot (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (sp_object_first_child(SP_OBJECT(pat_i))) { // find the first one with children
			return pat_i;
		}
	}
	return pat; // document is broken, we can't get to root; but at least we can return pat which is supposedly a valid pattern
}



// Access functions that look up fields up the chain of referenced patterns and return the first one which is set
// FIXME: all of them must use chase_hrefs the same as in SPGradient, to avoid lockup on circular refs

guint pattern_patternUnits (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->patternUnits_set)
			return pat_i->patternUnits;
	}
	return pat->patternUnits;
}

guint pattern_patternContentUnits (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->patternContentUnits_set)
			return pat_i->patternContentUnits;
	}
	return pat->patternContentUnits;
}

NR::Matrix const &pattern_patternTransform(SPPattern const *pat)
{
	for (SPPattern const *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->patternTransform_set)
			return pat_i->patternTransform;
	}
	return pat->patternTransform;
}

gdouble pattern_x (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->x._set)
			return pat_i->x.computed;
	}
	return 0;
}

gdouble pattern_y (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->y._set)
			return pat_i->y.computed;
	}
	return 0;
}

gdouble pattern_width (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->width._set)
			return pat_i->width.computed;
	}
	return 0;
}

gdouble pattern_height (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->height._set)
			return pat_i->height.computed;
	}
	return 0;
}

NRRect *pattern_viewBox (SPPattern *pat)
{
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i->viewBox_set)
			return &(pat_i->viewBox);
	}
	return &(pat->viewBox);
}

bool pattern_hasItemChildren (SPPattern *pat)
{
	for (SPObject *child = sp_object_first_child(SP_OBJECT(pat)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
		if (SP_IS_ITEM (child)) {
			return true;
		}
	}
	return false;
}



/* Painter */

static void sp_pat_fill (SPPainter *painter, NRPixBlock *pb);

/**
Creates a painter (i.e. the thing that does actual filling at the given zoom).
See (*) below for why the parent_transform may be necessary.
*/
static SPPainter *
sp_pattern_painter_new (SPPaintServer *ps, NR::Matrix const &full_transform, NR::Matrix const &/*parent_transform*/, const NRRect *bbox)
{
	SPPattern *pat = SP_PATTERN (ps);
	SPPatPainter *pp = g_new (SPPatPainter, 1);

	pp->painter.type = SP_PAINTER_IND;
	pp->painter.fill = sp_pat_fill;

	pp->pat = pat;

	if (pattern_patternUnits (pat) == SP_PATTERN_UNITS_OBJECTBOUNDINGBOX) {
		/* BBox to user coordinate system */
		NR::Matrix bbox2user (bbox->x1 - bbox->x0, 0.0, 0.0, bbox->y1 - bbox->y0, bbox->x0, bbox->y0);

		// the final patternTransform, taking into account bbox
		NR::Matrix const ps2user(pattern_patternTransform(pat) * bbox2user);

		// see (*) comment below
		pp->ps2px = ps2user * full_transform;
	} else {
		/* Problem: What to do, if we have mixed lengths and percentages? */
		/* Currently we do ignore percentages at all, but that is not good (lauris) */

		/* fixme: We may try to normalize here too, look at linearGradient (Lauris) */

		// (*) The spec says, "This additional transformation matrix [patternTransform] is
		// post-multiplied to (i.e., inserted to the right of) any previously defined
		// transformations, including the implicit transformation necessary to convert from
		// object bounding box units to user space." To me, this means that the order should be:
		// item_transform * patternTransform * parent_transform
		// However both Batik and Adobe plugin use:
		// patternTransform * item_transform * parent_transform
		// So here I comply with the majority opinion, but leave my interpretation commented out below.
		// (To get item_transform, I subtract parent from full.)

		//pp->ps2px = (full_transform / parent_transform) * pattern_patternTransform(pat) * parent_transform;
		pp->ps2px = pattern_patternTransform(pat) * full_transform;
	}

	pp->px2ps = pp->ps2px.inverse();

	if (pat->viewBox_set) {
		gdouble tmp_x = pattern_width (pat) / (pattern_viewBox(pat)->x1 - pattern_viewBox(pat)->x0);
		gdouble tmp_y = pattern_height (pat) / (pattern_viewBox(pat)->y1 - pattern_viewBox(pat)->y0);

		// FIXME: preserveAspectRatio must be taken into account here too!
		NR::Matrix vb2ps (tmp_x, 0.0, 0.0, tmp_y, pattern_x(pat) - pattern_viewBox(pat)->x0 * tmp_x, pattern_y(pat) - pattern_viewBox(pat)->y0 * tmp_y);

		NR::Matrix vb2us = vb2ps * pattern_patternTransform(pat);

		// see (*)
		pp->pcs2px = vb2us * full_transform;
	} else {
		/* No viewbox, have to parse units */
		if (pattern_patternContentUnits (pat) == SP_PATTERN_UNITS_OBJECTBOUNDINGBOX) {
			/* BBox to user coordinate system */
			NR::Matrix bbox2user (bbox->x1 - bbox->x0, 0.0, 0.0, bbox->y1 - bbox->y0, bbox->x0, bbox->y0);

			NR::Matrix pcs2user = pattern_patternTransform(pat) * bbox2user;

			// see (*)
			pp->pcs2px = pcs2user * full_transform;
		} else {
			// see (*)
			//pcs2px = (full_transform / parent_transform) * pattern_patternTransform(pat) * parent_transform;
			pp->pcs2px = pattern_patternTransform(pat) * full_transform;
		}

		pp->pcs2px = Geom::Translate (pattern_x (pat), pattern_y (pat)) * pp->pcs2px;
	}

	/* Create arena */
	pp->arena = NRArena::create();

	pp->dkey = sp_item_display_key_new (1);

	/* Create group */
	pp->root = NRArenaGroup::create(pp->arena);

	/* Show items */
	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i && SP_IS_OBJECT (pat_i) && pattern_hasItemChildren(pat_i)) { // find the first one with item children
			for (SPObject *child = sp_object_first_child(SP_OBJECT(pat_i)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
				if (SP_IS_ITEM (child)) {
					NRArenaItem *cai;
					cai = sp_item_invoke_show (SP_ITEM (child), pp->arena, pp->dkey, SP_ITEM_REFERENCE_FLAGS);
					nr_arena_item_append_child (pp->root, cai);
				}
			}
			break; // do not go further up the chain if children are found
		}
	}

	{
		NRRect    one_tile,tr_tile;
		one_tile.x0=pattern_x(pp->pat);
		one_tile.y0=pattern_y(pp->pat);
		one_tile.x1=one_tile.x0+pattern_width (pp->pat);
		one_tile.y1=one_tile.y0+pattern_height (pp->pat);
		nr_rect_d_matrix_transform (&tr_tile, &one_tile, &pp->ps2px);
		int       tr_width=(int)ceil(1.3*(tr_tile.x1-tr_tile.x0));
		int       tr_height=(int)ceil(1.3*(tr_tile.y1-tr_tile.y0));
//		if ( tr_width < 10000 && tr_height < 10000 && tr_width*tr_height < 1000000 ) {
		pp->use_cached_tile=false;//true;
			if ( tr_width > 1000 ) tr_width=1000;
			if ( tr_height > 1000 ) tr_height=1000;
			pp->cached_bbox.x0=0;
			pp->cached_bbox.y0=0;
			pp->cached_bbox.x1=tr_width;
			pp->cached_bbox.y1=tr_height;

			if (pp->use_cached_tile) {
				nr_pixblock_setup (&pp->cached_tile,NR_PIXBLOCK_MODE_R8G8B8A8N, pp->cached_bbox.x0, pp->cached_bbox.y0, pp->cached_bbox.x1, pp->cached_bbox.y1,TRUE);
			}

			pp->pa2ca[0]=((double)tr_width)/(one_tile.x1-one_tile.x0);
			pp->pa2ca[1]=0;
			pp->pa2ca[2]=0;
			pp->pa2ca[3]=((double)tr_height)/(one_tile.y1-one_tile.y0);
			pp->pa2ca[4]=-one_tile.x0*pp->pa2ca[0];
			pp->pa2ca[5]=-one_tile.y0*pp->pa2ca[1];
			pp->ca2pa[0]=(one_tile.x1-one_tile.x0)/((double)tr_width);
			pp->ca2pa[1]=0;
			pp->ca2pa[2]=0;
			pp->ca2pa[3]=(one_tile.y1-one_tile.y0)/((double)tr_height);
			pp->ca2pa[4]=one_tile.x0;
			pp->ca2pa[5]=one_tile.y0;
//		} else {
//			pp->use_cached_tile=false;
//		}
	}
	
	NRGC gc(NULL);
	if ( pp->use_cached_tile ) {
		gc.transform=pp->pa2ca;
	} else {
		gc.transform = pp->pcs2px;
	}
	nr_arena_item_invoke_update (pp->root, NULL, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_ALL);
	if ( pp->use_cached_tile ) {
		nr_arena_item_invoke_render (NULL, pp->root, &pp->cached_bbox, &pp->cached_tile, 0);
	} else {
		// nothing to do now
	}
	
	return (SPPainter *) pp;
}

static void
sp_pattern_painter_free (SPPaintServer */*ps*/, SPPainter *painter)
{
	SPPatPainter *pp = (SPPatPainter *) painter;
	SPPattern *pat = pp->pat;

	for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
		if (pat_i && SP_IS_OBJECT (pat_i) && pattern_hasItemChildren(pat_i)) { // find the first one with item children
			for (SPObject *child = sp_object_first_child(SP_OBJECT(pat_i)) ; child != NULL; child = SP_OBJECT_NEXT(child) ) {
				if (SP_IS_ITEM (child)) {
						sp_item_invoke_hide (SP_ITEM (child), pp->dkey);
				}
			}
			break; // do not go further up the chain if children are found
		}
	}
	if ( pp->use_cached_tile ) nr_pixblock_release(&pp->cached_tile);
	g_free (pp);
}

void
get_cached_tile_pixel(SPPatPainter* pp,double x,double y,unsigned char &r,unsigned char &g,unsigned char &b,unsigned char &a)
{
	int    ca_h=(int)floor(x);
	int    ca_v=(int)floor(y);
	int    r_x=(int)floor(16*(x-floor(x)));
	int    r_y=(int)floor(16*(y-floor(y)));
	unsigned int    tl_m=(16-r_x)*(16-r_y);
	unsigned int    bl_m=(16-r_x)*r_y;
	unsigned int    tr_m=r_x*(16-r_y);
	unsigned int    br_m=r_x*r_y;
	int    cb_h=ca_h+1;
	int    cb_v=ca_v+1;
	if ( cb_h >= pp->cached_bbox.x1 ) cb_h=0;
	if ( cb_v >= pp->cached_bbox.y1 ) cb_v=0;
	
	unsigned char* tlx=NR_PIXBLOCK_PX(&pp->cached_tile)+(ca_v*pp->cached_tile.rs)+4*ca_h;
	unsigned char* trx=NR_PIXBLOCK_PX(&pp->cached_tile)+(ca_v*pp->cached_tile.rs)+4*cb_h;
	unsigned char* blx=NR_PIXBLOCK_PX(&pp->cached_tile)+(cb_v*pp->cached_tile.rs)+4*ca_h;
	unsigned char* brx=NR_PIXBLOCK_PX(&pp->cached_tile)+(cb_v*pp->cached_tile.rs)+4*cb_h;
	
	unsigned int tl_c=tlx[0];
	unsigned int tr_c=trx[0];
	unsigned int bl_c=blx[0];
	unsigned int br_c=brx[0];
	unsigned int f_c=(tl_m*tl_c+tr_m*tr_c+bl_m*bl_c+br_m*br_c)>>8;
	r=f_c;
	tl_c=tlx[1];
	tr_c=trx[1];
	bl_c=blx[1];
	br_c=brx[1];
	f_c=(tl_m*tl_c+tr_m*tr_c+bl_m*bl_c+br_m*br_c)>>8;
	g=f_c;
	tl_c=tlx[2];
	tr_c=trx[2];
	bl_c=blx[2];
	br_c=brx[2];
	f_c=(tl_m*tl_c+tr_m*tr_c+bl_m*bl_c+br_m*br_c)>>8;
	b=f_c;
	tl_c=tlx[3];
	tr_c=trx[3];
	bl_c=blx[3];
	br_c=brx[3];
	f_c=(tl_m*tl_c+tr_m*tr_c+bl_m*bl_c+br_m*br_c)>>8;
	a=f_c;
}

static void
sp_pat_fill (SPPainter *painter, NRPixBlock *pb)
{
	SPPatPainter *pp;
	NRRect ba, psa;
	NRRectL area;
	double x, y;

	pp = (SPPatPainter *) painter;

	if (pattern_width (pp->pat) < NR_EPSILON) return;
	if (pattern_height (pp->pat) < NR_EPSILON) return;

	/* Find buffer area in gradient space */
	/* fixme: This is suboptimal (Lauris) */

	if ( pp->use_cached_tile ) {
		double   pat_w=pattern_width (pp->pat);
		double   pat_h=pattern_height (pp->pat);
		if ( pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8N || pb->mode == NR_PIXBLOCK_MODE_R8G8B8A8P ) { // same thing because it's filling an empty pixblock
			unsigned char*  lpx=NR_PIXBLOCK_PX(pb);
			double          px_y=pb->area.y0;
			for (int j=pb->area.y0;j<pb->area.y1;j++) {
				unsigned char* cpx=lpx;
				double         px_x = pb->area.x0;
				
				double ps_x=pp->px2ps[0]*px_x+pp->px2ps[2]*px_y+pp->px2ps[4];
				double ps_y=pp->px2ps[1]*px_x+pp->px2ps[3]*px_y+pp->px2ps[5];
				for (int i=pb->area.x0;i<pb->area.x1;i++) {
					while ( ps_x > pat_w ) ps_x-=pat_w;
					while ( ps_x < 0 ) ps_x+=pat_w;
					while ( ps_y > pat_h ) ps_y-=pat_h;
					while ( ps_y < 0 ) ps_y+=pat_h;
					double ca_x=pp->pa2ca[0]*ps_x+pp->pa2ca[2]*ps_y+pp->pa2ca[4];
					double ca_y=pp->pa2ca[1]*ps_x+pp->pa2ca[3]*ps_y+pp->pa2ca[5];
					unsigned char n_a,n_r,n_g,n_b;
					get_cached_tile_pixel(pp,ca_x,ca_y,n_r,n_g,n_b,n_a);
					cpx[0]=n_r;
					cpx[1]=n_g;
					cpx[2]=n_b;
					cpx[3]=n_a;
					
					px_x+=1.0;
					ps_x+=pp->px2ps[0];
					ps_y+=pp->px2ps[1];
					cpx+=4;
				}
				px_y+=1.0;
				lpx+=pb->rs;
			}
		} else if ( pb->mode == NR_PIXBLOCK_MODE_R8G8B8 ) {
			unsigned char*  lpx=NR_PIXBLOCK_PX(pb);
			double          px_y=pb->area.y0;
			for (int j=pb->area.y0;j<pb->area.y1;j++) {
				unsigned char* cpx=lpx;
				double         px_x = pb->area.x0;
				
				double ps_x=pp->px2ps[0]*px_x+pp->px2ps[2]*px_y+pp->px2ps[4];
				double ps_y=pp->px2ps[1]*px_x+pp->px2ps[3]*px_y+pp->px2ps[5];
				for (int i=pb->area.x0;i<pb->area.x1;i++) {
					while ( ps_x > pat_w ) ps_x-=pat_w;
					while ( ps_x < 0 ) ps_x+=pat_w;
					while ( ps_y > pat_h ) ps_y-=pat_h;
					while ( ps_y < 0 ) ps_y+=pat_h;
					double ca_x=pp->pa2ca[0]*ps_x+pp->pa2ca[2]*ps_y+pp->pa2ca[4];
					double ca_y=pp->pa2ca[1]*ps_x+pp->pa2ca[3]*ps_y+pp->pa2ca[5];
					unsigned char n_a,n_r,n_g,n_b;
					get_cached_tile_pixel(pp,ca_x,ca_y,n_r,n_g,n_b,n_a);
					cpx[0]=n_r;
					cpx[1]=n_g;
					cpx[2]=n_b;
					
					px_x+=1.0;
					ps_x+=pp->px2ps[0];
					ps_y+=pp->px2ps[1];
					cpx+=4;
				}
				px_y+=1.0;
				lpx+=pb->rs;
			}
		}
	} else {
		ba.x0 = pb->area.x0;
		ba.y0 = pb->area.y0;
		ba.x1 = pb->area.x1;
		ba.y1 = pb->area.y1;
		
        // Trying to solve this bug: https://bugs.launchpad.net/inkscape/+bug/167416
        // Bail out if the transformation matrix has extreme values. If we bail out
        // however, then something (which was meaningless anyway) won't be rendered, 
        // which is better than getting stuck in a virtually infinite loop
        if (fabs(pp->px2ps[0]) < 1e6 && 
            fabs(pp->px2ps[3]) < 1e6 &&
            fabs(pp->px2ps[4]) < 1e6 &&
            fabs(pp->px2ps[5]) < 1e6) 
        {
            nr_rect_d_matrix_transform (&psa, &ba, &pp->px2ps);
    		
    		psa.x0 = floor ((psa.x0 - pattern_x (pp->pat)) / pattern_width (pp->pat)) -1;
    		psa.y0 = floor ((psa.y0 - pattern_y (pp->pat)) / pattern_height (pp->pat)) -1;
    		psa.x1 = ceil ((psa.x1 - pattern_x (pp->pat)) / pattern_width (pp->pat)) +1;
    		psa.y1 = ceil ((psa.y1 - pattern_y (pp->pat)) / pattern_height (pp->pat)) +1;
    		
            // If psa is too wide or tall, then something must be wrong! This is due to
            // nr_rect_d_matrix_transform (&psa, &ba, &pp->px2ps) using a weird transformation matrix pp->px2ps.
            g_assert(std::abs(psa.x1 - psa.x0) < 1e6);
            g_assert(std::abs(psa.y1 - psa.y0) < 1e6);
            
            for (y = psa.y0; y < psa.y1; y++) {
    			for (x = psa.x0; x < psa.x1; x++) {
    				NRPixBlock ppb;
    				double psx, psy;
    				
    				psx = x * pattern_width (pp->pat);
    				psy = y * pattern_height (pp->pat);
    				
    				area.x0 = (gint32)(pb->area.x0 - (pp->ps2px[0] * psx + pp->ps2px[2] * psy));
    				area.y0 = (gint32)(pb->area.y0 - (pp->ps2px[1] * psx + pp->ps2px[3] * psy));
    				area.x1 = area.x0 + pb->area.x1 - pb->area.x0;
    				area.y1 = area.y0 + pb->area.y1 - pb->area.y0;
    				
    				// We do not update here anymore
    
    				// Set up buffer
    				// fixme: (Lauris)
    				nr_pixblock_setup_extern (&ppb, pb->mode, area.x0, area.y0, area.x1, area.y1, NR_PIXBLOCK_PX (pb), pb->rs, FALSE, FALSE);
    				
    				nr_arena_item_invoke_render (NULL, pp->root, &area, &ppb, 0);
    				
    				nr_pixblock_release (&ppb);
    			}
    		}
        } 
	}
}

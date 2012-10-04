/*
 * SVG <pattern> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma 
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
#include <2geom/transforms.h>
#include "macros.h"
#include "svg/svg.h"
#include "display/cairo-utils.h"
#include "display/drawing-context.h"
#include "display/drawing-surface.h"
#include "display/drawing.h"
#include "display/drawing-group.h"
#include "attributes.h"
#include "document-private.h"
#include "uri.h"
#include "style.h"
#include "sp-pattern.h"
#include "xml/repr.h"
#include "display/grayscale.h"

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

/*
 * Pattern
 */

static void sp_pattern_class_init (SPPatternClass *klass);
static void sp_pattern_init (SPPattern *gr);

static void sp_pattern_build (SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_pattern_release (SPObject *object);
static void sp_pattern_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_pattern_update (SPObject *object, SPCtx *ctx, unsigned int flags);
static void sp_pattern_modified (SPObject *object, unsigned int flags);

static void pattern_ref_changed(SPObject *old_ref, SPObject *ref, SPPattern *pat);
static void pattern_ref_modified (SPObject *ref, guint flags, SPPattern *pattern);

static cairo_pattern_t *sp_pattern_create_pattern(SPPaintServer *ps, cairo_t *ct, Geom::OptRect const &bbox, double opacity);

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
	sp_object_class->update = sp_pattern_update;
	sp_object_class->modified = sp_pattern_modified;

	// do we need _write? seems to work without it

    ps_class->pattern_new = sp_pattern_create_pattern;
}

static void
sp_pattern_init (SPPattern *pat)
{
	pat->ref = new SPPatternReference(pat);
	pat->ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(pattern_ref_changed), pat));

	pat->patternUnits = SP_PATTERN_UNITS_OBJECTBOUNDINGBOX;
	pat->patternUnits_set = FALSE;

	pat->patternContentUnits = SP_PATTERN_UNITS_USERSPACEONUSE;
	pat->patternContentUnits_set = FALSE;

	pat->patternTransform = Geom::identity();
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

	object->readAttr( "patternUnits" );
	object->readAttr( "patternContentUnits" );
	object->readAttr( "patternTransform" );
	object->readAttr( "x" );
	object->readAttr( "y" );
	object->readAttr( "width" );
	object->readAttr( "height" );
	object->readAttr( "viewBox" );
	object->readAttr( "xlink:href" );

	/* Register ourselves */
	document->addResource("pattern", object);
}

static void sp_pattern_release(SPObject *object)
{
    SPPattern *pat = reinterpret_cast<SPPattern *>(object);

    if (object->document) {
        // Unregister ourselves
        object->document->removeResource("pattern", object);
    }

    if (pat->ref) {
        pat->modified_connection.disconnect();
        pat->ref->detach();
        delete pat->ref;
        pat->ref = NULL;
    }

    pat->modified_connection.~connection();

    if (((SPObjectClass *) pattern_parent_class)->release) {
        ((SPObjectClass *) pattern_parent_class)->release (object);
    }
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
		Geom::Affine t;
		if (value && sp_svg_transform_read (value, &t)) {
			pat->patternTransform = t;
			pat->patternTransform_set = TRUE;
		} else {
			pat->patternTransform = Geom::identity();
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
			    pat->viewBox = Geom::Rect::from_xywh(x, y, width, height);
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

/* TODO: do we need a ::remove_child handler? */

/* fixme: We need ::order_changed handler too (Lauris) */

static GSList *pattern_getchildren(SPPattern *pat)
{
    GSList *l = NULL;

    for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->firstChild()) { // find the first one with children
	    for (SPObject *child = pat->firstChild() ; child ; child = child->getNext() ) {
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
static void pattern_ref_modified (SPObject */*ref*/, guint /*flags*/, SPPattern *pattern)
{
    if ( SP_IS_OBJECT(pattern) ) {
        pattern->requestModified(SP_OBJECT_MODIFIED_FLAG);
    }
    // Conditional to avoid causing infinite loop if there's a cycle in the href chain.
}


/**
Count how many times pat is used by the styles of o and its descendants
*/
static guint
count_pattern_hrefs(SPObject *o, SPPattern *pat)
{
    if (!o)
        return 1;

    guint i = 0;

    SPStyle *style = o->style;
    if (style
        && style->fill.isPaintserver()
        && SP_IS_PATTERN(SP_STYLE_FILL_SERVER(style))
        && SP_PATTERN(SP_STYLE_FILL_SERVER(style)) == pat)
    {
        i ++;
    }
    if (style
        && style->stroke.isPaintserver()
        && SP_IS_PATTERN(SP_STYLE_STROKE_SERVER(style))
        && SP_PATTERN(SP_STYLE_STROKE_SERVER(style)) == pat)
    {
        i ++;
    }

    for ( SPObject *child = o->firstChild(); child != NULL; child = child->next ) {
        i += count_pattern_hrefs(child, pat);
    }

    return i;
}

SPPattern *pattern_chain(SPPattern *pattern)
{
	SPDocument *document = pattern->document;
        Inkscape::XML::Document *xml_doc = document->getReprDoc();
	Inkscape::XML::Node *defsrepr = document->getDefs()->getRepr();

	Inkscape::XML::Node *repr = xml_doc->createElement("svg:pattern");
	repr->setAttribute("inkscape:collect", "always");
	gchar *parent_ref = g_strconcat("#", pattern->getRepr()->attribute("id"), NULL);
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
	if (!pattern->href || pattern->hrefcount > count_pattern_hrefs(item, pattern)) {
		pattern = pattern_chain (pattern);
		gchar *href = g_strconcat("url(#", pattern->getRepr()->attribute("id"), ")", NULL);

		SPCSSAttr *css = sp_repr_css_attr_new ();
		sp_repr_css_set_property (css, property, href);
		sp_repr_css_change_recursive(item->getRepr(), css, "style");
	}
	return pattern;
}

void
sp_pattern_transform_multiply (SPPattern *pattern, Geom::Affine postmul, bool set)
{
	// this formula is for a different interpretation of pattern transforms as described in (*) in sp-pattern.cpp
	// for it to work, we also need    sp_object_read_attr( item, "transform");
	//pattern->patternTransform = premul * item->transform * pattern->patternTransform * item->transform.inverse() * postmul;

	// otherwise the formula is much simpler
	if (set) {
		pattern->patternTransform = postmul;
	} else {
		pattern->patternTransform = pattern_patternTransform(pattern) * postmul;
	}
	pattern->patternTransform_set = TRUE;

	gchar *c=sp_svg_transform_write(pattern->patternTransform);
	pattern->getRepr()->setAttribute("patternTransform", c);
	g_free(c);
}

const gchar *pattern_tile(GSList *reprs, Geom::Rect bounds, SPDocument *document, Geom::Affine transform, Geom::Affine move)
{
    Inkscape::XML::Document *xml_doc = document->getReprDoc();
    Inkscape::XML::Node *defsrepr = document->getDefs()->getRepr();

	Inkscape::XML::Node *repr = xml_doc->createElement("svg:pattern");
	repr->setAttribute("patternUnits", "userSpaceOnUse");
	sp_repr_set_svg_double(repr, "width", bounds.dimensions()[Geom::X]);
	sp_repr_set_svg_double(repr, "height", bounds.dimensions()[Geom::Y]);

	gchar *t=sp_svg_transform_write(transform);
	repr->setAttribute("patternTransform", t);
	g_free(t);

	defsrepr->appendChild(repr);
	const gchar *pat_id = repr->attribute("id");
	SPObject *pat_object = document->getObjectById(pat_id);

	for (GSList *i = reprs; i != NULL; i = i->next) {
	        Inkscape::XML::Node *node = (Inkscape::XML::Node *)(i->data);
		SPItem *copy = SP_ITEM(pat_object->appendChildRepr(node));

		Geom::Affine dup_transform;
		if (!sp_svg_transform_read (node->attribute("transform"), &dup_transform))
			dup_transform = Geom::identity();
		dup_transform *= move;

        copy->doWriteTransform(copy->getRepr(), dup_transform, NULL, false);
    }

	Inkscape::GC::release(repr);
	return pat_id;
}

SPPattern *pattern_getroot(SPPattern *pat)
{
    for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if ( pat_i->firstChild() ) { // find the first one with children
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

Geom::Affine const &pattern_patternTransform(SPPattern const *pat)
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

Geom::OptRect pattern_viewBox (SPPattern *pat)
{
    Geom::OptRect viewbox;
    for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->viewBox_set) {
            viewbox = pat_i->viewBox;
            break;
        }
    }
    return viewbox;
}

static bool pattern_hasItemChildren (SPPattern *pat)
{
    bool hasChildren = false;
    for (SPObject *child = pat->firstChild() ; child && !hasChildren ; child = child->getNext() ) {
        if (SP_IS_ITEM(child)) {
            hasChildren = true;
        }
    }
    return hasChildren;
}

static cairo_pattern_t *
sp_pattern_create_pattern(SPPaintServer *ps,
                          cairo_t *base_ct,
                          Geom::OptRect const &bbox,
                          double opacity)
{
    SPPattern *pat = SP_PATTERN (ps);
    Geom::Affine ps2user;
    Geom::Affine vb2ps = Geom::identity();
    bool needs_opacity = (1.0 - opacity) >= 1e-3;
    bool visible = opacity >= 1e-3;

    if (!visible)
        return NULL;

    /* Show items */
    SPPattern *shown = NULL;
    for (SPPattern *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        // find the first one with item children
        if (pat_i && SP_IS_OBJECT (pat_i) && pattern_hasItemChildren(pat_i)) {
            shown = pat_i;
            break; // do not go further up the chain if children are found
        }
    }

    if (!shown) {
        return cairo_pattern_create_rgba(0,0,0,0);
    }

    /* Create drawing for rendering */
    Inkscape::Drawing drawing;
    unsigned int dkey = SPItem::display_key_new (1);
    Inkscape::DrawingGroup *root = new Inkscape::DrawingGroup(drawing);
    drawing.setRoot(root);

    for (SPObject *child = shown->firstChild(); child != NULL; child = child->getNext() ) {
        if (SP_IS_ITEM (child)) {
            // for each item in pattern, show it on our drawing, add to the group,
            // and connect to the release signal in case the item gets deleted
            Inkscape::DrawingItem *cai;
            cai = SP_ITEM(child)->invoke_show (drawing, dkey, SP_ITEM_SHOW_DISPLAY);
            root->appendChild(cai);
        }
    }

    if (pat->viewBox_set) {
        Geom::Rect vb = *pattern_viewBox(pat);
        gdouble tmp_x = pattern_width (pat) / vb.width();
        gdouble tmp_y = pattern_height (pat) / vb.height();

        // FIXME: preserveAspectRatio must be taken into account here too!
        vb2ps = Geom::Affine(tmp_x, 0.0, 0.0, tmp_y, pattern_x(pat) - vb.left() * tmp_x, pattern_y(pat) - vb.top() * tmp_y);
    }

    ps2user = pattern_patternTransform(pat);
    if (!pat->viewBox_set && pattern_patternContentUnits (pat) == SP_PATTERN_UNITS_OBJECTBOUNDINGBOX) {
        /* BBox to user coordinate system */
        Geom::Affine bbox2user (bbox->width(), 0.0, 0.0, bbox->height(), bbox->left(), bbox->top());
        ps2user *= bbox2user;
    }
    ps2user = Geom::Translate (pattern_x (pat), pattern_y (pat)) * ps2user;

    Geom::Rect pattern_tile = Geom::Rect::from_xywh(pattern_x(pat), pattern_y(pat),
        pattern_width(pat), pattern_height(pat));

    if (pattern_patternUnits(pat) == SP_PATTERN_UNITS_OBJECTBOUNDINGBOX) {
        // interpret x, y, width, height in relation to bbox
        Geom::Affine bbox2user(bbox->width(), 0.0, 0.0, bbox->height(), bbox->left(), bbox->top());
        pattern_tile = pattern_tile * bbox2user;
    }

    cairo_matrix_t cm;
    cairo_get_matrix(base_ct, &cm);
    Geom::Affine full(cm.xx, cm.yx, cm.xy, cm.yy, 0, 0);

    // oversample the pattern slightly
    // TODO: find optimum value
    // TODO: this is lame. instead of using descrim(), we should extract
    //       the scaling component from the complete matrix and use it
    //       to find the optimum tile size for rendering
    Geom::Point c(pattern_tile.dimensions()*vb2ps.descrim()*ps2user.descrim()*full.descrim()*1.1);
    c[Geom::X] = ceil(c[Geom::X]);
    c[Geom::Y] = ceil(c[Geom::Y]);
    
    Geom::IntRect one_tile = pattern_tile.roundOutwards();
    Inkscape::DrawingSurface temp(pattern_tile, c.ceil());
    Inkscape::DrawingContext ct(temp);

    // render pattern.
    if (needs_opacity) {
        ct.pushGroup(); // this group is for pattern + opacity
    }

    // TODO: make sure there are no leaks.
    Inkscape::UpdateContext ctx;
    ctx.ctm = vb2ps;
    drawing.update(Geom::IntRect::infinite(), ctx);
    drawing.render(ct, one_tile);
    for (SPObject *child = shown->firstChild() ; child != NULL; child = child->getNext() ) {
        if (SP_IS_ITEM (child)) {
            SP_ITEM(child)->invoke_hide(dkey);
        }
    }

    if (needs_opacity) {
        ct.popGroupToSource(); // pop raw pattern
        ct.paint(opacity); // apply opacity
    }

    cairo_pattern_t *cp = cairo_pattern_create_for_surface(temp.raw());

    // Apply transformation to user space. Also compensate for oversampling.
    ink_cairo_pattern_set_matrix(cp, ps2user.inverse() * temp.drawingTransform());
    cairo_pattern_set_extend(cp, CAIRO_EXTEND_REPEAT);

    return cp;
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

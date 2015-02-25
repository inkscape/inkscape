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
#include <glibmm.h>
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
static void pattern_ref_changed(SPObject *old_ref, SPObject *ref, SPPattern *pat);
static void pattern_ref_modified (SPObject *ref, guint flags, SPPattern *pattern);

SPPattern::SPPattern() : SPPaintServer(), SPViewBox() {
	this->href = NULL;

	this->ref = new SPPatternReference(this);
	this->ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(pattern_ref_changed), this));

	this->patternUnits = SP_PATTERN_UNITS_OBJECTBOUNDINGBOX;
	this->patternUnits_set = FALSE;

	this->patternContentUnits = SP_PATTERN_UNITS_USERSPACEONUSE;
	this->patternContentUnits_set = FALSE;

	this->patternTransform = Geom::identity();
	this->patternTransform_set = FALSE;

	this->x.unset();
	this->y.unset();
	this->width.unset();
	this->height.unset();
}

SPPattern::~SPPattern() {
}

void SPPattern::build(SPDocument* doc, Inkscape::XML::Node* repr) {
	SPPaintServer::build(doc, repr);

	this->readAttr( "patternUnits" );
	this->readAttr( "patternContentUnits" );
	this->readAttr( "patternTransform" );
	this->readAttr( "x" );
	this->readAttr( "y" );
	this->readAttr( "width" );
	this->readAttr( "height" );
	this->readAttr( "viewBox" );
        this->readAttr( "preserveAspectRatio" );
	this->readAttr( "xlink:href" );

	/* Register ourselves */
	doc->addResource("pattern", this);
}

void SPPattern::release() {
    if (this->document) {
        // Unregister ourselves
        this->document->removeResource("pattern", this);
    }

    if (this->ref) {
        this->modified_connection.disconnect();
        this->ref->detach();
        delete this->ref;
        this->ref = NULL;
    }

    SPPaintServer::release();
}

void SPPattern::set(unsigned int key, const gchar* value) {
	switch (key) {
	case SP_ATTR_PATTERNUNITS:
		if (value) {
			if (!strcmp (value, "userSpaceOnUse")) {
				this->patternUnits = SP_PATTERN_UNITS_USERSPACEONUSE;
			} else {
				this->patternUnits = SP_PATTERN_UNITS_OBJECTBOUNDINGBOX;
			}

			this->patternUnits_set = TRUE;
		} else {
			this->patternUnits_set = FALSE;
		}

		this->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;

	case SP_ATTR_PATTERNCONTENTUNITS:
		if (value) {
			if (!strcmp (value, "userSpaceOnUse")) {
				this->patternContentUnits = SP_PATTERN_UNITS_USERSPACEONUSE;
			} else {
				this->patternContentUnits = SP_PATTERN_UNITS_OBJECTBOUNDINGBOX;
			}

			this->patternContentUnits_set = TRUE;
		} else {
			this->patternContentUnits_set = FALSE;
		}

		this->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;

	case SP_ATTR_PATTERNTRANSFORM: {
		Geom::Affine t;

		if (value && sp_svg_transform_read (value, &t)) {
			this->patternTransform = t;
			this->patternTransform_set = TRUE;
		} else {
			this->patternTransform = Geom::identity();
			this->patternTransform_set = FALSE;
		}

		this->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;
	}
	case SP_ATTR_X:
	    this->x.readOrUnset(value);
		this->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;

	case SP_ATTR_Y:
	    this->y.readOrUnset(value);
		this->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;

	case SP_ATTR_WIDTH:
	    this->width.readOrUnset(value);
		this->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;

	case SP_ATTR_HEIGHT:
	    this->height.readOrUnset(value);
		this->requestModified(SP_OBJECT_MODIFIED_FLAG);
		break;

	case SP_ATTR_VIEWBOX:
            set_viewBox( value );
            this->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
            break;

        case SP_ATTR_PRESERVEASPECTRATIO:
            set_preserveAspectRatio( value );
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
            break;

	case SP_ATTR_XLINK_HREF:
		if ( value && this->href && ( strcmp(value, this->href) == 0 ) ) {
			/* Href unchanged, do nothing. */
		} else {
			g_free(this->href);
			this->href = NULL;

			if (value) {
				// First, set the href field; it's only used in the "unchanged" check above.
				this->href = g_strdup(value);
				// Now do the attaching, which emits the changed signal.
				if (value) {
					try {
						this->ref->attach(Inkscape::URI(value));
					} catch (Inkscape::BadURIException &e) {
						g_warning("%s", e.what());
						this->ref->detach();
					}
				} else {
					this->ref->detach();
				}
			}
		}
		break;

	default:
		SPPaintServer::set(key, value);
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

void SPPattern::update(SPCtx* ctx, unsigned int flags) {
	if (flags & SP_OBJECT_MODIFIED_FLAG) {
		flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	}

	flags &= SP_OBJECT_MODIFIED_CASCADE;

	GSList *l = pattern_getchildren (this);
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

void SPPattern::modified(unsigned int flags) {
	if (flags & SP_OBJECT_MODIFIED_FLAG) {
		flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	}

	flags &= SP_OBJECT_MODIFIED_CASCADE;

	GSList *l = pattern_getchildren (this);
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

guint pattern_patternUnits (SPPattern const *pat)
{
    for (SPPattern const *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->patternUnits_set)
            return pat_i->patternUnits;
    }
    return pat->patternUnits;
}

guint pattern_patternContentUnits (SPPattern const *pat)
{
    for (SPPattern const *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
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

gdouble pattern_x (SPPattern const *pat)
{
    for (SPPattern const *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->x._set)
            return pat_i->x.computed;
    }
    return 0;
}

gdouble pattern_y (SPPattern const *pat)
{
    for (SPPattern const *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->y._set)
            return pat_i->y.computed;
    }
    return 0;
}

gdouble pattern_width (SPPattern const* pat)
{
    for (SPPattern const *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->width._set)
            return pat_i->width.computed;
    }
    return 0;
}

gdouble pattern_height (SPPattern const *pat)
{
    for (SPPattern const *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->height._set)
            return pat_i->height.computed;
    }
    return 0;
}

Geom::OptRect pattern_viewBox (SPPattern const *pat)
{
    Geom::OptRect viewbox;
    for (SPPattern const *pat_i = pat; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->viewBox_set) {
            viewbox = pat_i->viewBox;
            break;
        }
    }
    return viewbox;
}

static bool pattern_hasItemChildren (SPPattern const *pat)
{
    bool hasChildren = false;
    for (SPObject const *child = pat->firstChild() ; child && !hasChildren ; child = child->getNext() ) {
        if (SP_IS_ITEM(child)) {
            hasChildren = true;
        }
    }
    return hasChildren;
}

bool SPPattern::isValid() const
{
	double tile_width = pattern_width(this);
	double tile_height = pattern_height(this);

	if (tile_width <= 0 || tile_height <= 0)
		return false;
	return true;
}

cairo_pattern_t* SPPattern::pattern_new(cairo_t *base_ct, Geom::OptRect const &bbox, double opacity) {

    bool needs_opacity = (1.0 - opacity) >= 1e-3;
    bool visible = opacity >= 1e-3;

    if (!visible) {
        return NULL;
    }

    /* Show items */
    SPPattern *shown = NULL;

    for (SPPattern *pat_i = this; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        // find the first one with item children
        if (pat_i && SP_IS_OBJECT(pat_i) && pattern_hasItemChildren(pat_i)) {
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

    //                 ****** Geometry ******
    //
    // * "width" and "height" determine tile size.
    // * "viewBox" (if defined) or "patternContentUnits" determines placement of content inside
    //   tile.
    // * "x", "y", and "patternTransform" transform tile to user space after tile is generated.

    // These functions recursively search up the tree to find the values.
    double tile_x      = pattern_x(this);
    double tile_y      = pattern_y(this);
    double tile_width  = pattern_width(this);
    double tile_height = pattern_height(this);
    if ( bbox && (pattern_patternUnits(this) == SP_PATTERN_UNITS_OBJECTBOUNDINGBOX) ) {
        tile_x      *= bbox->width();
        tile_y      *= bbox->height();
        tile_width  *= bbox->width();
        tile_height *= bbox->height();
    }

    // Pattern size in pattern space
    Geom::Rect pattern_tile = Geom::Rect::from_xywh(0, 0, tile_width, tile_height);
 
    // Content to tile (pattern space)
    Geom::Affine content2ps;
    Geom::OptRect effective_view_box = pattern_viewBox(this);
    if (effective_view_box) {
        // viewBox to pattern server (using SPViewBox) 
        viewBox = *effective_view_box;
        c2p.setIdentity();
        apply_viewbox( pattern_tile );
        content2ps = c2p;
    } else {

        // Content to bbox
        if (bbox && (pattern_patternContentUnits(this) == SP_PATTERN_UNITS_OBJECTBOUNDINGBOX) ) {
            content2ps = Geom::Affine(bbox->width(), 0.0, 0.0, bbox->height(), 0,0);
        }
    }


    // Tile (pattern space) to user.
    Geom::Affine ps2user = Geom::Translate(tile_x,tile_y) * pattern_patternTransform(this);


    // Transform of object with pattern (includes screen scaling)
    cairo_matrix_t cm;
    cairo_get_matrix(base_ct, &cm);
    Geom::Affine full(cm.xx, cm.yx, cm.xy, cm.yy, 0, 0);

    // The DrawingSurface class handles the mapping from "logical space"
    // (coordinates in the rendering) to "physical space" (surface pixels).
    // An oversampling is done as the pattern may not pixel align with the final surface.
    // The cairo surface is created when the DrawingContext is declared.

    // Oversample the pattern
    // TODO: find optimum value
    // TODO: this is lame. instead of using descrim(), we should extract
    //       the scaling component from the complete matrix and use it
    //       to find the optimum tile size for rendering
    // c is number of pixels in buffer x and y.
    // Scale factor of 1.1 is too small... see bug #1251039
    Geom::Point c(pattern_tile.dimensions()*ps2user.descrim()*full.descrim()*2.0);

    // Create drawing surface with size of pattern tile (in pattern space) but with number of pixels
    // based on required resolution (c).
    Inkscape::DrawingSurface pattern_surface(pattern_tile, c.ceil());
    Inkscape::DrawingContext dc(pattern_surface);

    pattern_tile *= pattern_surface.drawingTransform();
    Geom::IntRect one_tile = pattern_tile.roundOutwards();

    // Render pattern.
    if (needs_opacity) {
        dc.pushGroup(); // this group is for pattern + opacity
    }

    // TODO: make sure there are no leaks.
    Inkscape::UpdateContext ctx;  // UpdateContext is structure with only ctm!
    ctx.ctm = content2ps * pattern_surface.drawingTransform();
    dc.transform( pattern_surface.drawingTransform().inverse() );
    drawing.update(Geom::IntRect::infinite(), ctx);

    // Render drawing to pattern_surface via drawing context, this calls root->render
    // which is really DrawingItem->render().
    drawing.render(dc, one_tile);
    for (SPObject *child = shown->firstChild() ; child != NULL; child = child->getNext() ) {
        if (SP_IS_ITEM (child)) {
            SP_ITEM(child)->invoke_hide(dkey);
        }
    }

    // Uncomment to debug
    // cairo_surface_t* raw = pattern_surface.raw();
    // std::cout << "  cairo_surface (sp-pattern): "
    //           << " width: "  << cairo_image_surface_get_width( raw )
    //           << " height: " << cairo_image_surface_get_height( raw )
    //           << std::endl;
    // std::string filename = "sp-pattern-" + (std::string)getId() + ".png";
    // cairo_surface_write_to_png( pattern_surface.raw(), filename.c_str() );

    if (needs_opacity) {
        dc.popGroupToSource(); // pop raw pattern
        dc.paint(opacity); // apply opacity
    }

    cairo_pattern_t *cp = cairo_pattern_create_for_surface(pattern_surface.raw());
    // Apply transformation to user space. Also compensate for oversampling.
    ink_cairo_pattern_set_matrix(cp, ps2user.inverse() * pattern_surface.drawingTransform() );

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :

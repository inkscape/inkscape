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
#include "config.h"
#endif

#include <cstring>
#include <string>
#include <glibmm.h>
#include <2geom/transforms.h>
#include <sigc++/functors/mem_fun.h>

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

#include "sp-factory.h"

SPPattern::SPPattern()
    : SPPaintServer()
    , SPViewBox()
{
    this->ref = new SPPatternReference(this);
    this->ref->changedSignal().connect(sigc::mem_fun(this, &SPPattern::_onRefChanged));

    this->_pattern_units = UNITS_OBJECTBOUNDINGBOX;
    this->_pattern_units_set = false;

    this->_pattern_content_units = UNITS_USERSPACEONUSE;
    this->_pattern_content_units_set = false;

    this->_pattern_transform = Geom::identity();
    this->_pattern_transform_set = false;

    this->_x.unset();
    this->_y.unset();
    this->_width.unset();
    this->_height.unset();
}

SPPattern::~SPPattern() {}

void SPPattern::build(SPDocument *doc, Inkscape::XML::Node *repr)
{
    SPPaintServer::build(doc, repr);

    this->readAttr("patternUnits");
    this->readAttr("patternContentUnits");
    this->readAttr("patternTransform");
    this->readAttr("x");
    this->readAttr("y");
    this->readAttr("width");
    this->readAttr("height");
    this->readAttr("viewBox");
    this->readAttr("preserveAspectRatio");
    this->readAttr("xlink:href");

    /* Register ourselves */
    doc->addResource("pattern", this);
}

void SPPattern::release()
{
    if (this->document) {
        // Unregister ourselves
        this->document->removeResource("pattern", this);
    }

    if (this->ref) {
        this->_modified_connection.disconnect();
        this->ref->detach();
        delete this->ref;
        this->ref = NULL;
    }

    SPPaintServer::release();
}

void SPPattern::set(unsigned int key, const gchar *value)
{
    switch (key) {
        case SP_ATTR_PATTERNUNITS:
            if (value) {
                if (!strcmp(value, "userSpaceOnUse")) {
                    this->_pattern_units = UNITS_USERSPACEONUSE;
                }
                else {
                    this->_pattern_units = UNITS_OBJECTBOUNDINGBOX;
                }

                this->_pattern_units_set = true;
            }
            else {
                this->_pattern_units_set = false;
            }

            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_PATTERNCONTENTUNITS:
            if (value) {
                if (!strcmp(value, "userSpaceOnUse")) {
                    this->_pattern_content_units = UNITS_USERSPACEONUSE;
                }
                else {
                    this->_pattern_content_units = UNITS_OBJECTBOUNDINGBOX;
                }

                this->_pattern_content_units_set = true;
            }
            else {
                this->_pattern_content_units_set = false;
            }

            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_PATTERNTRANSFORM: {
            Geom::Affine t;

            if (value && sp_svg_transform_read(value, &t)) {
                this->_pattern_transform = t;
                this->_pattern_transform_set = true;
            }
            else {
                this->_pattern_transform = Geom::identity();
                this->_pattern_transform_set = false;
            }

            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_X:
            this->_x.readOrUnset(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_Y:
            this->_y.readOrUnset(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_WIDTH:
            this->_width.readOrUnset(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_HEIGHT:
            this->_height.readOrUnset(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_VIEWBOX:
            set_viewBox(value);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
            break;

        case SP_ATTR_PRESERVEASPECTRATIO:
            set_preserveAspectRatio(value);
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
            break;

        case SP_ATTR_XLINK_HREF:
            if (value && this->href == value) {
                /* Href unchanged, do nothing. */
            }
            else {
                this->href.clear();

                if (value) {
                    // First, set the href field; it's only used in the "unchanged" check above.
                    this->href = value;
                    // Now do the attaching, which emits the changed signal.
                    if (value) {
                        try {
                            this->ref->attach(Inkscape::URI(value));
                        }
                        catch (Inkscape::BadURIException &e) {
                            g_warning("%s", e.what());
                            this->ref->detach();
                        }
                    }
                    else {
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

void SPPattern::_getChildren(std::list<SPObject *> &l)
{
    for (SPPattern *pat_i = this; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->firstChild()) { // find the first one with children
            for (SPObject *child = pat_i->firstChild(); child; child = child->getNext()) {
                l.push_back(child);
            }
            break; // do not go further up the chain if children are found
        }
    }
}

void SPPattern::update(SPCtx *ctx, unsigned int flags)
{
    typedef std::list<SPObject *>::iterator SPObjectIterator;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    std::list<SPObject *> l;
    _getChildren(l);

    for (SPObjectIterator it = l.begin(); it != l.end(); ++it) {
        SPObject *child = *it;

        sp_object_ref(child, NULL);

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->updateDisplay(ctx, flags);
        }

        sp_object_unref(child, NULL);
    }
}

void SPPattern::modified(unsigned int flags)
{
    typedef std::list<SPObject *>::iterator SPObjectIterator;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    std::list<SPObject *> l;
    _getChildren(l);

    for (SPObjectIterator it = l.begin(); it != l.end(); ++it) {
        SPObject *child = *it;

        sp_object_ref(child, NULL);

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }

        sp_object_unref(child, NULL);
    }
}

void SPPattern::_onRefChanged(SPObject *old_ref, SPObject *ref)
{
    if (old_ref) {
        _modified_connection.disconnect();
    }

    if (SP_IS_PATTERN(ref)) {
        _modified_connection = ref->connectModified(sigc::mem_fun(this, &SPPattern::_onRefModified));
    }

    _onRefModified(ref, 0);
}

void SPPattern::_onRefModified(SPObject * /*ref*/, guint /*flags*/)
{
    requestModified(SP_OBJECT_MODIFIED_FLAG);
    // Conditional to avoid causing infinite loop if there's a cycle in the href chain.
}

guint SPPattern::_countHrefs(SPObject *o) const
{
    if (!o)
        return 1;

    guint i = 0;

    SPStyle *style = o->style;
    if (style && style->fill.isPaintserver() && SP_IS_PATTERN(SP_STYLE_FILL_SERVER(style)) &&
        SP_PATTERN(SP_STYLE_FILL_SERVER(style)) == this) {
        i++;
    }
    if (style && style->stroke.isPaintserver() && SP_IS_PATTERN(SP_STYLE_STROKE_SERVER(style)) &&
        SP_PATTERN(SP_STYLE_STROKE_SERVER(style)) == this) {
        i++;
    }

    for (SPObject *child = o->firstChild(); child != NULL; child = child->next) {
        i += _countHrefs(child);
    }

    return i;
}

SPPattern *SPPattern::_chain() const
{
    Inkscape::XML::Document *xml_doc = document->getReprDoc();
    Inkscape::XML::Node *defsrepr = document->getDefs()->getRepr();

    Inkscape::XML::Node *repr = xml_doc->createElement("svg:pattern");
    repr->setAttribute("inkscape:collect", "always");
    Glib::ustring parent_ref = Glib::ustring::compose("#%1", getRepr()->attribute("id"));
    repr->setAttribute("xlink:href", parent_ref);

    defsrepr->addChild(repr, NULL);
    const gchar *child_id = repr->attribute("id");
    SPObject *child = document->getObjectById(child_id);
    g_assert(SP_IS_PATTERN(child));

    return SP_PATTERN(child);
}

SPPattern *SPPattern::clone_if_necessary(SPItem *item, const gchar *property)
{
    SPPattern *pattern = this;
    if (pattern->href.empty() || pattern->hrefcount > _countHrefs(item)) {
        pattern = _chain();
        Glib::ustring href = Glib::ustring::compose("url(#%1)", pattern->getRepr()->attribute("id"));

        SPCSSAttr *css = sp_repr_css_attr_new();
        sp_repr_css_set_property(css, property, href.c_str());
        sp_repr_css_change_recursive(item->getRepr(), css, "style");
    }
    return pattern;
}

void SPPattern::transform_multiply(Geom::Affine postmul, bool set)
{
    // this formula is for a different interpretation of pattern transforms as described in (*) in sp-pattern.cpp
    // for it to work, we also need    sp_object_read_attr( item, "transform");
    // pattern->patternTransform = premul * item->transform * pattern->patternTransform * item->transform.inverse() *
    // postmul;

    // otherwise the formula is much simpler
    if (set) {
        _pattern_transform = postmul;
    }
    else {
        _pattern_transform = getTransform() * postmul;
    }
    _pattern_transform_set = true;

    Glib::ustring c = sp_svg_transform_write(_pattern_transform);
    getRepr()->setAttribute("patternTransform", c);
}

const gchar *SPPattern::produce(const std::vector<Inkscape::XML::Node *> &reprs, Geom::Rect bounds,
                                SPDocument *document, Geom::Affine transform, Geom::Affine move)
{
    typedef std::vector<Inkscape::XML::Node *>::const_iterator NodePtrIterator;

    Inkscape::XML::Document *xml_doc = document->getReprDoc();
    Inkscape::XML::Node *defsrepr = document->getDefs()->getRepr();

    Inkscape::XML::Node *repr = xml_doc->createElement("svg:pattern");
    repr->setAttribute("patternUnits", "userSpaceOnUse");
    sp_repr_set_svg_double(repr, "width", bounds.dimensions()[Geom::X]);
    sp_repr_set_svg_double(repr, "height", bounds.dimensions()[Geom::Y]);

    Glib::ustring t = sp_svg_transform_write(transform);
    repr->setAttribute("patternTransform", t);

    defsrepr->appendChild(repr);
    const gchar *pat_id = repr->attribute("id");
    SPObject *pat_object = document->getObjectById(pat_id);

    for (NodePtrIterator i = reprs.begin(); i != reprs.end(); ++i) {
        Inkscape::XML::Node *node = *i;
        SPItem *copy = SP_ITEM(pat_object->appendChildRepr(node));

        Geom::Affine dup_transform;
        if (!sp_svg_transform_read(node->attribute("transform"), &dup_transform))
            dup_transform = Geom::identity();
        dup_transform *= move;

        copy->doWriteTransform(copy->getRepr(), dup_transform, NULL, false);
    }

    Inkscape::GC::release(repr);
    return pat_id;
}

SPPattern *SPPattern::rootPattern()
{
    for (SPPattern *pat_i = this; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->firstChild()) { // find the first one with children
            return pat_i;
        }
    }
    return this; // document is broken, we can't get to root; but at least we can return pat which is supposedly a valid
                 // pattern
}



// Access functions that look up fields up the chain of referenced patterns and return the first one which is set
// FIXME: all of them must use chase_hrefs the same as in SPGradient, to avoid lockup on circular refs

SPPattern::PatternUnits SPPattern::patternUnits() const
{
    for (SPPattern const *pat_i = this; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_pattern_units_set)
            return pat_i->_pattern_units;
    }
    return _pattern_units;
}

SPPattern::PatternUnits SPPattern::patternContentUnits() const
{
    for (SPPattern const *pat_i = this; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_pattern_content_units_set)
            return pat_i->_pattern_content_units;
    }
    return _pattern_content_units;
}

Geom::Affine const &SPPattern::getTransform() const
{
    for (SPPattern const *pat_i = this; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_pattern_transform_set)
            return pat_i->_pattern_transform;
    }
    return _pattern_transform;
}

gdouble SPPattern::x() const
{
    for (SPPattern const *pat_i = this; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_x._set)
            return pat_i->_x.computed;
    }
    return 0;
}

gdouble SPPattern::y() const
{
    for (SPPattern const *pat_i = this; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_y._set)
            return pat_i->_y.computed;
    }
    return 0;
}

gdouble SPPattern::width() const
{
    for (SPPattern const *pat_i = this; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_width._set)
            return pat_i->_width.computed;
    }
    return 0;
}

gdouble SPPattern::height() const
{
    for (SPPattern const *pat_i = this; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_height._set)
            return pat_i->_height.computed;
    }
    return 0;
}

Geom::OptRect SPPattern::viewbox() const
{
    Geom::OptRect viewbox;
    for (SPPattern const *pat_i = this; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->viewBox_set) {
            viewbox = pat_i->viewBox;
            break;
        }
    }
    return viewbox;
}

bool SPPattern::_hasItemChildren() const
{
    bool hasChildren = false;
    for (SPObject const *child = firstChild(); child && !hasChildren; child = child->getNext()) {
        if (SP_IS_ITEM(child)) {
            hasChildren = true;
        }
    }
    return hasChildren;
}

bool SPPattern::isValid() const
{
    double tile_width = width();
    double tile_height = height();

    if (tile_width <= 0 || tile_height <= 0)
        return false;
    return true;
}

cairo_pattern_t *SPPattern::pattern_new(cairo_t *base_ct, Geom::OptRect const &bbox, double opacity)
{

    bool needs_opacity = (1.0 - opacity) >= 1e-3;
    bool visible = opacity >= 1e-3;

    if (!visible) {
        return NULL;
    }

    /* Show items */
    SPPattern *shown = NULL;

    for (SPPattern *pat_i = this; pat_i != NULL; pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        // find the first one with item children
        if (pat_i && SP_IS_OBJECT(pat_i) && pat_i->_hasItemChildren()) {
            shown = pat_i;
            break; // do not go further up the chain if children are found
        }
    }

    if (!shown) {
        return cairo_pattern_create_rgba(0, 0, 0, 0);
    }

    /* Create drawing for rendering */
    Inkscape::Drawing drawing;
    unsigned int dkey = SPItem::display_key_new(1);
    Inkscape::DrawingGroup *root = new Inkscape::DrawingGroup(drawing);
    drawing.setRoot(root);

    for (SPObject *child = shown->firstChild(); child != NULL; child = child->getNext()) {
        if (SP_IS_ITEM(child)) {
            // for each item in pattern, show it on our drawing, add to the group,
            // and connect to the release signal in case the item gets deleted
            Inkscape::DrawingItem *cai;
            cai = SP_ITEM(child)->invoke_show(drawing, dkey, SP_ITEM_SHOW_DISPLAY);
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
    double tile_x = x();
    double tile_y = y();
    double tile_width = width();
    double tile_height = height();
    if (bbox && (patternUnits() == UNITS_OBJECTBOUNDINGBOX)) {
        tile_x *= bbox->width();
        tile_y *= bbox->height();
        tile_width *= bbox->width();
        tile_height *= bbox->height();
    }

    // Pattern size in pattern space
    Geom::Rect pattern_tile = Geom::Rect::from_xywh(0, 0, tile_width, tile_height);

    // Content to tile (pattern space)
    Geom::Affine content2ps;
    Geom::OptRect effective_view_box = viewbox();
    if (effective_view_box) {
        // viewBox to pattern server (using SPViewBox)
        viewBox = *effective_view_box;
        c2p.setIdentity();
        apply_viewbox(pattern_tile);
        content2ps = c2p;
    }
    else {

        // Content to bbox
        if (bbox && (patternContentUnits() == UNITS_OBJECTBOUNDINGBOX)) {
            content2ps = Geom::Affine(bbox->width(), 0.0, 0.0, bbox->height(), 0, 0);
        }
    }


    // Tile (pattern space) to user.
    Geom::Affine ps2user = Geom::Translate(tile_x, tile_y) * getTransform();


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
    Geom::Point c(pattern_tile.dimensions() * ps2user.descrim() * full.descrim() * 2.0);

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
    Inkscape::UpdateContext ctx; // UpdateContext is structure with only ctm!
    ctx.ctm = content2ps * pattern_surface.drawingTransform();
    dc.transform(pattern_surface.drawingTransform().inverse());
    drawing.update(Geom::IntRect::infinite(), ctx);

    // Render drawing to pattern_surface via drawing context, this calls root->render
    // which is really DrawingItem->render().
    drawing.render(dc, one_tile);
    for (SPObject *child = shown->firstChild(); child != NULL; child = child->getNext()) {
        if (SP_IS_ITEM(child)) {
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
        dc.paint(opacity);     // apply opacity
    }

    cairo_pattern_t *cp = cairo_pattern_create_for_surface(pattern_surface.raw());
    // Apply transformation to user space. Also compensate for oversampling.
    ink_cairo_pattern_set_matrix(cp, ps2user.inverse() * pattern_surface.drawingTransform());

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

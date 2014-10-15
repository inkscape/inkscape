/** @file
 * SVG <hatch> implementation
 *//*
 * Author:
 *   Tomasz Boczkowski <penginsbacon@gmail.com>
 *
 * Copyright (C) 2014 Tomasz Boczkowski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>
#include <2geom/transforms.h>
#include <sigc++/functors/mem_fun.h>

#include "svg/svg.h"
#include "display/cairo-utils.h"
#include "display/drawing-context.h"
#include "display/drawing-surface.h"
#include "display/drawing.h"
#include "display/drawing-pattern.h"
#include "attributes.h"
#include "document-private.h"
#include "uri.h"
#include "style.h"
#include "sp-hatch.h"
#include "sp-hatch-path.h"
#include "xml/repr.h"

#include "sp-factory.h"

namespace {
SPObject* createHatch() {
    return new SPHatch();
}

bool hatchRegistered = SPFactory::instance().registerObject("svg:hatch", createHatch);
}

SPHatch::SPHatch()
        : SPPaintServer()
{
    this->ref = new SPHatchReference(this);
    this->ref->changedSignal().connect(sigc::mem_fun(this, &SPHatch::_onRefChanged));

    this->_hatchUnits = UNITS_OBJECTBOUNDINGBOX;
    this->_hatchUnits_set = false;

    this->_hatchContentUnits = UNITS_USERSPACEONUSE;
    this->_hatchContentUnits_set = false;

    this->_hatchTransform = Geom::identity();
    this->_hatchTransform_set = false;

    this->_x.unset();
    this->_y.unset();
    this->_pitch.unset();
    this->_rotate.unset();
}

SPHatch::~SPHatch() {
}

void SPHatch::build(SPDocument* doc, Inkscape::XML::Node* repr) {
    SPPaintServer::build(doc, repr);

    this->readAttr("hatchUnits");
    this->readAttr("hatchContentUnits");
    this->readAttr("hatchTransform");
    this->readAttr("x");
    this->readAttr("y");
    this->readAttr("pitch");
    this->readAttr("rotate");
    this->readAttr("xlink:href");
    this->readAttr( "style" );

    /* Register ourselves */
    doc->addResource("hatch", this);
}

void SPHatch::release() {
    if (this->document) {
        // Unregister ourselves
        this->document->removeResource("hatch", this);
    }

    std::vector<SPHatchPath *> children;
    hatchPaths(children);
    for (ViewIterator view_iter = _display.begin(); view_iter != _display.end(); view_iter++) {
        for (ChildIterator child_iter = children.begin(); child_iter != children.end();
                child_iter++) {
            SPHatchPath *child = *child_iter;
            child->hide(view_iter->key);
        }
        delete view_iter->arenaitem;
        view_iter->arenaitem = NULL;
    }

    if (this->ref) {
        this->_modified_connection.disconnect();
        this->ref->detach();
        delete this->ref;
        this->ref = NULL;
    }

    SPPaintServer::release();
}

void SPHatch::child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref) {
    SPObject::child_added(child, ref);

    SPHatchPath *path_child = SP_HATCH_PATH(this->document->getObjectByRepr(child));

    if (path_child) {
        for (ViewIterator iter = _display.begin(); iter != _display.end(); iter++) {
            Geom::OptInterval extents = _calculateStripExtents(iter->bbox);
            Inkscape::DrawingItem *ac = path_child->show(iter->arenaitem->drawing(), iter->key, extents);

            path_child->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            if (ac) {
                iter->arenaitem->prependChild(ac);
            }
        }
    }
    //FIXME: notify all hatches that refer to this child set
}

void SPHatch::set(unsigned int key, const gchar* value) {
    switch (key) {
    case SP_ATTR_HATCHUNITS:
        if (value) {
            if (!strcmp(value, "userSpaceOnUse")) {
                this->_hatchUnits = UNITS_USERSPACEONUSE;
            } else {
                this->_hatchUnits = UNITS_OBJECTBOUNDINGBOX;
            }

            this->_hatchUnits_set = true;
        } else {
            this->_hatchUnits_set = false;
        }

        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    case SP_ATTR_HATCHCONTENTUNITS:
        if (value) {
            if (!strcmp(value, "userSpaceOnUse")) {
                this->_hatchContentUnits = UNITS_USERSPACEONUSE;
            } else {
                this->_hatchContentUnits = UNITS_OBJECTBOUNDINGBOX;
            }

            this->_hatchContentUnits_set = true;
        } else {
            this->_hatchContentUnits_set = false;
        }

        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    case SP_ATTR_HATCHTRANSFORM: {
        Geom::Affine t;

        if (value && sp_svg_transform_read(value, &t)) {
            this->_hatchTransform = t;
            this->_hatchTransform_set = true;
        } else {
            this->_hatchTransform = Geom::identity();
            this->_hatchTransform_set = false;
        }

        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;
    }
    case SP_ATTR_X:
        this->_x.readOrUnset(value);
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    case SP_ATTR_Y:
        this->_y.readOrUnset(value);
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    case SP_ATTR_PITCH:
        this->_pitch.readOrUnset(value);
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    case SP_ATTR_ROTATE:
        this->_rotate.readOrUnset(value);
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    case SP_ATTR_XLINK_HREF:
        if (value && this->href == value) {
            /* Href unchanged, do nothing. */
        } else {
            this->href.clear();

            if (value) {
                // First, set the href field; it's only used in the "unchanged" check above.
                this->href = value;
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
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    default:
        if (SP_ATTRIBUTE_IS_CSS(key)) {
            sp_style_read_from_object(this->style, this);
            requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        } else {
            SPPaintServer::set(key, value);
        }
        break;
    }
}

bool SPHatch::_hasHatchPatchChildren(SPHatch const *hatch) {
    for (SPObject const *child = hatch->firstChild(); child; child = child->getNext() ) {
        if (SP_IS_HATCH_PATH(child)) {
            return true;
        }
    }
    return false;
}

void SPHatch::hatchPaths(std::vector<SPHatchPath*>& l) {
    SPHatch *src = chase_hrefs<SPHatch>(this, sigc::ptr_fun(&_hasHatchPatchChildren));

    if (src) {
        for (SPObject *child = src->firstChild(); child; child = child->getNext()) {
            if (SP_IS_HATCH_PATH(child)) {
                l.push_back(SP_HATCH_PATH(child));
            }
        }
    }
}

void SPHatch::hatchPaths(std::vector<SPHatchPath const*>& l) const {
    SPHatch const *src = chase_hrefs<SPHatch const>(this, sigc::ptr_fun(&_hasHatchPatchChildren));

    if (src) {
        for (SPObject const *child = src->firstChild(); child; child = child->getNext()) {
            if (SP_IS_HATCH_PATH(child)) {
                l.push_back(SP_HATCH_PATH(child));
            }
        }
    }
}

/* TODO: ::remove_child and ::order_changed handles - see SPPattern */


void SPHatch::update(SPCtx* ctx, unsigned int flags) {
    typedef std::list<SPHatch::View>::iterator ViewIterator;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    std::vector<SPHatchPath *> children;
    hatchPaths(children);

    for (ChildIterator iter = children.begin(); iter != children.end(); iter++) {
        SPHatchPath* child = *iter;

        sp_object_ref(child, NULL);

        for (ViewIterator view_iter = _display.begin(); view_iter != _display.end(); view_iter++) {
            Geom::OptInterval strip_extents = _calculateStripExtents(view_iter->bbox);
            child->setStripExtents(view_iter->key, strip_extents);
        }

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {

            child->updateDisplay(ctx, flags);
        }

        sp_object_unref(child, NULL);
    }

    for (ViewIterator iter = _display.begin(); iter != _display.end(); iter++) {
        _updateView(*iter);
    }
}

void SPHatch::modified(unsigned int flags) {
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    std::vector<SPHatchPath *> children;
    hatchPaths(children);

    for (ChildIterator iter = children.begin(); iter != children.end(); iter++) {
        SPObject *child = *iter;

        sp_object_ref(child, NULL);

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }

        sp_object_unref(child, NULL);
    }
}

void SPHatch::_onRefChanged(SPObject *old_ref, SPObject *ref) {
    typedef std::list<SPHatch::View>::iterator ViewIterator;

    if (old_ref) {
        _modified_connection.disconnect();
    }

    if (SP_IS_HATCH(ref)) {
        _modified_connection = ref->connectModified(sigc::mem_fun(this, &SPHatch::_onRefModified));
    }

    if (!_hasHatchPatchChildren(this)) {
        SPHatch *old_shown = NULL;
        SPHatch *new_shown = NULL;
        std::vector<SPHatchPath *> oldhatchPaths;
        std::vector<SPHatchPath *> newhatchPaths;
        if (SP_IS_HATCH(old_ref)) {
            old_shown = SP_HATCH(old_ref)->rootHatch();
            old_shown->hatchPaths(oldhatchPaths);
        }
        if (SP_IS_HATCH(ref)) {
            new_shown = SP_HATCH(ref)->rootHatch();
            new_shown->hatchPaths(newhatchPaths);
        }
        if (old_shown != new_shown) {

            for (ViewIterator iter = _display.begin(); iter != _display.end(); iter++) {
                Geom::OptInterval extents = _calculateStripExtents(iter->bbox);

                for (ChildIterator child_iter = oldhatchPaths.begin(); child_iter != oldhatchPaths.end(); child_iter++) {
                    SPHatchPath *child = *child_iter;
                    child->hide(iter->key);
                }
                for (ChildIterator child_iter = newhatchPaths.begin(); child_iter != newhatchPaths.end(); child_iter++) {
                    SPHatchPath *child = *child_iter;
                    Inkscape::DrawingItem *cai = child->show(iter->arenaitem->drawing(), iter->key, extents);
                    child->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
                    if (cai) {
                        iter->arenaitem->appendChild(cai);
                    }

                }
            }
        }
    }

    _onRefModified(ref, 0);
}

void SPHatch::_onRefModified(SPObject */*ref*/, guint /*flags*/) {
    requestModified(SP_OBJECT_MODIFIED_FLAG);
    // Conditional to avoid causing infinite loop if there's a cycle in the href chain.
}


SPHatch *SPHatch::rootHatch() {
    SPHatch *src = chase_hrefs<SPHatch>(this, sigc::ptr_fun(&_hasHatchPatchChildren));
    return src ? src : this; // document is broken, we can't get to root; but at least we can return pat which is supposedly a valid hatch
}

// Access functions that look up fields up the chain of referenced hatchs and return the first one which is set
// FIXME: all of them must use chase_hrefs as children() and rootHatch()

SPHatch::HatchUnits SPHatch::hatchUnits() const {
    for (SPHatch const *pat_i = this; pat_i != NULL;
            pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_hatchUnits_set)
            return pat_i->_hatchUnits;
    }
    return _hatchUnits;
}

SPHatch::HatchUnits SPHatch::hatchContentUnits() const {
    for (SPHatch const *pat_i = this; pat_i != NULL;
            pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_hatchContentUnits_set)
            return pat_i->_hatchContentUnits;
    }
    return _hatchContentUnits;
}

Geom::Affine const &SPHatch::hatchTransform() const {
    for (SPHatch const *pat_i = this; pat_i != NULL;
            pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_hatchTransform_set)
            return pat_i->_hatchTransform;
    }
    return _hatchTransform;
}

gdouble SPHatch::x() const {
    for (SPHatch const *pat_i = this; pat_i != NULL;
            pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_x._set)
            return pat_i->_x.computed;
    }
    return 0;
}

gdouble SPHatch::y() const {
    for (SPHatch const *pat_i = this; pat_i != NULL;
            pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_y._set)
            return pat_i->_y.computed;
    }
    return 0;
}

gdouble SPHatch::pitch() const {
    for (SPHatch const *pat_i = this; pat_i != NULL;
            pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_pitch._set)
            return pat_i->_pitch.computed;
    }
    return 0;
}

gdouble SPHatch::rotate() const {
    for (SPHatch const *pat_i = this; pat_i != NULL;
            pat_i = pat_i->ref ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_rotate._set)
            return pat_i->_rotate.computed;
    }
    return 0;
}

bool SPHatch::isValid() const {
    double strip_pitch = pitch();
    if (strip_pitch <= 0) {
        return false;
    }

    std::vector<SPHatchPath const *> children;
    hatchPaths(children);
    if (children.empty()) {
        return false;
    }
    for (ConstChildIterator iter = children.begin(); iter != children.end(); iter++) {
        SPHatchPath const *child = *iter;
        if (!child->isValid()) {
            return false;
        }
    }

    return true;
}

Inkscape::DrawingPattern *SPHatch::show(Inkscape::Drawing &drawing, unsigned int key, Geom::OptRect bbox) {
    Inkscape::DrawingPattern *ai = new Inkscape::DrawingPattern(drawing);
    //TODO: set some debug flag to see DrawingPattern
    _display.push_front(View(ai, key));
    _display.front().bbox = bbox;

    std::vector<SPHatchPath *> children;
    hatchPaths(children);

    Geom::OptInterval extents = _calculateStripExtents(bbox);
    for (ChildIterator iter = children.begin(); iter != children.end(); iter++) {
        SPHatchPath *child = *iter;
        Inkscape::DrawingItem *cai = child->show(drawing, key, extents);
        if (cai) {
            ai->appendChild(cai);
        }
    }

    View& view = _display.front();
    _updateView(view);

    return ai;
}

void SPHatch::hide(unsigned int key) {
    std::vector<SPHatchPath *> children;
    hatchPaths(children);

    for (ChildIterator iter = children.begin(); iter != children.end(); iter++) {
        SPHatchPath *child = *iter;
        child->hide(key);
    }

    for (ViewIterator iter = _display.begin(); iter != _display.end(); iter++) {
        if (iter->key == key) {
            delete iter->arenaitem;
            _display.erase(iter);
            return;
        }
    }

    g_assert_not_reached();
}


Geom::Interval SPHatch::bounds() const {
    Geom::Interval result;
    std::vector<SPHatchPath const *> children;
    hatchPaths(children);

    for (ConstChildIterator iter = children.begin(); iter != children.end(); iter++) {
        SPHatchPath const *child = *iter;
        if (result.extent() == 0) {
            result = child->bounds();
        } else {
            result |= child->bounds();
        }
    }
    return result;
}

SPHatch::RenderInfo SPHatch::calculateRenderInfo(unsigned key) const {
    RenderInfo info;
    for (ConstViewIterator iter = _display.begin(); iter != _display.end(); iter++) {
        if (iter->key == key) {
            return _calculateRenderInfo(*iter);
        }
    }
    g_assert_not_reached();
    return info;
}

void SPHatch::_updateView(View &view) {
    RenderInfo info = _calculateRenderInfo(view);
    //The rendering of hatch overflow is implemented by repeated drawing
    //of hatch paths over one strip. Within each iteration paths are moved by pitch value.
    //The movement progresses from right to left. This gives the same result
    //as drawing whole strips in left-to-right order.


    view.arenaitem->setChildTransform(info.child_transform);
    view.arenaitem->setPatternToUserTransform(info.pattern_to_user_transform);
    view.arenaitem->setTileRect(info.tile_rect);
    view.arenaitem->setStyle(this->style);
    view.arenaitem->setOverflow(info.overflow_initial_transform, info.overflow_steps,
            info.overflow_step_transform);
}

SPHatch::RenderInfo SPHatch::_calculateRenderInfo(View const &view) const {
    RenderInfo info;

    Geom::OptInterval extents = _calculateStripExtents(view.bbox);
    if (!extents) {
        return info;
    }

    double tile_x = x();
    double tile_y = y();
    double tile_width = pitch();
    double tile_height = extents->max() - extents->min();
    double tile_rotate = rotate();
    double tile_render_y = extents->min();

    if (view.bbox && (hatchUnits() == UNITS_OBJECTBOUNDINGBOX)) {
        tile_x *= view.bbox->width();
        tile_y *= view.bbox->height();
        tile_width *= view.bbox->width();
        tile_height *= view.bbox->height();
        tile_render_y *= view.bbox->height();
    }

    // Pattern size in hatch space
    Geom::Rect hatch_tile = Geom::Rect::from_xywh(0, tile_render_y, tile_width, tile_height);
    // Content to bbox
    Geom::Affine content2ps;
    if (view.bbox && (hatchContentUnits() == UNITS_OBJECTBOUNDINGBOX)) {
        content2ps = Geom::Affine(view.bbox->width(), 0.0, 0.0, view.bbox->height(), 0, 0);
    }

    // Tile (hatch space) to user.
    Geom::Affine ps2user = Geom::Translate(tile_x, tile_y) * Geom::Rotate::from_degrees(tile_rotate) * hatchTransform();

    info.child_transform = content2ps;
    info.pattern_to_user_transform = ps2user;
    info.tile_rect = hatch_tile;

    if (style->overflow.computed == SP_CSS_OVERFLOW_VISIBLE) {
        Geom::Interval bounds = this->bounds();
        gdouble pitch = this->pitch();
        gdouble overflow_right_strip = floor(bounds.max() / pitch) * pitch;
        info.overflow_steps = ceil((overflow_right_strip - bounds.min()) / pitch) + 1;
        info.overflow_step_transform = Geom::Translate(pitch, 0.0);
        info.overflow_initial_transform = Geom::Translate(-overflow_right_strip, 0.0);
    } else {
        info.overflow_steps = 1;
    }

    return info;
}

//calculates strip extents in content space
Geom::OptInterval SPHatch::_calculateStripExtents(Geom::OptRect bbox) const {
    if (!bbox || (bbox->area() == 0)) {
        return Geom::OptInterval();
    }

    double tile_x = x();
    double tile_y = y();
    double tile_rotate = rotate();

    Geom::Affine ps2user = Geom::Translate(tile_x, tile_y) * Geom::Rotate::from_degrees(tile_rotate) * hatchTransform();
    Geom::Affine user2ps = ps2user.inverse();

    Geom::Interval extents;
    for (int i = 0; i < 4; i++) {
        Geom::Point corner = bbox->corner(i);
        Geom::Point corner_ps  =  corner * user2ps;
        if (i == 0 || corner_ps.y() < extents.min()) {
            extents.setMin(corner_ps.y());
        }
        if (i == 0 || corner_ps.y() > extents.max()) {
            extents.setMax(corner_ps.y());
        }
    }

    if (hatchUnits() == UNITS_OBJECTBOUNDINGBOX) {
        extents /= bbox->height();
    }

    return extents;
}

cairo_pattern_t* SPHatch::pattern_new(cairo_t * /*base_ct*/, Geom::OptRect const &/*bbox*/, double /*opacity*/)
{
    //this code should not be used
    //it is however required by the fact that SPPaintServer::hatch_new is pure virtual
    return cairo_pattern_create_rgb(0.5, 0.5, 1.0);
}

void SPHatch::setBBox(unsigned int key, Geom::OptRect const &bbox) {
    for (ViewIterator iter = _display.begin(); iter != _display.end(); iter++) {
        if (iter->key == key) {
            iter->bbox = bbox;
            break;
        }
    }
}

SPHatch::View::View(Inkscape::DrawingPattern *arenaitem, int key)
        : arenaitem(arenaitem), key(key)
{
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

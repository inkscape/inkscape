/**
 * @file
 * SVG <hatch> implementation
 */
/*
 * Authors:
 *   Tomasz Boczkowski <penginsbacon@gmail.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2014 Tomasz Boczkowski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

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

SPHatch::SPHatch()
    : SPPaintServer(),
      href(),
      ref(NULL), // avoiding 'this' in initializer list
      _hatchUnits(UNITS_OBJECTBOUNDINGBOX),
      _hatchUnits_set(false),
      _hatchContentUnits(UNITS_USERSPACEONUSE),
      _hatchContentUnits_set(false),
      _hatchTransform(Geom::identity()),
      _hatchTransform_set(false),
      _x(),
      _y(),
      _pitch(),
      _rotate(),
      _modified_connection(),
      _display()
{
    ref = new SPHatchReference(this);
    ref->changedSignal().connect(sigc::mem_fun(this, &SPHatch::_onRefChanged));

    // TODO check that these should start already as unset:
    _x.unset();
    _y.unset();
    _pitch.unset();
    _rotate.unset();
}

SPHatch::~SPHatch() {
}

void SPHatch::build(SPDocument* doc, Inkscape::XML::Node* repr)
{
    SPPaintServer::build(doc, repr);

    readAttr("hatchUnits");
    readAttr("hatchContentUnits");
    readAttr("hatchTransform");
    readAttr("x");
    readAttr("y");
    readAttr("pitch");
    readAttr("rotate");
    readAttr("xlink:href");
    readAttr( "style" );

    // Register ourselves
    doc->addResource("hatch", this);
}

void SPHatch::release()
{
    if (document) {
        // Unregister ourselves
        document->removeResource("hatch", this);
    }

    std::vector<SPHatchPath *> children(hatchPaths());
    for (ViewIterator view_iter = _display.begin(); view_iter != _display.end(); ++view_iter) {
        for (ChildIterator child_iter = children.begin(); child_iter != children.end(); ++child_iter) {
            SPHatchPath *child = *child_iter;
            child->hide(view_iter->key);
        }
        delete view_iter->arenaitem;
        view_iter->arenaitem = NULL;
    }

    if (ref) {
        _modified_connection.disconnect();
        ref->detach();
        delete ref;
        ref = NULL;
    }

    SPPaintServer::release();
}

void SPHatch::child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref)
{
    SPObject::child_added(child, ref);

    SPHatchPath *path_child = dynamic_cast<SPHatchPath *>(document->getObjectByRepr(child));

    if (path_child) {
        for (ViewIterator iter = _display.begin(); iter != _display.end(); ++iter) {
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

void SPHatch::set(unsigned int key, const gchar* value)
{
    switch (key) {
    case SP_ATTR_HATCHUNITS:
        if (value) {
            if (!strcmp(value, "userSpaceOnUse")) {
                _hatchUnits = UNITS_USERSPACEONUSE;
            } else {
                _hatchUnits = UNITS_OBJECTBOUNDINGBOX;
            }

            _hatchUnits_set = true;
        } else {
            _hatchUnits_set = false;
        }

        requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    case SP_ATTR_HATCHCONTENTUNITS:
        if (value) {
            if (!strcmp(value, "userSpaceOnUse")) {
                _hatchContentUnits = UNITS_USERSPACEONUSE;
            } else {
                _hatchContentUnits = UNITS_OBJECTBOUNDINGBOX;
            }

            _hatchContentUnits_set = true;
        } else {
            _hatchContentUnits_set = false;
        }

        requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    case SP_ATTR_HATCHTRANSFORM: {
        Geom::Affine t;

        if (value && sp_svg_transform_read(value, &t)) {
            _hatchTransform = t;
            _hatchTransform_set = true;
        } else {
            _hatchTransform = Geom::identity();
            _hatchTransform_set = false;
        }

        requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;
    }
    case SP_ATTR_X:
        _x.readOrUnset(value);
        requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    case SP_ATTR_Y:
        _y.readOrUnset(value);
        requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    case SP_ATTR_PITCH:
        _pitch.readOrUnset(value);
        requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    case SP_ATTR_ROTATE:
        _rotate.readOrUnset(value);
        requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    case SP_ATTR_XLINK_HREF:
        if (value && href == value) {
            // Href unchanged, do nothing.
        } else {
            href.clear();

            if (value) {
                // First, set the href field; it's only used in the "unchanged" check above.
                href = value;
                // Now do the attaching, which emits the changed signal.
                if (value) {
                    try {
                        ref->attach(Inkscape::URI(value));
                    } catch (Inkscape::BadURIException &e) {
                        g_warning("%s", e.what());
                        ref->detach();
                    }
                } else {
                    ref->detach();
                }
            }
        }
        requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        break;

    default:
        if (SP_ATTRIBUTE_IS_CSS(key)) {
            style->readFromObject( this );
            requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        } else {
            SPPaintServer::set(key, value);
        }
        break;
    }
}

bool SPHatch::_hasHatchPatchChildren(SPHatch const *hatch)
{
    bool matched = false;
    for (SPObject const *child = hatch->firstChild(); child && !matched; child = child->getNext() ) {
        SPHatchPath const *hatchPath = dynamic_cast<SPHatchPath const *>(child);
        if (hatchPath) {
            matched = true;
        }
    }
    return matched;
}

std::vector<SPHatchPath*> SPHatch::hatchPaths()
{
    std::vector<SPHatchPath*> list;
    SPHatch *src = chase_hrefs<SPHatch>(this, sigc::ptr_fun(&_hasHatchPatchChildren));

    if (src) {
        for (SPObject *child = src->firstChild(); child; child = child->getNext()) {
            SPHatchPath *hatchPath = dynamic_cast<SPHatchPath *>(child);
            if (hatchPath) {
                list.push_back(hatchPath);
            }
        }
    }
    return list;
}

std::vector<SPHatchPath const*> SPHatch::hatchPaths() const
{
    std::vector<SPHatchPath const*> list;
    SPHatch const *src = chase_hrefs<SPHatch const>(this, sigc::ptr_fun(&_hasHatchPatchChildren));

    if (src) {
        for (SPObject const *child = src->firstChild(); child; child = child->getNext()) {
            SPHatchPath const *hatchPath = dynamic_cast<SPHatchPath const*>(child);
            if (hatchPath) {
                list.push_back(hatchPath);
            }
        }
    }
    return list;
}

// TODO: ::remove_child and ::order_changed handles - see SPPattern


void SPHatch::update(SPCtx* ctx, unsigned int flags)
{
    typedef std::list<SPHatch::View>::iterator ViewIterator;

    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    std::vector<SPHatchPath *> children(hatchPaths());

    for (ChildIterator iter = children.begin(); iter != children.end(); ++iter) {
        SPHatchPath* child = *iter;

        sp_object_ref(child, NULL);

        for (ViewIterator view_iter = _display.begin(); view_iter != _display.end(); ++view_iter) {
            Geom::OptInterval strip_extents = _calculateStripExtents(view_iter->bbox);
            child->setStripExtents(view_iter->key, strip_extents);
        }

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {

            child->updateDisplay(ctx, flags);
        }

        sp_object_unref(child, NULL);
    }

    for (ViewIterator iter = _display.begin(); iter != _display.end(); ++iter) {
        _updateView(*iter);
    }
}

void SPHatch::modified(unsigned int flags)
{
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    std::vector<SPHatchPath *> children(hatchPaths());

    for (ChildIterator iter = children.begin(); iter != children.end(); ++iter) {
        SPObject *child = *iter;

        sp_object_ref(child, NULL);

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }

        sp_object_unref(child, NULL);
    }
}

void SPHatch::_onRefChanged(SPObject *old_ref, SPObject *ref)
{
    typedef std::list<SPHatch::View>::iterator ViewIterator;

    if (old_ref) {
        _modified_connection.disconnect();
    }

    SPHatch *hatch = dynamic_cast<SPHatch *>(ref);
    if (hatch) {
        _modified_connection = ref->connectModified(sigc::mem_fun(this, &SPHatch::_onRefModified));
    }

    if (!_hasHatchPatchChildren(this)) {
        SPHatch *old_shown = NULL;
        SPHatch *new_shown = NULL;
        std::vector<SPHatchPath *> oldhatchPaths;
        std::vector<SPHatchPath *> newhatchPaths;

        SPHatch *old_hatch = dynamic_cast<SPHatch *>(old_ref);
        if (old_hatch) {
            old_shown = old_hatch->rootHatch();
            oldhatchPaths = old_shown->hatchPaths();
        }
        if (hatch) {
            new_shown = hatch->rootHatch();
            newhatchPaths = new_shown->hatchPaths();
        }
        if (old_shown != new_shown) {

            for (ViewIterator iter = _display.begin(); iter != _display.end(); ++iter) {
                Geom::OptInterval extents = _calculateStripExtents(iter->bbox);

                for (ChildIterator child_iter = oldhatchPaths.begin(); child_iter != oldhatchPaths.end(); ++child_iter) {
                    SPHatchPath *child = *child_iter;
                    child->hide(iter->key);
                }
                for (ChildIterator child_iter = newhatchPaths.begin(); child_iter != newhatchPaths.end(); ++child_iter) {
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

void SPHatch::_onRefModified(SPObject */*ref*/, guint /*flags*/)
{
    requestModified(SP_OBJECT_MODIFIED_FLAG);
    // Conditional to avoid causing infinite loop if there's a cycle in the href chain.
}


SPHatch *SPHatch::rootHatch()
{
    SPHatch *src = chase_hrefs<SPHatch>(this, sigc::ptr_fun(&_hasHatchPatchChildren));
    return src ? src : this; // document is broken, we can't get to root; but at least we can return pat which is supposedly a valid hatch
}

// Access functions that look up fields up the chain of referenced hatchs and return the first one which is set
// FIXME: all of them must use chase_hrefs as children() and rootHatch()

SPHatch::HatchUnits SPHatch::hatchUnits() const
{
    HatchUnits units = _hatchUnits;
    for (SPHatch const *pat_i = this; pat_i; pat_i = (pat_i->ref) ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_hatchUnits_set) {
            units = pat_i->_hatchUnits;
            break;
        }
    }
    return units;
}

SPHatch::HatchUnits SPHatch::hatchContentUnits() const
{
    HatchUnits units = _hatchContentUnits;
    for (SPHatch const *pat_i = this; pat_i; pat_i = (pat_i->ref) ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_hatchContentUnits_set) {
            units = pat_i->_hatchContentUnits;
            break;
        }
    }
    return units;
}

Geom::Affine const &SPHatch::hatchTransform() const
{
    for (SPHatch const *pat_i = this; pat_i; pat_i = (pat_i->ref) ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_hatchTransform_set) {
            return pat_i->_hatchTransform;
        }
    }
    return _hatchTransform;
}

gdouble SPHatch::x() const
{
    gdouble val = 0;
    for (SPHatch const *pat_i = this; pat_i; pat_i = (pat_i->ref) ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_x._set) {
            val = pat_i->_x.computed;
            break;
        }
    }
    return val;
}

gdouble SPHatch::y() const
{
    gdouble val = 0;
    for (SPHatch const *pat_i = this; pat_i; pat_i = (pat_i->ref) ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_y._set) {
            val = pat_i->_y.computed;
            break;
        }
    }
    return val;
}

gdouble SPHatch::pitch() const
{
    gdouble val = 0;
    for (SPHatch const *pat_i = this; pat_i; pat_i = (pat_i->ref) ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_pitch._set) {
            val = pat_i->_pitch.computed;
            break;
        }
    }
    return val;
}

gdouble SPHatch::rotate() const
{
    gdouble val = 0;
    for (SPHatch const *pat_i = this; pat_i; pat_i = (pat_i->ref) ? pat_i->ref->getObject() : NULL) {
        if (pat_i->_rotate._set) {
            val = pat_i->_rotate.computed;
            break;
        }
    }
    return val;
}

bool SPHatch::isValid() const
{
    bool valid = false;

    if (pitch() > 0) {
        std::vector<SPHatchPath const *> children(hatchPaths());
        if (!children.empty()) {
            valid = true;
            for (ConstChildIterator iter = children.begin(); (iter != children.end()) && valid; ++iter) {
                SPHatchPath const *child = *iter;
                valid = child->isValid();
            }
        }
    }

    return valid;
}

Inkscape::DrawingPattern *SPHatch::show(Inkscape::Drawing &drawing, unsigned int key, Geom::OptRect bbox)
{
    Inkscape::DrawingPattern *ai = new Inkscape::DrawingPattern(drawing);
    //TODO: set some debug flag to see DrawingPattern
    _display.push_front(View(ai, key));
    _display.front().bbox = bbox;

    std::vector<SPHatchPath *> children(hatchPaths());

    Geom::OptInterval extents = _calculateStripExtents(bbox);
    for (ChildIterator iter = children.begin(); iter != children.end(); ++iter) {
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

void SPHatch::hide(unsigned int key)
{
    std::vector<SPHatchPath *> children(hatchPaths());

    for (ChildIterator iter = children.begin(); iter != children.end(); ++iter) {
        SPHatchPath *child = *iter;
        child->hide(key);
    }

    for (ViewIterator iter = _display.begin(); iter != _display.end(); ++iter) {
        if (iter->key == key) {
            delete iter->arenaitem;
            _display.erase(iter);
            return;
        }
    }

    g_assert_not_reached();
}


Geom::Interval SPHatch::bounds() const
{
    Geom::Interval result;
    std::vector<SPHatchPath const *> children(hatchPaths());

    for (ConstChildIterator iter = children.begin(); iter != children.end(); ++iter) {
        SPHatchPath const *child = *iter;
        if (result.extent() == 0) {
            result = child->bounds();
        } else {
            result |= child->bounds();
        }
    }
    return result;
}

SPHatch::RenderInfo SPHatch::calculateRenderInfo(unsigned key) const
{
    RenderInfo info;
    for (ConstViewIterator iter = _display.begin(); iter != _display.end(); ++iter) {
        if (iter->key == key) {
            return _calculateRenderInfo(*iter);
        }
    }
    g_assert_not_reached();
    return info;
}

void SPHatch::_updateView(View &view)
{
    RenderInfo info = _calculateRenderInfo(view);
    //The rendering of hatch overflow is implemented by repeated drawing
    //of hatch paths over one strip. Within each iteration paths are moved by pitch value.
    //The movement progresses from right to left. This gives the same result
    //as drawing whole strips in left-to-right order.


    view.arenaitem->setChildTransform(info.child_transform);
    view.arenaitem->setPatternToUserTransform(info.pattern_to_user_transform);
    view.arenaitem->setTileRect(info.tile_rect);
    view.arenaitem->setStyle(style);
    view.arenaitem->setOverflow(info.overflow_initial_transform, info.overflow_steps,
                                info.overflow_step_transform);
}

SPHatch::RenderInfo SPHatch::_calculateRenderInfo(View const &view) const
{
    RenderInfo info;

    Geom::OptInterval extents = _calculateStripExtents(view.bbox);
    if (extents) {
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
    }

    return info;
}

//calculates strip extents in content space
Geom::OptInterval SPHatch::_calculateStripExtents(Geom::OptRect const &bbox) const
{
    if (!bbox || (bbox->area() == 0)) {
        return Geom::OptInterval();
    } else {
        double tile_x = x();
        double tile_y = y();
        double tile_rotate = rotate();

        Geom::Affine ps2user = Geom::Translate(tile_x, tile_y) * Geom::Rotate::from_degrees(tile_rotate) * hatchTransform();
        Geom::Affine user2ps = ps2user.inverse();

        Geom::Interval extents;
        for (int i = 0; i < 4; ++i) {
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
}

cairo_pattern_t* SPHatch::pattern_new(cairo_t * /*base_ct*/, Geom::OptRect const &/*bbox*/, double /*opacity*/)
{
    //this code should not be used
    //it is however required by the fact that SPPaintServer::hatch_new is pure virtual
    return cairo_pattern_create_rgb(0.5, 0.5, 1.0);
}

void SPHatch::setBBox(unsigned int key, Geom::OptRect const &bbox)
{
    for (ViewIterator iter = _display.begin(); iter != _display.end(); ++iter) {
        if (iter->key == key) {
            iter->bbox = bbox;
            break;
        }
    }
}

//

SPHatch::RenderInfo::RenderInfo()
    : child_transform(),
      pattern_to_user_transform(),
      tile_rect(),
      overflow_steps(0),
      overflow_step_transform(),
      overflow_initial_transform()
{
}

SPHatch::RenderInfo::~RenderInfo()
{
}

//

SPHatch::View::View(Inkscape::DrawingPattern *arenaitem, int key)
    : arenaitem(arenaitem),
      bbox(),
      key(key)
{
}

SPHatch::View::~View()
{
    // remember, do not delete arenaitem here
    arenaitem = NULL;
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

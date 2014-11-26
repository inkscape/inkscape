/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2001-2006 authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "sp-item.h"
#include "svg/svg.h"
#include "print.h"
#include "display/drawing-item.h"
#include "attributes.h"
#include "document.h"
#include "uri.h"
#include "inkscape.h"
#include "desktop.h"
#include "desktop-handles.h"

#include "style.h"
#include <glibmm/i18n.h>
#include "sp-root.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "sp-rect.h"
#include "sp-use.h"
#include "sp-text.h"
#include "sp-textpath.h"
#include "sp-item-rm-unsatisfied-cns.h"
#include "sp-pattern.h"
#include "sp-paint-server.h"
#include "sp-switch.h"
#include "sp-guide-constraint.h"
#include "gradient-chemistry.h"
#include "preferences.h"
#include "conn-avoid-ref.h"
#include "conditions.h"
#include "sp-filter-reference.h"
#include "filter-chemistry.h"
#include "sp-guide.h"
#include "sp-title.h"
#include "sp-desc.h"

#include "util/find-last-if.h"
#include "util/reverse-list.h"
#include <2geom/rect.h>
#include <2geom/affine.h>
#include <2geom/transforms.h>

#include "xml/repr.h"
#include "extract-uri.h"
#include "helper/geom.h"

#include "live_effects/lpeobject.h"
#include "live_effects/effect.h"
#include "live_effects/lpeobject-reference.h"

#include "util/units.h"

#define noSP_ITEM_DEBUG_IDLE


static SPItemView*          sp_item_view_list_remove(SPItemView     *list,
                                                     SPItemView     *view);


SPItem::SPItem() : SPObject() {
	this->sensitive = 0;
	this->clip_ref = NULL;
	this->avoidRef = NULL;
	this->_is_evaluated = false;
	this->stop_paint = 0;
	this->_evaluated_status = StatusUnknown;
	this->bbox_valid = 0;
	this->freeze_stroke_width = false;
	this->transform_center_x = 0;
	this->transform_center_y = 0;
	this->display = NULL;
	this->mask_ref = NULL;

    sensitive = TRUE;
    bbox_valid = FALSE;

    _highlightColor = NULL;

    transform_center_x = 0;
    transform_center_y = 0;

    _is_evaluated = true;
    _evaluated_status = StatusUnknown;

    transform = Geom::identity();
    doc_bbox = Geom::OptRect();
    freeze_stroke_width = false;

    display = NULL;

    clip_ref = new SPClipPathReference(this);
    clip_ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(clip_ref_changed), this));

    mask_ref = new SPMaskReference(this);
    mask_ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(mask_ref_changed), this));

    style->signal_fill_ps_changed.connect(sigc::bind(sigc::ptr_fun(fill_ps_ref_changed), this));
    style->signal_stroke_ps_changed.connect(sigc::bind(sigc::ptr_fun(stroke_ps_ref_changed), this));

    avoidRef = new SPAvoidRef(this);
}

SPItem::~SPItem() {
}

bool SPItem::isVisibleAndUnlocked() const {
    return (!isHidden() && !isLocked());
}

bool SPItem::isVisibleAndUnlocked(unsigned display_key) const {
    return (!isHidden(display_key) && !isLocked());
}

bool SPItem::isLocked() const {
    for (SPObject const *o = this; o != NULL; o = o->parent) {
        SPItem const *item = dynamic_cast<SPItem const *>(o);
        if (item && !(item->sensitive)) {
            return true;
        }
    }
    return false;
}

void SPItem::setLocked(bool locked) {
    setAttribute("sodipodi:insensitive",
                 ( locked ? "1" : NULL ));
    updateRepr();
}

bool SPItem::isHidden() const {
    if (!isEvaluated())
        return true;
    return style->display.computed == SP_CSS_DISPLAY_NONE;
}

void SPItem::setHidden(bool hide) {
    style->display.set = TRUE;
    style->display.value = ( hide ? SP_CSS_DISPLAY_NONE : SP_CSS_DISPLAY_INLINE );
    style->display.computed = style->display.value;
    style->display.inherit = FALSE;
    updateRepr();
}

bool SPItem::isHidden(unsigned display_key) const {
    if (!isEvaluated())
        return true;
    for ( SPItemView *view(display) ; view ; view = view->next ) {
        if ( view->key == display_key ) {
            g_assert(view->arenaitem != NULL);
            for ( Inkscape::DrawingItem *arenaitem = view->arenaitem ;
                  arenaitem ; arenaitem = arenaitem->parent() )
            {
                if (!arenaitem->visible()) {
                    return true;
                }
            }
            return false;
        }
    }
    return true;
}

bool SPItem::isHighlightSet() const {
    return _highlightColor != NULL;
}

guint32 SPItem::highlight_color() const {
    if (_highlightColor)
    {
        return atoi(_highlightColor) | 0x00000000;
    }
    else {
        SPItem const *item = dynamic_cast<SPItem const *>(parent);
        if (parent && (parent != this) && item)
        {
            return item->highlight_color();
        }
        else
        {
            static Inkscape::Preferences *prefs = Inkscape::Preferences::get();
            return prefs->getInt("/tools/nodes/highlight_color", 0xff0000ff) | 0x00000000;
        }
    }
}

void SPItem::setEvaluated(bool evaluated) {
    _is_evaluated = evaluated;
    _evaluated_status = StatusSet;
}

void SPItem::resetEvaluated() {
    if ( StatusCalculated == _evaluated_status ) {
        _evaluated_status = StatusUnknown;
        bool oldValue = _is_evaluated;
        if ( oldValue != isEvaluated() ) {
            requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
        }
    } if ( StatusSet == _evaluated_status ) {
        SPSwitch *switchItem = dynamic_cast<SPSwitch *>(parent);
        if (switchItem) {
            switchItem->resetChildEvaluated();
        }
    }
}

bool SPItem::isEvaluated() const {
    if ( StatusUnknown == _evaluated_status ) {
        _is_evaluated = sp_item_evaluate(this);
        _evaluated_status = StatusCalculated;
    }
    return _is_evaluated;
}

bool SPItem::isExplicitlyHidden() const
{
    return (style->display.set
            && style->display.value == SP_CSS_DISPLAY_NONE);
}

void SPItem::setExplicitlyHidden(bool val) {
    style->display.set = val;
    style->display.value = ( val ? SP_CSS_DISPLAY_NONE : SP_CSS_DISPLAY_INLINE );
    style->display.computed = style->display.value;
    updateRepr();
}

void SPItem::setCenter(Geom::Point const &object_centre) {
    document->ensureUpToDate();

    // Copied from DocumentProperties::onDocUnitChange()
    gdouble viewscale = 1.0;
    Geom::Rect vb = this->document->getRoot()->viewBox;
    if ( !vb.hasZeroArea() ) {
        gdouble viewscale_w = this->document->getWidth().value("px") / vb.width();
        gdouble viewscale_h = this->document->getHeight().value("px")/ vb.height();
        viewscale = std::min(viewscale_h, viewscale_w);
    }

    // FIXME this is seriously wrong
    Geom::OptRect bbox = desktopGeometricBounds();
    if (bbox) {
        // object centre is document coordinates (i.e. in pixels), so we need to consider the viewbox
        // to translate to user units; transform_center_x/y is in user units
        transform_center_x = (object_centre[Geom::X] - bbox->midpoint()[Geom::X])/viewscale;
        if (Geom::are_near(transform_center_x, 0)) // rounding error
            transform_center_x = 0;
        transform_center_y = (object_centre[Geom::Y] - bbox->midpoint()[Geom::Y])/viewscale;
        if (Geom::are_near(transform_center_y, 0)) // rounding error
            transform_center_y = 0;
    }
}

void
SPItem::unsetCenter() {
    transform_center_x = 0;
    transform_center_y = 0;
}

bool SPItem::isCenterSet() const {
    return (transform_center_x != 0 || transform_center_y != 0);
}

// Get the item's transformation center in document coordinates (i.e. in pixels)
Geom::Point SPItem::getCenter() const {
    document->ensureUpToDate();

    // Copied from DocumentProperties::onDocUnitChange()
    gdouble viewscale = 1.0;
    Geom::Rect vb = this->document->getRoot()->viewBox;
    if ( !vb.hasZeroArea() ) {
        gdouble viewscale_w = this->document->getWidth().value("px") / vb.width();
        gdouble viewscale_h = this->document->getHeight().value("px")/ vb.height();
        viewscale = std::min(viewscale_h, viewscale_w);
    }

    // FIXME this is seriously wrong
    Geom::OptRect bbox = desktopGeometricBounds();
    if (bbox) {
        // transform_center_x/y are stored in user units, so we have to take the viewbox into account to translate to document coordinates
        return bbox->midpoint() + Geom::Point (transform_center_x*viewscale, transform_center_y*viewscale);

    } else {
        return Geom::Point(0, 0); // something's wrong!
    }

}

void
SPItem::scaleCenter(Geom::Scale const &sc) {
    transform_center_x *= sc[Geom::X];
    transform_center_y *= sc[Geom::Y];
}

namespace {

bool is_item(SPObject const &object) {
    return dynamic_cast<SPItem const *>(&object) != NULL;
}

}

void SPItem::raiseToTop() {
    using Inkscape::Algorithms::find_last_if;

    SPObject *topmost=find_last_if<SPObject::SiblingIterator>(
        next, NULL, &is_item
    );
    if (topmost) {
        getRepr()->parent()->changeOrder( getRepr(), topmost->getRepr() );
    }
}

void SPItem::raiseOne() {
    SPObject *next_higher=std::find_if<SPObject::SiblingIterator>(
        next, NULL, &is_item
    );
    if (next_higher) {
        Inkscape::XML::Node *ref = next_higher->getRepr();
        getRepr()->parent()->changeOrder(getRepr(), ref);
    }
}

void SPItem::lowerOne() {
    using Inkscape::Util::MutableList;
    using Inkscape::Util::reverse_list;

    MutableList<SPObject &> next_lower=std::find_if(
        reverse_list<SPObject::SiblingIterator>(
            parent->firstChild(), this
        ),
        MutableList<SPObject &>(),
        &is_item
    );
    if (next_lower) {
        ++next_lower;
        Inkscape::XML::Node *ref = ( next_lower ? next_lower->getRepr() : NULL );
        getRepr()->parent()->changeOrder(getRepr(), ref);
    }
}

void SPItem::lowerToBottom() {
    using Inkscape::Algorithms::find_last_if;
    using Inkscape::Util::MutableList;
    using Inkscape::Util::reverse_list;

    MutableList<SPObject &> bottom=find_last_if(
        reverse_list<SPObject::SiblingIterator>(
            parent->firstChild(), this
        ),
        MutableList<SPObject &>(),
        &is_item
    );
    if (bottom) {
        ++bottom;
        Inkscape::XML::Node *ref = ( bottom ? bottom->getRepr() : NULL );
        getRepr()->parent()->changeOrder(getRepr(), ref);
    }
}

void SPItem::moveTo(SPItem *target, bool intoafter) {

    Inkscape::XML::Node *target_ref = ( target ? target->getRepr() : NULL );
    Inkscape::XML::Node *our_ref = getRepr();
    gboolean first = FALSE;

    if (target_ref == our_ref) {
        // Move to ourself ignore
        return;
    }

    if (!target_ref) {
        // Assume move to the "first" in the top node, find the top node
        target_ref = our_ref;
        while (target_ref->parent() != target_ref->root()) {
            target_ref = target_ref->parent();
        }
        first = TRUE;
    }

    if (intoafter) {
        // Move this inside of the target at the end
        our_ref->parent()->removeChild(our_ref);
        target_ref->addChild(our_ref, NULL);
    } else if (target_ref->parent() != our_ref->parent()) {
        // Change in parent, need to remove and add
        our_ref->parent()->removeChild(our_ref);
        target_ref->parent()->addChild(our_ref, target_ref);
    } else if (!first) {
        // Same parent, just move
        our_ref->parent()->changeOrder(our_ref, target_ref);
    }

    if (first && parent) {
        // If "first" ensure it appears after the defs etc
        lowerToBottom();
        return;
    }
}

void SPItem::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPItem* object = this;

    object->readAttr( "style" );
    object->readAttr( "transform" );
    object->readAttr( "clip-path" );
    object->readAttr( "mask" );
    object->readAttr( "sodipodi:insensitive" );
    object->readAttr( "sodipodi:nonprintable" );
    object->readAttr( "inkscape:transform-center-x" );
    object->readAttr( "inkscape:transform-center-y" );
    object->readAttr( "inkscape:connector-avoid" );
    object->readAttr( "inkscape:connection-points" );
    object->readAttr( "inkscape:highlight-color" );

    SPObject::build(document, repr);
}

void SPItem::release() {
	SPItem* item = this;

    // Note: do this here before the clip_ref is deleted, since calling
    // ensureUpToDate() for triggered routing may reference
    // the deleted clip_ref.
    delete item->avoidRef;

    // we do NOT disconnect from the changed signal of those before deletion.
    // The destructor will call *_ref_changed with NULL as the new value,
    // which will cause the hide() function to be called.
    delete item->clip_ref;
    delete item->mask_ref;

    SPObject::release();

    SPPaintServer *fill_ps = style->getFillPaintServer();
    SPPaintServer *stroke_ps = style->getStrokePaintServer();
    while (item->display) {
        if (fill_ps) {
            fill_ps->hide(item->display->arenaitem->key());
        }
        if (stroke_ps) {
            stroke_ps->hide(item->display->arenaitem->key());
        }
        item->display = sp_item_view_list_remove(item->display, item->display);
    }

    //item->_transformed_signal.~signal();
}

void SPItem::set(unsigned int key, gchar const* value) {
    SPItem *item = this;
    SPItem* object = item;

    switch (key) {
        case SP_ATTR_TRANSFORM: {
            Geom::Affine t;
            if (value && sp_svg_transform_read(value, &t)) {
                item->set_item_transform(t);
            } else {
                item->set_item_transform(Geom::identity());
            }
            break;
        }
        case SP_PROP_CLIP_PATH: {
            gchar *uri = extract_uri(value);
            if (uri) {
                try {
                    item->clip_ref->attach(Inkscape::URI(uri));
                } catch (Inkscape::BadURIException &e) {
                    g_warning("%s", e.what());
                    item->clip_ref->detach();
                }
                g_free(uri);
            } else {
                item->clip_ref->detach();
            }

            break;
        }
        case SP_PROP_MASK: {
            gchar *uri = extract_uri(value);
            if (uri) {
                try {
                    item->mask_ref->attach(Inkscape::URI(uri));
                } catch (Inkscape::BadURIException &e) {
                    g_warning("%s", e.what());
                    item->mask_ref->detach();
                }
                g_free(uri);
            } else {
                item->mask_ref->detach();
            }

            break;
        }
        case SP_ATTR_SODIPODI_INSENSITIVE:
        {
            item->sensitive = !value;
            for (SPItemView *v = item->display; v != NULL; v = v->next) {
                v->arenaitem->setSensitive(item->sensitive);
            }
            break;
        }
        case SP_ATTR_INKSCAPE_HIGHLIGHT_COLOR:
        {
            g_free(item->_highlightColor);
            if (value) {
                item->_highlightColor = g_strdup(value);
            } else {
                item->_highlightColor = NULL;
            }
            break;
        }
        case SP_ATTR_CONNECTOR_AVOID:
            item->avoidRef->setAvoid(value);
            break;
        case SP_ATTR_TRANSFORM_CENTER_X:
            if (value) {
                item->transform_center_x = g_strtod(value, NULL);
            } else {
                item->transform_center_x = 0;
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_ATTR_TRANSFORM_CENTER_Y:
            if (value) {
                item->transform_center_y = g_strtod(value, NULL);
            } else {
                item->transform_center_y = 0;
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
        case SP_PROP_SYSTEM_LANGUAGE:
        case SP_PROP_REQUIRED_FEATURES:
        case SP_PROP_REQUIRED_EXTENSIONS:
            {
                item->resetEvaluated();
                // pass to default handler
            }
        default:
            if (SP_ATTRIBUTE_IS_CSS(key)) {
                sp_style_read_from_object(object->style, object);
                object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            } else {
                SPObject::set(key, value);
            }
            break;
    }
}

void SPItem::clip_ref_changed(SPObject *old_clip, SPObject *clip, SPItem *item)
{
    item->bbox_valid = FALSE; // force a re-evaluation
    if (old_clip) {
        SPItemView *v;
        /* Hide clippath */
        for (v = item->display; v != NULL; v = v->next) {
            SPClipPath *oldPath = dynamic_cast<SPClipPath *>(old_clip);
            g_assert(oldPath != NULL);
            oldPath->hide(v->arenaitem->key());
        }
    }
    SPClipPath *clipPath = dynamic_cast<SPClipPath *>(clip);
    if (clipPath) {
        Geom::OptRect bbox = item->geometricBounds();
        for (SPItemView *v = item->display; v != NULL; v = v->next) {
            if (!v->arenaitem->key()) {
                v->arenaitem->setKey(SPItem::display_key_new(3));
            }
            Inkscape::DrawingItem *ai = clipPath->show(
                                               v->arenaitem->drawing(),
                                               v->arenaitem->key());
            v->arenaitem->setClip(ai);
            clipPath->setBBox(v->arenaitem->key(), bbox);
            clip->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }
    }
}

void SPItem::mask_ref_changed(SPObject *old_mask, SPObject *mask, SPItem *item)
{
    if (old_mask) {
        /* Hide mask */
        for (SPItemView *v = item->display; v != NULL; v = v->next) {
            SPMask *maskItem = dynamic_cast<SPMask *>(old_mask);
            g_assert(maskItem != NULL);
            maskItem->sp_mask_hide(v->arenaitem->key());
        }
    }
    SPMask *maskItem = dynamic_cast<SPMask *>(mask);
    if (maskItem) {
        Geom::OptRect bbox = item->geometricBounds();
        for (SPItemView *v = item->display; v != NULL; v = v->next) {
            if (!v->arenaitem->key()) {
                v->arenaitem->setKey(SPItem::display_key_new(3));
            }
            Inkscape::DrawingItem *ai = maskItem->sp_mask_show(
                                           v->arenaitem->drawing(),
                                           v->arenaitem->key());
            v->arenaitem->setMask(ai);
            maskItem->sp_mask_set_bbox(v->arenaitem->key(), bbox);
            mask->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }
    }
}

void SPItem::fill_ps_ref_changed(SPObject *old_ps, SPObject *ps, SPItem *item) {
    SPPaintServer *old_fill_ps = dynamic_cast<SPPaintServer *>(old_ps);
    if (old_fill_ps) {
        for (SPItemView *v =item->display; v != NULL; v = v->next) {
            old_fill_ps->hide(v->arenaitem->key());
        }
    }

    SPPaintServer *new_fill_ps = dynamic_cast<SPPaintServer *>(ps);
    if (new_fill_ps) {
        Geom::OptRect bbox = item->geometricBounds();
        for (SPItemView *v = item->display; v != NULL; v = v->next) {
            if (!v->arenaitem->key()) {
                v->arenaitem->setKey(SPItem::display_key_new(3));
            }
            Inkscape::DrawingPattern *pi = new_fill_ps->show(
                    v->arenaitem->drawing(), v->arenaitem->key(), bbox);
            v->arenaitem->setFillPattern(pi);
            if (pi) {
                new_fill_ps->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            }
        }
    }
}

void SPItem::stroke_ps_ref_changed(SPObject *old_ps, SPObject *ps, SPItem *item) {
    SPPaintServer *old_stroke_ps = dynamic_cast<SPPaintServer *>(old_ps);
    if (old_stroke_ps) {
        for (SPItemView *v =item->display; v != NULL; v = v->next) {
            old_stroke_ps->hide(v->arenaitem->key());
        }
    }

    SPPaintServer *new_stroke_ps = dynamic_cast<SPPaintServer *>(ps);
    if (new_stroke_ps) {
        Geom::OptRect bbox = item->geometricBounds();
        for (SPItemView *v = item->display; v != NULL; v = v->next) {
            if (!v->arenaitem->key()) {
                v->arenaitem->setKey(SPItem::display_key_new(3));
            }
            Inkscape::DrawingPattern *pi = new_stroke_ps->show(
                    v->arenaitem->drawing(), v->arenaitem->key(), bbox);
            v->arenaitem->setStrokePattern(pi);
            if (pi) {
                new_stroke_ps->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            }
        }
    }
}

void SPItem::update(SPCtx* /*ctx*/, guint flags) {
    SPItem *item = this;
    SPItem* object = item;

//    SPObject::onUpdate(ctx, flags);

    // any of the modifications defined in sp-object.h might change bbox,
    // so we invalidate it unconditionally
    item->bbox_valid = FALSE;

    if (flags & (SP_OBJECT_CHILD_MODIFIED_FLAG | SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG)) {
        if (flags & SP_OBJECT_MODIFIED_FLAG) {
            for (SPItemView *v = item->display; v != NULL; v = v->next) {
                v->arenaitem->setTransform(item->transform);
            }
        }

        SPClipPath *clip_path = item->clip_ref ? item->clip_ref->getObject() : NULL;
        SPMask *mask = item->mask_ref ? item->mask_ref->getObject() : NULL;

        if ( clip_path || mask ) {
            Geom::OptRect bbox = item->geometricBounds();
            if (clip_path) {
                for (SPItemView *v = item->display; v != NULL; v = v->next) {
                    clip_path->setBBox(v->arenaitem->key(), bbox);
                }
            }
            if (mask) {
                for (SPItemView *v = item->display; v != NULL; v = v->next) {
                    mask->sp_mask_set_bbox(v->arenaitem->key(), bbox);
                }
            }
        }

        if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
            for (SPItemView *v = item->display; v != NULL; v = v->next) {
                v->arenaitem->setOpacity(SP_SCALE24_TO_FLOAT(object->style->opacity.value));
                v->arenaitem->setAntialiasing(object->style->shape_rendering.computed != SP_CSS_SHAPE_RENDERING_CRISPEDGES);
                v->arenaitem->setIsolation( object->style->isolation.value );
                v->arenaitem->setBlendMode( object->style->mix_blend_mode.value );
                v->arenaitem->setVisible(!item->isHidden());
            }
        }
    }
    /* Update bounding box in user space, used for filter and objectBoundingBox units */
    if (item->style->filter.set && item->display) {
        Geom::OptRect item_bbox = item->geometricBounds();
        SPItemView *itemview = item->display;
        do {
            if (itemview->arenaitem)
                itemview->arenaitem->setItemBounds(item_bbox);
        } while ( (itemview = itemview->next) );
    }

    // Update libavoid with item geometry (for connector routing).
    if (item->avoidRef)
        item->avoidRef->handleSettingChange();
}

void SPItem::modified(unsigned int /*flags*/)
{
}

Inkscape::XML::Node* SPItem::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    SPItem *item = this;
    SPItem* object = item;

    // in the case of SP_OBJECT_WRITE_BUILD, the item should always be newly created,
    // so we need to add any children from the underlying object to the new repr
    if (flags & SP_OBJECT_WRITE_BUILD) {
        GSList *l = NULL;
        for (SPObject *child = object->firstChild(); child != NULL; child = child->next ) {
            if (dynamic_cast<SPTitle *>(child) || dynamic_cast<SPDesc *>(child)) {
                Inkscape::XML::Node *crepr = child->updateRepr(xml_doc, NULL, flags);
                if (crepr) {
                    l = g_slist_prepend (l, crepr);
                }
            }
        }
        while (l) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove (l, l->data);
        }
    } else {
        for (SPObject *child = object->firstChild() ; child != NULL; child = child->next ) {
            if (dynamic_cast<SPTitle *>(child) || dynamic_cast<SPDesc *>(child)) {
                child->updateRepr(flags);
            }
        }
    }

    gchar *c = sp_svg_transform_write(item->transform);
    repr->setAttribute("transform", c);
    g_free(c);

    if (flags & SP_OBJECT_WRITE_EXT) {
        repr->setAttribute("sodipodi:insensitive", ( item->sensitive ? NULL : "true" ));
        if (item->transform_center_x != 0)
            sp_repr_set_svg_double (repr, "inkscape:transform-center-x", item->transform_center_x);
        else
            repr->setAttribute ("inkscape:transform-center-x", NULL);
        if (item->transform_center_y != 0)
            sp_repr_set_svg_double (repr, "inkscape:transform-center-y", item->transform_center_y);
        else
            repr->setAttribute ("inkscape:transform-center-y", NULL);
    }

    if (item->clip_ref){
        if (item->clip_ref->getObject()) {
            gchar *uri = item->clip_ref->getURI()->toString();
            const gchar *value = g_strdup_printf ("url(%s)", uri);
            repr->setAttribute ("clip-path", value);
            g_free ((void *) value);
            g_free ((void *) uri);
        }
    }
    if (item->mask_ref){
        if (item->mask_ref->getObject()) {
            gchar *uri = item->mask_ref->getURI()->toString();
            const gchar *value = g_strdup_printf ("url(%s)", uri);
            repr->setAttribute ("mask", value);
            g_free ((void *) value);
            g_free ((void *) uri);
        }
    }
    if (item->_highlightColor){
        repr->setAttribute("inkscape:highlight-color", item->_highlightColor);
    } else {
        repr->setAttribute("inkscape:highlight-color", NULL);
    }

    SPObject::write(xml_doc, repr, flags);

    return repr;
}

// CPPIFY: make pure virtual
Geom::OptRect SPItem::bbox(Geom::Affine const & /*transform*/, SPItem::BBoxType /*type*/) const {
	//throw;
	return Geom::OptRect();
}

Geom::OptRect SPItem::geometricBounds(Geom::Affine const &transform) const
{
    Geom::OptRect bbox;

    // call the subclass method
    // CPPIFY
    //bbox = this->bbox(transform, SPItem::GEOMETRIC_BBOX);
    bbox = const_cast<SPItem*>(this)->bbox(transform, SPItem::GEOMETRIC_BBOX);

    return bbox;
}

Geom::OptRect SPItem::visualBounds(Geom::Affine const &transform) const
{
    using Geom::X;
    using Geom::Y;

    Geom::OptRect bbox;


    SPFilter *filter = (style && style->filter.href) ? dynamic_cast<SPFilter *>(style->getFilter()) : NULL;
    if ( filter ) {
        // call the subclass method
    	// CPPIFY
    	//bbox = this->bbox(Geom::identity(), SPItem::VISUAL_BBOX);
    	bbox = const_cast<SPItem*>(this)->bbox(Geom::identity(), SPItem::GEOMETRIC_BBOX); // see LP Bug 1229971

        // default filer area per the SVG spec:
        SVGLength x, y, w, h;
        Geom::Point minp, maxp;
        x.set(SVGLength::PERCENT, -0.10, 0);
        y.set(SVGLength::PERCENT, -0.10, 0);
        w.set(SVGLength::PERCENT, 1.20, 0);
        h.set(SVGLength::PERCENT, 1.20, 0);

        // if area is explicitly set, override:
        if (filter->x._set)
            x = filter->x;
        if (filter->y._set)
            y = filter->y;
        if (filter->width._set)
            w = filter->width;
        if (filter->height._set)
            h = filter->height;

        double len_x = bbox ? bbox->width() : 0;
        double len_y = bbox ? bbox->height() : 0;
        
        x.update(12, 6, len_x);
        y.update(12, 6, len_y);
        w.update(12, 6, len_x);
        h.update(12, 6, len_y);

        if (filter->filterUnits == SP_FILTER_UNITS_OBJECTBOUNDINGBOX && bbox) {
            minp[X] = bbox->left() + x.computed * (x.unit == SVGLength::PERCENT ? 1.0 : len_x);
            maxp[X] = minp[X] + w.computed * (w.unit == SVGLength::PERCENT ? 1.0 : len_x);
            minp[Y] = bbox->top() + y.computed * (y.unit == SVGLength::PERCENT ? 1.0 : len_y);
            maxp[Y] = minp[Y] + h.computed * (h.unit == SVGLength::PERCENT ? 1.0 : len_y);
        } else if (filter->filterUnits == SP_FILTER_UNITS_USERSPACEONUSE) {
            minp[X] = x.computed;
            maxp[X] = minp[X] + w.computed;
            minp[Y] = y.computed;
            maxp[Y] = minp[Y] + h.computed;
        }
        bbox = Geom::OptRect(minp, maxp);
        *bbox *= transform;
    } else {
        // call the subclass method
    	// CPPIFY
    	//bbox = this->bbox(transform, SPItem::VISUAL_BBOX);
    	bbox = const_cast<SPItem*>(this)->bbox(transform, SPItem::VISUAL_BBOX);
    }
    if (clip_ref->getObject()) {
        SPItem *ownerItem = dynamic_cast<SPItem *>(clip_ref->getOwner());
        g_assert(ownerItem != NULL);
        ownerItem->bbox_valid = FALSE;  // LP Bug 1349018
        bbox.intersectWith(clip_ref->getObject()->geometricBounds(transform));
    }

    return bbox;
}

Geom::OptRect SPItem::bounds(BBoxType type, Geom::Affine const &transform) const
{
    if (type == GEOMETRIC_BBOX) {
        return geometricBounds(transform);
    } else {
        return visualBounds(transform);
    }
}

Geom::OptRect SPItem::documentGeometricBounds() const
{
    return geometricBounds(i2doc_affine());
}

Geom::OptRect SPItem::documentVisualBounds() const
{
    if (!bbox_valid) {
        doc_bbox = visualBounds(i2doc_affine());
        bbox_valid = true;
    }
    return doc_bbox;
}
Geom::OptRect SPItem::documentBounds(BBoxType type) const
{
    if (type == GEOMETRIC_BBOX) {
        return documentGeometricBounds();
    } else {
        return documentVisualBounds();
    }
}

Geom::OptRect SPItem::desktopGeometricBounds() const
{
    return geometricBounds(i2dt_affine());
}

Geom::OptRect SPItem::desktopVisualBounds() const
{
    /// @fixme hardcoded desktop transform
    Geom::Affine m = Geom::Scale(1, -1) * Geom::Translate(0, document->getHeight().value("px"));
    Geom::OptRect ret = documentVisualBounds();
    if (ret) *ret *= m;
    return ret;
}

Geom::OptRect SPItem::desktopPreferredBounds() const
{
    if (Inkscape::Preferences::get()->getInt("/tools/bounding_box") == 0) {
        return desktopBounds(SPItem::VISUAL_BBOX);
    } else {
        return desktopBounds(SPItem::GEOMETRIC_BBOX);
    }
}

Geom::OptRect SPItem::desktopBounds(BBoxType type) const
{
    if (type == GEOMETRIC_BBOX) {
        return desktopGeometricBounds();
    } else {
        return desktopVisualBounds();
    }
}

unsigned int SPItem::pos_in_parent() const {
    g_assert(parent != NULL);
    g_assert(SP_IS_OBJECT(parent));

    unsigned int pos = 0;

    for ( SPObject *iter = parent->firstChild() ; iter ; iter = iter->next) {
        if (iter == this) {
            return pos;
        }

        if (dynamic_cast<SPItem *>(iter)) {
            pos++;
        }
    }

    g_assert_not_reached();
    return 0;
}

// CPPIFY: make pure virtual, see below!
void SPItem::snappoints(std::vector<Inkscape::SnapCandidatePoint> & /*p*/, Inkscape::SnapPreferences const */*snapprefs*/) const {
	//throw;
}
    /* This will only be called if the derived class doesn't override this.
     * see for example sp_genericellipse_snappoints in sp-ellipse.cpp
     * We don't know what shape we could be dealing with here, so we'll just
     * do nothing
     */

void SPItem::getSnappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const
{
    // Get the snappoints of the item
	// CPPIFY
	//this->snappoints(p, snapprefs);
	const_cast<SPItem*>(this)->snappoints(p, snapprefs);

    // Get the snappoints at the item's center
    if (snapprefs != NULL && snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_ROTATION_CENTER)) {
        p.push_back(Inkscape::SnapCandidatePoint(getCenter(), Inkscape::SNAPSOURCE_ROTATION_CENTER, Inkscape::SNAPTARGET_ROTATION_CENTER));
    }

    // Get the snappoints of clipping paths and mask, if any
    std::list<SPObject const *> clips_and_masks;

    clips_and_masks.push_back(clip_ref->getObject());
    clips_and_masks.push_back(mask_ref->getObject());

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    for (std::list<SPObject const *>::const_iterator o = clips_and_masks.begin(); o != clips_and_masks.end(); ++o) {
        if (*o) {
            // obj is a group object, the children are the actual clippers
            for (SPObject *child = (*o)->children ; child ; child = child->next) {
                SPItem *item = dynamic_cast<SPItem *>(child);
                if (item) {
                    std::vector<Inkscape::SnapCandidatePoint> p_clip_or_mask;
                    // Please note the recursive call here!
                    item->getSnappoints(p_clip_or_mask, snapprefs);
                    // Take into account the transformation of the item being clipped or masked
                    for (std::vector<Inkscape::SnapCandidatePoint>::const_iterator p_orig = p_clip_or_mask.begin(); p_orig != p_clip_or_mask.end(); ++p_orig) {
                        // All snappoints are in desktop coordinates, but the item's transformation is
                        // in document coordinates. Hence the awkward construction below
                        Geom::Point pt = desktop->dt2doc((*p_orig).getPoint()) * i2dt_affine();
                        p.push_back(Inkscape::SnapCandidatePoint(pt, (*p_orig).getSourceType(), (*p_orig).getTargetType()));
                    }
                }
            }
        }
    }
}

// CPPIFY: make pure virtual
void SPItem::print(SPPrintContext* /*ctx*/) {
	//throw;
}

void SPItem::invoke_print(SPPrintContext *ctx)
{
    if ( !isHidden() ) {
    	if (!transform.isIdentity() || style->opacity.value != SP_SCALE24_MAX) {
			sp_print_bind(ctx, transform, SP_SCALE24_TO_FLOAT(style->opacity.value));
			this->print(ctx);
			sp_print_release(ctx);
    	} else {
    		this->print(ctx);
    	}
    }
}

const char* SPItem::displayName() const {
    return _("Object");
}

gchar* SPItem::description() const {
    return g_strdup("");
}

gchar *SPItem::detailedDescription() const {
        gchar* s = g_strdup_printf("<b>%s</b> %s",
                    this->displayName(), this->description());

	if (s && clip_ref->getObject()) {
		gchar *snew = g_strdup_printf (_("%s; <i>clipped</i>"), s);
		g_free (s);
		s = snew;
	}

	if (s && mask_ref->getObject()) {
		gchar *snew = g_strdup_printf (_("%s; <i>masked</i>"), s);
		g_free (s);
		s = snew;
	}

	if ( style && style->filter.href && style->filter.href->getObject() ) {
		const gchar *label = style->filter.href->getObject()->label();
		gchar *snew = 0;

		if (label) {
			snew = g_strdup_printf (_("%s; <i>filtered (%s)</i>"), s, _(label));
		} else {
			snew = g_strdup_printf (_("%s; <i>filtered</i>"), s);
		}

		g_free (s);
		s = snew;
	}

	return s;
}

bool SPItem::isFiltered() const {
	return (style && style->filter.href && style->filter.href->getObject());
}

unsigned SPItem::display_key_new(unsigned numkeys)
{
    static unsigned dkey = 0;

    dkey += numkeys;

    return dkey - numkeys;
}

// CPPIFY: make pure virtual
Inkscape::DrawingItem* SPItem::show(Inkscape::Drawing& /*drawing*/, unsigned int /*key*/, unsigned int /*flags*/) {
	//throw;
	return 0;
}

Inkscape::DrawingItem *SPItem::invoke_show(Inkscape::Drawing &drawing, unsigned key, unsigned flags)
{
    Inkscape::DrawingItem *ai = NULL;

    ai = this->show(drawing, key, flags);

    if (ai != NULL) {
        Geom::OptRect item_bbox = geometricBounds();

        display = sp_item_view_new_prepend(display, this, flags, key, ai);
        ai->setTransform(transform);
        ai->setOpacity(SP_SCALE24_TO_FLOAT(style->opacity.value));
        ai->setIsolation( style->isolation.value );
        ai->setBlendMode( style->mix_blend_mode.value );
        //ai->setCompositeOperator( style->composite_op.value );
        ai->setVisible(!isHidden());
        ai->setSensitive(sensitive);
        if (clip_ref->getObject()) {
            SPClipPath *cp = clip_ref->getObject();

            if (!display->arenaitem->key()) {
                display->arenaitem->setKey(display_key_new(3));
            }
            int clip_key = display->arenaitem->key();

            // Show and set clip
            Inkscape::DrawingItem *ac = cp->show(drawing, clip_key);
            ai->setClip(ac);

            // Update bbox, in case the clip uses bbox units
            cp->setBBox(clip_key, item_bbox);
            cp->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }
        if (mask_ref->getObject()) {
            SPMask *mask = mask_ref->getObject();

            if (!display->arenaitem->key()) {
                display->arenaitem->setKey(display_key_new(3));
            }
            int mask_key = display->arenaitem->key();

            // Show and set mask
            Inkscape::DrawingItem *ac = mask->sp_mask_show(drawing, mask_key);
            ai->setMask(ac);

            // Update bbox, in case the mask uses bbox units
            mask->sp_mask_set_bbox(mask_key, item_bbox);
            mask->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }

        SPPaintServer *fill_ps = style->getFillPaintServer();
        if (fill_ps) {
            if (!display->arenaitem->key()) {
                display->arenaitem->setKey(display_key_new(3));
            }
            int fill_key = display->arenaitem->key();

            Inkscape::DrawingPattern *ap = fill_ps->show(drawing, fill_key, item_bbox);
            ai->setFillPattern(ap);
            if (ap) {
                fill_ps->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            }
        }
        SPPaintServer *stroke_ps = style->getStrokePaintServer();
        if (stroke_ps) {
            if (!display->arenaitem->key()) {
                display->arenaitem->setKey(display_key_new(3));
            }
            int stroke_key = display->arenaitem->key();

            Inkscape::DrawingPattern *ap = stroke_ps->show(drawing, stroke_key, item_bbox);
            ai->setStrokePattern(ap);
            if (ap) {
                stroke_ps->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            }
        }
        ai->setData(this);
        ai->setItemBounds(geometricBounds());
    }

    return ai;
}

// CPPIFY: make pure virtual
void SPItem::hide(unsigned int /*key*/) {
	//throw;
}

void SPItem::invoke_hide(unsigned key)
{
	this->hide(key);

    SPItemView *ref = NULL;
    SPItemView *v = display;
    while (v != NULL) {
        SPItemView *next = v->next;
        if (v->key == key) {
            if (clip_ref->getObject()) {
                (clip_ref->getObject())->hide(v->arenaitem->key());
                v->arenaitem->setClip(NULL);
            }
            if (mask_ref->getObject()) {
                mask_ref->getObject()->sp_mask_hide(v->arenaitem->key());
                v->arenaitem->setMask(NULL);
            }
            SPPaintServer *fill_ps = style->getFillPaintServer();
            if (fill_ps) {
                fill_ps->hide(v->arenaitem->key());
            }
            SPPaintServer *stroke_ps = style->getStrokePaintServer();
            if (stroke_ps) {
                stroke_ps->hide(v->arenaitem->key());
            }
            if (!ref) {
                display = v->next;
            } else {
                ref->next = v->next;
            }
            delete v->arenaitem;
            g_free(v);
        } else {
            ref = v;
        }
        v = next;
    }
}

// Adjusters

void SPItem::adjust_pattern(Geom::Affine const &postmul, bool set, PatternTransform pt)
{
    bool fill = (pt == TRANSFORM_FILL || pt == TRANSFORM_BOTH);
    if (fill && style && (style->fill.isPaintserver())) {
        SPObject *server = style->getFillPaintServer();
        SPPattern *serverPatt = dynamic_cast<SPPattern *>(server);
        if ( serverPatt ) {
            SPPattern *pattern = sp_pattern_clone_if_necessary(this, serverPatt, "fill");
            sp_pattern_transform_multiply(pattern, postmul, set);
        }
    }

    bool stroke = (pt == TRANSFORM_STROKE || pt == TRANSFORM_BOTH);
    if (stroke && style && (style->stroke.isPaintserver())) {
        SPObject *server = style->getStrokePaintServer();
        SPPattern *serverPatt = dynamic_cast<SPPattern *>(server);
        if ( serverPatt ) {
            SPPattern *pattern = sp_pattern_clone_if_necessary(this, serverPatt, "stroke");
            sp_pattern_transform_multiply(pattern, postmul, set);
        }
    }
}

void SPItem::adjust_gradient( Geom::Affine const &postmul, bool set )
{
    if ( style && style->fill.isPaintserver() ) {
        SPPaintServer *server = style->getFillPaintServer();
        SPGradient *serverGrad = dynamic_cast<SPGradient *>(server);
        if ( serverGrad ) {

            /**
             * \note Bbox units for a gradient are generally a bad idea because
             * with them, you cannot preserve the relative position of the
             * object and its gradient after rotation or skew. So now we
             * convert them to userspace units which are easy to keep in sync
             * just by adding the object's transform to gradientTransform.
             * \todo FIXME: convert back to bbox units after transforming with
             * the item, so as to preserve the original units.
             */
            SPGradient *gradient = sp_gradient_convert_to_userspace( serverGrad, this, "fill" );

            sp_gradient_transform_multiply( gradient, postmul, set );
        }
    }

    if ( style && style->stroke.isPaintserver() ) {
        SPPaintServer *server = style->getStrokePaintServer();
        SPGradient *serverGrad = dynamic_cast<SPGradient *>(server);
        if ( serverGrad ) {
            SPGradient *gradient = sp_gradient_convert_to_userspace( serverGrad, this, "stroke");
            sp_gradient_transform_multiply( gradient, postmul, set );
        }
    }
}

void SPItem::adjust_stroke( gdouble ex )
{
    if (freeze_stroke_width) {
        return;
    }

    SPStyle *style = this->style;

    if (style && !style->stroke.isNone() && !Geom::are_near(ex, 1.0, Geom::EPSILON)) {
        style->stroke_width.computed *= ex;
        style->stroke_width.set = TRUE;

        if ( !style->stroke_dasharray.values.empty() ) {
            for (unsigned i = 0; i < style->stroke_dasharray.values.size(); i++) {
                style->stroke_dasharray.values[i] *= ex;
            }
            style->stroke_dashoffset.value *= ex;
        }

        updateRepr();
    }
}

/**
 * Find out the inverse of previous transform of an item (from its repr)
 */
static Geom::Affine sp_item_transform_repr (SPItem *item)
{
    Geom::Affine t_old(Geom::identity());
    gchar const *t_attr = item->getRepr()->attribute("transform");
    if (t_attr) {
        Geom::Affine t;
        if (sp_svg_transform_read(t_attr, &t)) {
            t_old = t;
        }
    }

    return t_old;
}


void SPItem::adjust_stroke_width_recursive(double expansion)
{
    adjust_stroke (expansion);

// A clone's child is the ghost of its original - we must not touch it, skip recursion
    if ( !dynamic_cast<SPUse *>(this) ) {
        for ( SPObject *o = children; o; o = o->getNext() ) {
            SPItem *item = dynamic_cast<SPItem *>(o);
            if (item) {
                item->adjust_stroke_width_recursive(expansion);
            }
        }
    }
}

void SPItem::freeze_stroke_width_recursive(bool freeze)
{
    freeze_stroke_width = freeze;

// A clone's child is the ghost of its original - we must not touch it, skip recursion
    if ( !dynamic_cast<SPUse *>(this) ) {
        for ( SPObject *o = children; o; o = o->getNext() ) {
            SPItem *item = dynamic_cast<SPItem *>(o);
            if (item) {
                item->freeze_stroke_width_recursive(freeze);
            }
        }
    }
}

/**
 * Recursively adjust rx and ry of rects.
 */
static void
sp_item_adjust_rects_recursive(SPItem *item, Geom::Affine advertized_transform)
{
    SPRect *rect = dynamic_cast<SPRect *>(item);
    if (rect) {
    	rect->compensateRxRy(advertized_transform);
    }

    for (SPObject *o = item->children; o != NULL; o = o->next) {
        SPItem *item = dynamic_cast<SPItem *>(o);
        if (item) {
            sp_item_adjust_rects_recursive(item, advertized_transform);
        }
    }
}

void SPItem::adjust_paint_recursive (Geom::Affine advertized_transform, Geom::Affine t_ancestors, bool is_pattern)
{
// _Before_ full pattern/gradient transform: t_paint * t_item * t_ancestors
// _After_ full pattern/gradient transform: t_paint_new * t_item * t_ancestors * advertised_transform
// By equating these two expressions we get t_paint_new = t_paint * paint_delta, where:
    Geom::Affine t_item = sp_item_transform_repr (this);
    Geom::Affine paint_delta = t_item * t_ancestors * advertized_transform * t_ancestors.inverse() * t_item.inverse();

// Within text, we do not fork gradients, and so must not recurse to avoid double compensation;
// also we do not recurse into clones, because a clone's child is the ghost of its original -
// we must not touch it
    if (!(this && (dynamic_cast<SPText *>(this) || dynamic_cast<SPUse *>(this)))) {
        for (SPObject *o = children; o != NULL; o = o->next) {
            SPItem *item = dynamic_cast<SPItem *>(o);
            if (item) {
// At the level of the transformed item, t_ancestors is identity;
// below it, it is the accmmulated chain of transforms from this level to the top level
                item->adjust_paint_recursive (advertized_transform, t_item * t_ancestors, is_pattern);
            }
        }
    }

// We recursed into children first, and are now adjusting this object second;
// this is so that adjustments in a tree are done from leaves up to the root,
// and paintservers on leaves inheriting their values from ancestors could adjust themselves properly
// before ancestors themselves are adjusted, probably differently (bug 1286535)

    if (is_pattern) {
        adjust_pattern(paint_delta);
    } else {
        adjust_gradient(paint_delta);
    }
}

void SPItem::adjust_livepatheffect (Geom::Affine const &postmul, bool set)
{
    SPLPEItem *lpeitem = dynamic_cast<SPLPEItem *>(this);
    if ( lpeitem && lpeitem->hasPathEffect() ) {
        lpeitem->forkPathEffectsIfNecessary();

        // now that all LPEs are forked_if_necessary, we can apply the transform
        PathEffectList effect_list =  lpeitem->getEffectList();
        for (PathEffectList::iterator it = effect_list.begin(); it != effect_list.end(); ++it)
        {
            LivePathEffectObject *lpeobj = (*it)->lpeobject;
            if (lpeobj && lpeobj->get_lpe()) {
                Inkscape::LivePathEffect::Effect * effect = lpeobj->get_lpe();
                effect->transform_multiply(postmul, set);
            }
        }
    }
}

// CPPIFY:: make pure virtual?
// Not all SPItems must necessarily have a set transform method!
Geom::Affine SPItem::set_transform(Geom::Affine const &transform) {
//	throw;
	return transform;
}

void SPItem::doWriteTransform(Inkscape::XML::Node *repr, Geom::Affine const &transform, Geom::Affine const *adv, bool compensate)
{
    g_return_if_fail(repr != NULL);

    // calculate the relative transform, if not given by the adv attribute
    Geom::Affine advertized_transform;
    if (adv != NULL) {
        advertized_transform = *adv;
    } else {
        advertized_transform = sp_item_transform_repr (this).inverse() * transform;
    }

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (compensate) {
        // recursively compensating for stroke scaling will not always work, because it can be scaled to zero or infinite
        // from which we cannot ever recover by applying an inverse scale; therefore we temporarily block any changes
        // to the strokewidth in such a case instead, and unblock these after the transformation
        // (as reported in https://bugs.launchpad.net/inkscape/+bug/825840/comments/4)
        if (!prefs->getBool("/options/transform/stroke", true)) {
            double const expansion = 1. / advertized_transform.descrim();
            if (expansion < 1e-9 || expansion > 1e9) {
                freeze_stroke_width_recursive(true);
                // This will only work if the item has a set_transform method (in this method adjust_stroke() will be called)
                // We will still have to apply the inverse scaling to other items, not having a set_transform method
                // such as ellipses and stars
                // PS: We cannot use this freeze_stroke_width_recursive() trick in all circumstances. For example, it will
                // break pasting objects within their group (because in such a case the transformation of the group will affect
                // the strokewidth, and has to be compensated for. See https://bugs.launchpad.net/inkscape/+bug/959223/comments/10)
            } else {
                adjust_stroke_width_recursive(expansion);
            }
        }

        // recursively compensate rx/ry of a rect if requested
        if (!prefs->getBool("/options/transform/rectcorners", true)) {
            sp_item_adjust_rects_recursive(this, advertized_transform);
        }

        // recursively compensate pattern fill if it's not to be transformed
        if (!prefs->getBool("/options/transform/pattern", true)) {
            adjust_paint_recursive (advertized_transform.inverse(), Geom::identity(), true);
        }
        /// \todo FIXME: add the same else branch as for gradients below, to convert patterns to userSpaceOnUse as well
        /// recursively compensate gradient fill if it's not to be transformed
        if (!prefs->getBool("/options/transform/gradient", true)) {
            adjust_paint_recursive (advertized_transform.inverse(), Geom::identity(), false);
        } else {
            // this converts the gradient/pattern fill/stroke, if any, to userSpaceOnUse; we need to do
            // it here _before_ the new transform is set, so as to use the pre-transform bbox
            adjust_paint_recursive (Geom::identity(), Geom::identity(), false);
        }

    } // endif(compensate)

    gint preserve = prefs->getBool("/options/preservetransform/value", 0);
    Geom::Affine transform_attr (transform);

    // CPPIFY: check this code.
    // If onSetTransform is not overridden, CItem::onSetTransform will return the transform it was given as a parameter.
    // onSetTransform cannot be pure due to the fact that not all visible Items are transformable.

    if ( // run the object's set_transform (i.e. embed transform) only if:
        (dynamic_cast<SPText *>(this) && firstChild() && dynamic_cast<SPTextPath *>(firstChild())) ||
             (!preserve && // user did not chose to preserve all transforms
             (!clip_ref || !clip_ref->getObject()) && // the object does not have a clippath
             (!mask_ref || !mask_ref->getObject()) && // the object does not have a mask
             !(!transform.isTranslation() && style && style->getFilter())) // the object does not have a filter, or the transform is translation (which is supposed to not affect filters)
        )
    {
        transform_attr = this->set_transform(transform);

        if (freeze_stroke_width) {
            freeze_stroke_width_recursive(false);
        }
    } else {
        if (freeze_stroke_width) {
            freeze_stroke_width_recursive(false);
            if (compensate) {
                if (!prefs->getBool("/options/transform/stroke", true)) {
                    // Recursively compensate for stroke scaling, depending on user preference
                    // (As to why we need to do this, see the comment a few lines above near the freeze_stroke_width_recursive(true) call)
                    double const expansion = 1. / advertized_transform.descrim();
                    adjust_stroke_width_recursive(expansion);
                }
            }
        }
    }
    set_item_transform(transform_attr);



    // Note: updateRepr comes before emitting the transformed signal since
    // it causes clone SPUse's copy of the original object to brought up to
    // date with the original.  Otherwise, sp_use_bbox returns incorrect
    // values if called in code handling the transformed signal.
    updateRepr();

    // send the relative transform with a _transformed_signal
    _transformed_signal.emit(&advertized_transform, this);
}

// CPPIFY: see below, do not make pure?
gint SPItem::event(SPEvent* /*event*/) {
	return FALSE;
}

gint SPItem::emitEvent(SPEvent &event)
{
	return this->event(&event);
}

void SPItem::set_item_transform(Geom::Affine const &transform_matrix)
{
    if (!Geom::are_near(transform_matrix, transform, 1e-18)) {
        transform = transform_matrix;
        /* The SP_OBJECT_USER_MODIFIED_FLAG_B is used to mark the fact that it's only a
           transformation.  It's apparently not used anywhere else. */
        requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_USER_MODIFIED_FLAG_B);
        sp_item_rm_unsatisfied_cns(*this);
    }
}

//void SPItem::convert_to_guides() const {
//	// CPPIFY: If not overridden, call SPItem::convert_to_guides() const, see below!
//	this->convert_to_guides();
//}


Geom::Affine i2anc_affine(SPObject const *object, SPObject const *const ancestor) {
    Geom::Affine ret(Geom::identity());
    g_return_val_if_fail(object != NULL, ret);

    /* stop at first non-renderable ancestor */
    while ( object != ancestor && dynamic_cast<SPItem const *>(object) ) {
        SPRoot const *root = dynamic_cast<SPRoot const *>(object);
        if (root) {
            ret *= root->c2p;
        } else {
            SPItem const *item = dynamic_cast<SPItem const *>(object);
            g_assert(item != NULL);
            ret *= item->transform;
        }
        object = object->parent;
    }
    return ret;
}

Geom::Affine
i2i_affine(SPObject const *src, SPObject const *dest) {
    g_return_val_if_fail(src != NULL && dest != NULL, Geom::identity());
    SPObject const *ancestor = src->nearestCommonAncestor(dest);
    return i2anc_affine(src, ancestor) * i2anc_affine(dest, ancestor).inverse();
}

Geom::Affine SPItem::getRelativeTransform(SPObject const *dest) const {
    return i2i_affine(this, dest);
}

Geom::Affine SPItem::i2doc_affine() const
{
    return i2anc_affine(this, NULL);
}

Geom::Affine SPItem::i2dt_affine() const
{
    Geom::Affine ret;
    SPDesktop const *desktop = SP_ACTIVE_DESKTOP;
    if ( desktop ) {
        ret = i2doc_affine() * desktop->doc2dt();
    } else {
        // TODO temp code to prevent crashing on command-line launch:
        ret = i2doc_affine()
            * Geom::Scale(1, -1)
            * Geom::Translate(0, document->getHeight().value("px"));
    }
    return ret;
}

void SPItem::set_i2d_affine(Geom::Affine const &i2dt)
{
    Geom::Affine dt2p; /* desktop to item parent transform */
    if (parent) {
        dt2p = static_cast<SPItem *>(parent)->i2dt_affine().inverse();
    } else {
        SPDesktop *dt = SP_ACTIVE_DESKTOP;
        dt2p = dt->dt2doc();
    }

    Geom::Affine const i2p( i2dt * dt2p );
    set_item_transform(i2p);
}


Geom::Affine SPItem::dt2i_affine() const
{
    /* fixme: Implement the right way (Lauris) */
    return i2dt_affine().inverse();
}

/* Item views */

SPItemView *SPItem::sp_item_view_new_prepend(SPItemView *list, SPItem *item, unsigned flags, unsigned key, Inkscape::DrawingItem *drawing_item)
{
    g_assert(item != NULL);
    g_assert(dynamic_cast<SPItem *>(item) != NULL);
    g_assert(drawing_item != NULL);

    SPItemView *new_view = g_new(SPItemView, 1);

    new_view->next = list;
    new_view->flags = flags;
    new_view->key = key;
    new_view->arenaitem = drawing_item;

    return new_view;
}

static SPItemView*
sp_item_view_list_remove(SPItemView *list, SPItemView *view)
{
    SPItemView *ret = list;
    if (view == list) {
        ret = list->next;
    } else {
        SPItemView *prev;
        prev = list;
        while (prev->next != view) prev = prev->next;
        prev->next = view->next;
    }

    delete view->arenaitem;
    g_free(view);

    return ret;
}

Inkscape::DrawingItem *SPItem::get_arenaitem(unsigned key)
{
    for ( SPItemView *iv = display ; iv ; iv = iv->next ) {
        if ( iv->key == key ) {
            return iv->arenaitem;
        }
    }

    return NULL;
}

int sp_item_repr_compare_position(SPItem const *first, SPItem const *second)
{
    return sp_repr_compare_position(first->getRepr(),
                                    second->getRepr());
}

SPItem const *sp_item_first_item_child(SPObject const *obj)
{
    return sp_item_first_item_child( const_cast<SPObject *>(obj) );
}

SPItem *sp_item_first_item_child(SPObject *obj)
{
    SPItem *child = 0;
    for ( SPObject *iter = obj->firstChild() ; iter ; iter = iter->next ) {
        SPItem *tmp = dynamic_cast<SPItem *>(iter);
        if ( tmp ) {
            child = tmp;
            break;
        }
    }
    return child;
}

void SPItem::convert_to_guides() const {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int prefs_bbox = prefs->getInt("/tools/bounding_box", 0);

    Geom::OptRect bbox = (prefs_bbox == 0) ? desktopVisualBounds() : desktopGeometricBounds();
    if (!bbox) {
        g_warning ("Cannot determine item's bounding box during conversion to guides.\n");
        return;
    }

    std::list<std::pair<Geom::Point, Geom::Point> > pts;

    Geom::Point A((*bbox).min());
    Geom::Point C((*bbox).max());
    Geom::Point B(A[Geom::X], C[Geom::Y]);
    Geom::Point D(C[Geom::X], A[Geom::Y]);

    pts.push_back(std::make_pair(A, B));
    pts.push_back(std::make_pair(B, C));
    pts.push_back(std::make_pair(C, D));
    pts.push_back(std::make_pair(D, A));

    sp_guide_pt_pairs_to_guides(document, pts);
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

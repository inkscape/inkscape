/** \file
 * Base class for visual SVG elements
 */
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

/** \class SPItem
 *
 * SPItem is an abstract base class for all graphic (visible) SVG nodes. It
 * is a subclass of SPObject, with great deal of specific functionality.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include "sp-item.h"
#include "svg/svg.h"
#include "print.h"
#include "display/nr-arena.h"
#include "display/nr-arena-item.h"
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

#include "libnr/nr-convert2geom.h"
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

#define noSP_ITEM_DEBUG_IDLE

SPObjectClass * SPItemClass::static_parent_class=0;

/**
 * Registers SPItem class and returns its type number.
 */
GType
SPItem::getType(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPItemClass),
            NULL, NULL,
            (GClassInitFunc) SPItemClass::sp_item_class_init,
            NULL, NULL,
            sizeof(SPItem),
            16,
            (GInstanceInitFunc) sp_item_init,
            NULL,   /* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECT, "SPItem", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * SPItem vtable initialization.
 */
void
SPItemClass::sp_item_class_init(SPItemClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    static_parent_class = (SPObjectClass *)g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->build = SPItem::sp_item_build;
    sp_object_class->release = SPItem::sp_item_release;
    sp_object_class->set = SPItem::sp_item_set;
    sp_object_class->update = SPItem::sp_item_update;
    sp_object_class->write = SPItem::sp_item_write;

    klass->description = SPItem::sp_item_private_description;
    klass->snappoints = SPItem::sp_item_private_snappoints;
}

/**
 * Callback for SPItem object initialization.
 */
void SPItem::sp_item_init(SPItem *item)
{
    item->init();
}

void SPItem::init() {
    sensitive = TRUE;

    transform_center_x = 0;
    transform_center_y = 0;

    _is_evaluated = true;
    _evaluated_status = StatusUnknown;

    transform = Geom::identity();

    display = NULL;

    clip_ref = new SPClipPathReference(this);
    sigc::signal<void, SPObject *, SPObject *> cs1 = clip_ref->changedSignal();
    sigc::slot2<void,SPObject*, SPObject *> sl1 = sigc::bind(sigc::ptr_fun(clip_ref_changed), this);
    _clip_ref_connection = cs1.connect(sl1);

    mask_ref = new SPMaskReference(this);
    sigc::signal<void, SPObject *, SPObject *> cs2 = mask_ref->changedSignal();
    sigc::slot2<void,SPObject*, SPObject *> sl2=sigc::bind(sigc::ptr_fun(mask_ref_changed), this);
    _mask_ref_connection = cs2.connect(sl2);

    avoidRef = new SPAvoidRef(this);

    new (&constraints) std::vector<SPGuideConstraint>();

    new (&_transformed_signal) sigc::signal<void, Geom::Affine const *, SPItem *>();
}

bool SPItem::isVisibleAndUnlocked() const {
    return (!isHidden() && !isLocked());
}

bool SPItem::isVisibleAndUnlocked(unsigned display_key) const {
    return (!isHidden(display_key) && !isLocked());
}

bool SPItem::isLocked() const {
    for (SPObject const *o = this; o != NULL; o = o->parent) {
        if (SP_IS_ITEM(o) && !(SP_ITEM(o)->sensitive)) {
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
            for ( NRArenaItem *arenaitem = view->arenaitem ;
                  arenaitem ; arenaitem = arenaitem->parent )
            {
                if (!arenaitem->visible) {
                    return true;
                }
            }
            return false;
        }
    }
    return true;
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
        if (SP_IS_SWITCH(parent)) {
            SP_SWITCH(parent)->resetChildEvaluated();
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

/**
 * Returns something suitable for the `Hide' checkbox in the Object Properties dialog box.
 *  Corresponds to setExplicitlyHidden.
 */
bool SPItem::isExplicitlyHidden() const
{
    return (style->display.set
            && style->display.value == SP_CSS_DISPLAY_NONE);
}

/**
 * Sets the display CSS property to `hidden' if \a val is true,
 * otherwise makes it unset
 */
void SPItem::setExplicitlyHidden(bool const val) {
    style->display.set = val;
    style->display.value = ( val ? SP_CSS_DISPLAY_NONE : SP_CSS_DISPLAY_INLINE );
    style->display.computed = style->display.value;
    updateRepr();
}

/**
 * Sets the transform_center_x and transform_center_y properties to retain the rotation centre
 */
void SPItem::setCenter(Geom::Point object_centre) {
    // for getBounds() to work
    document->ensureUpToDate();

    Geom::OptRect bbox = getBounds(i2dt_affine());
    if (bbox) {
        transform_center_x = object_centre[Geom::X] - bbox->midpoint()[Geom::X];
        if (fabs(transform_center_x) < 1e-5) // rounding error
            transform_center_x = 0;
        transform_center_y = object_centre[Geom::Y] - bbox->midpoint()[Geom::Y];
        if (fabs(transform_center_y) < 1e-5) // rounding error
            transform_center_y = 0;
    }
}

void
SPItem::unsetCenter() {
    transform_center_x = 0;
    transform_center_y = 0;
}

bool SPItem::isCenterSet() {
    return (transform_center_x != 0 || transform_center_y != 0);
}

Geom::Point SPItem::getCenter() const {
    // for getBounds() to work
    document->ensureUpToDate();

    Geom::OptRect bbox = getBounds(i2dt_affine());
    if (bbox) {
        return bbox->midpoint() + Geom::Point (transform_center_x, transform_center_y);
    } else {
        return Geom::Point(0, 0); // something's wrong!
    }
}


namespace {

bool is_item(SPObject const &object) {
    return SP_IS_ITEM(&object);
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

void SPItem::sp_item_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
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

    if (((SPObjectClass *) (SPItemClass::static_parent_class))->build) {
        (* ((SPObjectClass *) (SPItemClass::static_parent_class))->build)(object, document, repr);
    }
}

void SPItem::sp_item_release(SPObject *object)
{
    SPItem *item = (SPItem *) object;

    item->_clip_ref_connection.disconnect();
    item->_mask_ref_connection.disconnect();

    // Note: do this here before the clip_ref is deleted, since calling
    // ensureUpToDate() for triggered routing may reference
    // the deleted clip_ref.
    if (item->avoidRef) {
        delete item->avoidRef;
        item->avoidRef = NULL;
    }

    if (item->clip_ref) {
        item->clip_ref->detach();
        delete item->clip_ref;
        item->clip_ref = NULL;
    }

    if (item->mask_ref) {
        item->mask_ref->detach();
        delete item->mask_ref;
        item->mask_ref = NULL;
    }

    if (((SPObjectClass *) (SPItemClass::static_parent_class))->release) {
        ((SPObjectClass *) SPItemClass::static_parent_class)->release(object);
    }

    while (item->display) {
        nr_arena_item_unparent(item->display->arenaitem);
        item->display = sp_item_view_list_remove(item->display, item->display);
    }

    item->_transformed_signal.~signal();
}

void SPItem::sp_item_set(SPObject *object, unsigned key, gchar const *value)
{
    SPItem *item = (SPItem *) object;

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
            item->sensitive = !value;
            for (SPItemView *v = item->display; v != NULL; v = v->next) {
                nr_arena_item_set_sensitive(v->arenaitem, item->sensitive);
            }
            break;
        case SP_ATTR_CONNECTOR_AVOID:
            item->avoidRef->setAvoid(value);
            break;
        case SP_ATTR_CONNECTION_POINTS:
            item->avoidRef->setConnectionPoints(value);
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
                if (((SPObjectClass *) (SPItemClass::static_parent_class))->set) {
                    (* ((SPObjectClass *) (SPItemClass::static_parent_class))->set)(object, key, value);
                }
            }
            break;
    }
}

void SPItem::clip_ref_changed(SPObject *old_clip, SPObject *clip, SPItem *item)
{
    if (old_clip) {
        SPItemView *v;
        /* Hide clippath */
        for (v = item->display; v != NULL; v = v->next) {
            SP_CLIPPATH(old_clip)->hide(NR_ARENA_ITEM_GET_KEY(v->arenaitem));
            nr_arena_item_set_clip(v->arenaitem, NULL);
        }
    }
    if (SP_IS_CLIPPATH(clip)) {
        NRRect bbox;
        item->invoke_bbox( &bbox, Geom::identity(), TRUE);
        for (SPItemView *v = item->display; v != NULL; v = v->next) {
            if (!v->arenaitem->key) {
                NR_ARENA_ITEM_SET_KEY(v->arenaitem, SPItem::display_key_new(3));
            }
            NRArenaItem *ai = SP_CLIPPATH(clip)->show(
                                               NR_ARENA_ITEM_ARENA(v->arenaitem),
                                               NR_ARENA_ITEM_GET_KEY(v->arenaitem));
            nr_arena_item_set_clip(v->arenaitem, ai);
            nr_arena_item_unref(ai);
            SP_CLIPPATH(clip)->setBBox(NR_ARENA_ITEM_GET_KEY(v->arenaitem), &bbox);
            clip->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }
    }
}

void SPItem::mask_ref_changed(SPObject *old_mask, SPObject *mask, SPItem *item)
{
    if (old_mask) {
        /* Hide mask */
        for (SPItemView *v = item->display; v != NULL; v = v->next) {
            sp_mask_hide(SP_MASK(old_mask), NR_ARENA_ITEM_GET_KEY(v->arenaitem));
            nr_arena_item_set_mask(v->arenaitem, NULL);
        }
    }
    if (SP_IS_MASK(mask)) {
        NRRect bbox;
        item->invoke_bbox( &bbox, Geom::identity(), TRUE);
        for (SPItemView *v = item->display; v != NULL; v = v->next) {
            if (!v->arenaitem->key) {
                NR_ARENA_ITEM_SET_KEY(v->arenaitem, SPItem::display_key_new(3));
            }
            NRArenaItem *ai = sp_mask_show(SP_MASK(mask),
                                           NR_ARENA_ITEM_ARENA(v->arenaitem),
                                           NR_ARENA_ITEM_GET_KEY(v->arenaitem));
            nr_arena_item_set_mask(v->arenaitem, ai);
            nr_arena_item_unref(ai);
            sp_mask_set_bbox(SP_MASK(mask), NR_ARENA_ITEM_GET_KEY(v->arenaitem), &bbox);
            mask->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }
    }
}

void SPItem::sp_item_update(SPObject *object, SPCtx *ctx, guint flags)
{
    SPItem *item = SP_ITEM(object);

    if (((SPObjectClass *) (SPItemClass::static_parent_class))->update) {
        (* ((SPObjectClass *) (SPItemClass::static_parent_class))->update)(object, ctx, flags);
    }

    if (flags & (SP_OBJECT_CHILD_MODIFIED_FLAG | SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG)) {
        if (flags & SP_OBJECT_MODIFIED_FLAG) {
            for (SPItemView *v = item->display; v != NULL; v = v->next) {
                nr_arena_item_set_transform(v->arenaitem, item->transform);
            }
        }

        SPClipPath *clip_path = item->clip_ref ? item->clip_ref->getObject() : NULL;
        SPMask *mask = item->mask_ref ? item->mask_ref->getObject() : NULL;

        if ( clip_path || mask ) {
            NRRect bbox;
            item->invoke_bbox( &bbox, Geom::identity(), TRUE);
            if (clip_path) {
                for (SPItemView *v = item->display; v != NULL; v = v->next) {
                    clip_path->setBBox(NR_ARENA_ITEM_GET_KEY(v->arenaitem), &bbox);
                }
            }
            if (mask) {
                for (SPItemView *v = item->display; v != NULL; v = v->next) {
                    sp_mask_set_bbox(mask, NR_ARENA_ITEM_GET_KEY(v->arenaitem), &bbox);
                }
            }
        }

        if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
            for (SPItemView *v = item->display; v != NULL; v = v->next) {
                nr_arena_item_set_opacity(v->arenaitem, SP_SCALE24_TO_FLOAT(object->style->opacity.value));
                nr_arena_item_set_visible(v->arenaitem, !item->isHidden());
            }
        }
    }

    /* Update bounding box data used by filters */
    if (item->style->filter.set && item->display) {
        Geom::OptRect item_bbox;
        item->invoke_bbox( item_bbox, Geom::identity(), TRUE, SPItem::GEOMETRIC_BBOX);

        SPItemView *itemview = item->display;
        do {
            if (itemview->arenaitem)
                nr_arena_item_set_item_bbox(itemview->arenaitem, item_bbox);
        } while ( (itemview = itemview->next) );
    }

    // Update libavoid with item geometry (for connector routing).
    if (item->avoidRef)
        item->avoidRef->handleSettingChange();
}

Inkscape::XML::Node *SPItem::sp_item_write(SPObject *const object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPItem *item = SP_ITEM(object);

    // in the case of SP_OBJECT_WRITE_BUILD, the item should always be newly created,
    // so we need to add any children from the underlying object to the new repr
    if (flags & SP_OBJECT_WRITE_BUILD) {
        GSList *l = NULL;
        for (SPObject *child = object->firstChild(); child != NULL; child = child->next ) {
            if (SP_IS_TITLE(child) || SP_IS_DESC(child)) {
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
            if (SP_IS_TITLE(child) || SP_IS_DESC(child)) {
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

    if (item->clip_ref->getObject()) {
        const gchar *value = g_strdup_printf ("url(%s)", item->clip_ref->getURI()->toString());
        repr->setAttribute ("clip-path", value);
        g_free ((void *) value);
    }
    if (item->mask_ref->getObject()) {
        const gchar *value = g_strdup_printf ("url(%s)", item->mask_ref->getURI()->toString());
        repr->setAttribute ("mask", value);
        g_free ((void *) value);
    }

    if (((SPObjectClass *) (SPItemClass::static_parent_class))->write) {
        ((SPObjectClass *) (SPItemClass::static_parent_class))->write(object, xml_doc, repr, flags);
    }

    return repr;
}

/**
 * \return  There is no guarantee that the return value will contain a rectangle.
            If this item does not have a boundingbox, it might well be empty.
 */
Geom::OptRect SPItem::getBounds(Geom::Affine const &transform,
                                      SPItem::BBoxType type,
                                      unsigned int /*dkey*/) const
{
    Geom::OptRect r;
    invoke_bbox_full( r, transform, type, TRUE);
    return r;
}

void SPItem::invoke_bbox( Geom::OptRect &bbox, Geom::Affine const &transform, unsigned const clear, SPItem::BBoxType type)
{
    invoke_bbox_full( bbox, transform, type, clear);
}

// DEPRECATED to phase out the use of NRRect in favor of Geom::OptRect
void SPItem::invoke_bbox( NRRect *bbox, Geom::Affine const &transform, unsigned const clear, SPItem::BBoxType type)
{
    invoke_bbox_full( bbox, transform, type, clear);
}

/** Calls \a item's subclass' bounding box method; clips it by the bbox of clippath, if any; and
 * unions the resulting bbox with \a bbox. If \a clear is true, empties \a bbox first. Passes the
 * transform and the flags to the actual bbox methods. Note that many of subclasses (e.g. groups,
 * clones), in turn, call this function in their bbox methods.
 * \retval bbox  Note that there is no guarantee that bbox will contain a rectangle when the
 *               function returns. If this item does not have a boundingbox, this might well be empty.
 */
void SPItem::invoke_bbox_full( Geom::OptRect &bbox, Geom::Affine const &transform, unsigned const flags, unsigned const clear) const
{
    if (clear) {
        bbox = Geom::OptRect();
    }

    // TODO: replace NRRect by Geom::Rect, for all SPItemClasses, and for SP_CLIPPATH

    NRRect temp_bbox;
    temp_bbox.x0 = temp_bbox.y0 = Geom::infinity();
    temp_bbox.x1 = temp_bbox.y1 = -Geom::infinity();

    // call the subclass method
    if (((SPItemClass *) G_OBJECT_GET_CLASS(this))->bbox) {
        ((SPItemClass *) G_OBJECT_GET_CLASS(this))->bbox(this, &temp_bbox, transform, flags);
    }

    // unless this is geometric bbox, extend by filter area and crop the bbox by clip path, if any
    if ((SPItem::BBoxType) flags != SPItem::GEOMETRIC_BBOX) {
        if ( style && style->filter.href) {
            SPObject *filter = style->getFilter();
            if (filter && SP_IS_FILTER(filter)) {
                // default filer area per the SVG spec:
                double x = -0.1;
                double y = -0.1;
                double w = 1.2;
                double h = 1.2;

                // if area is explicitly set, override:
                if (SP_FILTER(filter)->x._set)
                    x = SP_FILTER(filter)->x.computed;
                if (SP_FILTER(filter)->y._set)
                    y = SP_FILTER(filter)->y.computed;
                if (SP_FILTER(filter)->width._set)
                    w = SP_FILTER(filter)->width.computed;
                if (SP_FILTER(filter)->height._set)
                    h = SP_FILTER(filter)->height.computed;

                double dx0 = 0;
                double dx1 = 0;
                double dy0 = 0;
                double dy1 = 0;
                if (filter_is_single_gaussian_blur(SP_FILTER(filter))) {
                    // if this is a single blur, use 2.4*radius
                    // which may be smaller than the default area;
                    // see set_filter_area for why it's 2.4
                    double r = get_single_gaussian_blur_radius (SP_FILTER(filter));
                    dx0 = -2.4 * r;
                    dx1 = 2.4 * r;
                    dy0 = -2.4 * r;
                    dy1 = 2.4 * r;
                } else {
                    // otherwise, calculate expansion from relative to absolute units:
                    dx0 = x * (temp_bbox.x1 - temp_bbox.x0);
                    dx1 = (w + x - 1) * (temp_bbox.x1 - temp_bbox.x0);
                    dy0 = y * (temp_bbox.y1 - temp_bbox.y0);
                    dy1 = (h + y - 1) * (temp_bbox.y1 - temp_bbox.y0);
                }

                // transform the expansions by the item's transform:
                Geom::Affine i2dt(i2dt_affine ());
                dx0 *= i2dt.expansionX();
                dx1 *= i2dt.expansionX();
                dy0 *= i2dt.expansionY();
                dy1 *= i2dt.expansionY();

                // expand the bbox
                temp_bbox.x0 += dx0;
                temp_bbox.x1 += dx1;
                temp_bbox.y0 += dy0;
                temp_bbox.y1 += dy1;
            }
        }
        if (clip_ref->getObject()) {
            NRRect b;
            SP_CLIPPATH(clip_ref->getObject())->getBBox(&b, transform, flags);
            nr_rect_d_intersect (&temp_bbox, &temp_bbox, &b);
        }
    }

    if (temp_bbox.x0 > temp_bbox.x1 || temp_bbox.y0 > temp_bbox.y1) {
        // Either the bbox hasn't been touched by the SPItemClass' bbox method
        // (it still has its initial values, see above: x0 = y0 = Geom::infinity() and x1 = y1 = -Geom::infinity())
        // or it has explicitely been set to be like this (e.g. in sp_shape_bbox)

        // When x0 > x1 or y0 > y1, the bbox is considered to be "nothing", although it has not been
        // explicitely defined this way for NRRects (as opposed to Geom::OptRect)
        // So union bbox with nothing = do nothing, just return
        return;
    }

    // Do not use temp_bbox.upgrade() here, because it uses a test that returns an empty Geom::OptRect()
    // for any rectangle with zero area. The geometrical bbox of for example a vertical line
    // would therefore be translated into empty Geom::OptRect() (see bug https://bugs.launchpad.net/inkscape/+bug/168684)
    Geom::OptRect temp_bbox_new = Geom::Rect(Geom::Point(temp_bbox.x0, temp_bbox.y0), Geom::Point(temp_bbox.x1, temp_bbox.y1));

    bbox.unionWith(temp_bbox_new);
}

// DEPRECATED to phase out the use of NRRect in favor of Geom::OptRect
/** Calls \a item's subclass' bounding box method; clips it by the bbox of clippath, if any; and
 * unions the resulting bbox with \a bbox. If \a clear is true, empties \a bbox first. Passes the
 * transform and the flags to the actual bbox methods. Note that many of subclasses (e.g. groups,
 * clones), in turn, call this function in their bbox methods. */
void SPItem::invoke_bbox_full( NRRect *bbox, Geom::Affine const &transform, unsigned const flags, unsigned const clear)
{
    g_assert(bbox != NULL);

    if (clear) {
        bbox->x0 = bbox->y0 = 1e18;
        bbox->x1 = bbox->y1 = -1e18;
    }

    NRRect this_bbox;
    this_bbox.x0 = this_bbox.y0 = 1e18;
    this_bbox.x1 = this_bbox.y1 = -1e18;

    // call the subclass method
    if (((SPItemClass *) G_OBJECT_GET_CLASS(this))->bbox) {
        ((SPItemClass *) G_OBJECT_GET_CLASS(this))->bbox(this, &this_bbox, transform, flags);
    }

    // unless this is geometric bbox, crop the bbox by clip path, if any
    if ((SPItem::BBoxType) flags != SPItem::GEOMETRIC_BBOX && clip_ref->getObject()) {
        NRRect b;
        SP_CLIPPATH(clip_ref->getObject())->getBBox(&b, transform, flags);
        nr_rect_d_intersect (&this_bbox, &this_bbox, &b);
    }

    // if non-empty (with some tolerance - ?) union this_bbox with the bbox we've got passed
    if ( fabs(this_bbox.x1-this_bbox.x0) > -0.00001 && fabs(this_bbox.y1-this_bbox.y0) > -0.00001 ) {
        nr_rect_d_union (bbox, bbox, &this_bbox);
    }
}

unsigned SPItem::pos_in_parent()
{
    g_assert(parent != NULL);
    g_assert(SP_IS_OBJECT(parent));

    SPObject *object = this;

    unsigned pos=0;
    for ( SPObject *iter = parent->firstChild() ; iter ; iter = iter->next) {
        if ( iter == object ) {
            return pos;
        }
        if (SP_IS_ITEM(iter)) {
            pos++;
        }
    }

    g_assert_not_reached();
    return 0;
}

void SPItem::getBboxDesktop(NRRect *bbox, SPItem::BBoxType type)
{
    g_assert(bbox != NULL);

    invoke_bbox( bbox, i2dt_affine(), TRUE, type);
}

Geom::OptRect SPItem::getBboxDesktop(SPItem::BBoxType type)
{
    Geom::OptRect rect = Geom::OptRect();
    invoke_bbox( rect, i2dt_affine(), TRUE, type);
    return rect;
}

void SPItem::sp_item_private_snappoints(SPItem const *item, std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const */*snapprefs*/)
{
    /* This will only be called if the derived class doesn't override this.
     * see for example sp_genericellipse_snappoints in sp-ellipse.cpp
     * We don't know what shape we could be dealing with here, so we'll just
     * return the corners of the bounding box */

    Geom::OptRect bbox = item->getBounds(item->i2dt_affine());

    if (bbox) {
        Geom::Point p1, p2;
        p1 = bbox->min();
        p2 = bbox->max();
        p.push_back(Inkscape::SnapCandidatePoint(p1, Inkscape::SNAPSOURCE_BBOX_CORNER, Inkscape::SNAPTARGET_BBOX_CORNER));
        p.push_back(Inkscape::SnapCandidatePoint(Geom::Point(p1[Geom::X], p2[Geom::Y]), Inkscape::SNAPSOURCE_BBOX_CORNER, Inkscape::SNAPTARGET_BBOX_CORNER));
        p.push_back(Inkscape::SnapCandidatePoint(p2, Inkscape::SNAPSOURCE_BBOX_CORNER, Inkscape::SNAPTARGET_BBOX_CORNER));
        p.push_back(Inkscape::SnapCandidatePoint(Geom::Point(p2[Geom::X], p1[Geom::Y]), Inkscape::SNAPSOURCE_BBOX_CORNER, Inkscape::SNAPTARGET_BBOX_CORNER));
    }

}

void SPItem::getSnappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const
{
    // Get the snappoints of the item
    SPItemClass const &item_class = *(SPItemClass const *) G_OBJECT_GET_CLASS(this);
    if (item_class.snappoints) {
        item_class.snappoints(this, p, snapprefs);
    }

    // Get the snappoints at the item's center
    if (snapprefs != NULL && snapprefs->getIncludeItemCenter()) {
        p.push_back(Inkscape::SnapCandidatePoint(getCenter(), Inkscape::SNAPSOURCE_ROTATION_CENTER, Inkscape::SNAPTARGET_ROTATION_CENTER));
    }

    // Get the snappoints of clipping paths and mask, if any
    std::list<SPObject const *> clips_and_masks;

    clips_and_masks.push_back(clip_ref->getObject());
    clips_and_masks.push_back(mask_ref->getObject());

    SPDesktop *desktop = inkscape_active_desktop();
    for (std::list<SPObject const *>::const_iterator o = clips_and_masks.begin(); o != clips_and_masks.end(); o++) {
        if (*o) {
            // obj is a group object, the children are the actual clippers
            for (SPObject *child = (*o)->children ; child ; child = child->next) {
                if (SP_IS_ITEM(child)) {
                    std::vector<Inkscape::SnapCandidatePoint> p_clip_or_mask;
                    // Please note the recursive call here!
                    SP_ITEM(child)->getSnappoints(p_clip_or_mask, snapprefs);
                    // Take into account the transformation of the item being clipped or masked
                    for (std::vector<Inkscape::SnapCandidatePoint>::const_iterator p_orig = p_clip_or_mask.begin(); p_orig != p_clip_or_mask.end(); p_orig++) {
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

void SPItem::invoke_print(SPPrintContext *ctx)
{
    if ( !isHidden() ) {
        if ( reinterpret_cast<SPItemClass *>(G_OBJECT_GET_CLASS(this))->print ) {
            if (!transform.isIdentity()
                || style->opacity.value != SP_SCALE24_MAX)
            {
                sp_print_bind(ctx, transform, SP_SCALE24_TO_FLOAT(style->opacity.value));
                reinterpret_cast<SPItemClass *>(G_OBJECT_GET_CLASS(this))->print(this, ctx);
                sp_print_release(ctx);
            } else {
                reinterpret_cast<SPItemClass *>(G_OBJECT_GET_CLASS(this))->print(this, ctx);
            }
        }
    }
}

gchar *SPItem::sp_item_private_description(SPItem */*item*/)
{
    return g_strdup(_("Object"));
}

/**
 * Returns a string suitable for status bar, formatted in pango markup language.
 *
 * Must be freed by caller.
 */
gchar *SPItem::description()
{
    if (((SPItemClass *) G_OBJECT_GET_CLASS(this))->description) {
        gchar *s = ((SPItemClass *) G_OBJECT_GET_CLASS(this))->description(this);
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

    g_assert_not_reached();
    return NULL;
}

/**
 * Allocates unique integer keys.
 * \param numkeys Number of keys required.
 * \return First allocated key; hence if the returned key is n
 * you can use n, n + 1, ..., n + (numkeys - 1)
 */
unsigned SPItem::display_key_new(unsigned numkeys)
{
    static unsigned dkey = 0;

    dkey += numkeys;

    return dkey - numkeys;
}

NRArenaItem *SPItem::invoke_show(NRArena *arena, unsigned key, unsigned flags)
{
    g_assert(arena != NULL);
    g_assert(NR_IS_ARENA(arena));

    NRArenaItem *ai = NULL;
    if (((SPItemClass *) G_OBJECT_GET_CLASS(this))->show) {
        ai = ((SPItemClass *) G_OBJECT_GET_CLASS(this))->show(this, arena, key, flags);
    }

    if (ai != NULL) {
        display = sp_item_view_new_prepend(display, this, flags, key, ai);
        nr_arena_item_set_transform(ai, transform);
        nr_arena_item_set_opacity(ai, SP_SCALE24_TO_FLOAT(style->opacity.value));
        nr_arena_item_set_visible(ai, !isHidden());
        nr_arena_item_set_sensitive(ai, sensitive);
        if (clip_ref->getObject()) {
            SPClipPath *cp = clip_ref->getObject();

            if (!display->arenaitem->key) {
                NR_ARENA_ITEM_SET_KEY(display->arenaitem, display_key_new(3));
            }
            int clip_key = NR_ARENA_ITEM_GET_KEY(display->arenaitem);

            // Show and set clip
            NRArenaItem *ac = cp->show(arena, clip_key);
            nr_arena_item_set_clip(ai, ac);
            nr_arena_item_unref(ac);

            // Update bbox, in case the clip uses bbox units
            NRRect bbox;
            invoke_bbox( &bbox, Geom::identity(), TRUE);
            SP_CLIPPATH(cp)->setBBox(clip_key, &bbox);
            cp->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }
        if (mask_ref->getObject()) {
            SPMask *mask = mask_ref->getObject();

            if (!display->arenaitem->key) {
                NR_ARENA_ITEM_SET_KEY(display->arenaitem, display_key_new(3));
            }
            int mask_key = NR_ARENA_ITEM_GET_KEY(display->arenaitem);

            // Show and set mask
            NRArenaItem *ac = sp_mask_show(mask, arena, mask_key);
            nr_arena_item_set_mask(ai, ac);
            nr_arena_item_unref(ac);

            // Update bbox, in case the mask uses bbox units
            NRRect bbox;
            invoke_bbox( &bbox, Geom::identity(), TRUE);
            sp_mask_set_bbox(SP_MASK(mask), mask_key, &bbox);
            mask->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }
        NR_ARENA_ITEM_SET_DATA(ai, this);
        Geom::OptRect item_bbox;
        invoke_bbox( item_bbox, Geom::identity(), TRUE, SPItem::GEOMETRIC_BBOX);
        nr_arena_item_set_item_bbox(ai, item_bbox);
    }

    return ai;
}

void SPItem::invoke_hide(unsigned key)
{
    if (((SPItemClass *) G_OBJECT_GET_CLASS(this))->hide) {
        ((SPItemClass *) G_OBJECT_GET_CLASS(this))->hide(this, key);
    }

    SPItemView *ref = NULL;
    SPItemView *v = display;
    while (v != NULL) {
        SPItemView *next = v->next;
        if (v->key == key) {
            if (clip_ref->getObject()) {
                (clip_ref->getObject())->hide(NR_ARENA_ITEM_GET_KEY(v->arenaitem));
                nr_arena_item_set_clip(v->arenaitem, NULL);
            }
            if (mask_ref->getObject()) {
                sp_mask_hide(mask_ref->getObject(), NR_ARENA_ITEM_GET_KEY(v->arenaitem));
                nr_arena_item_set_mask(v->arenaitem, NULL);
            }
            if (!ref) {
                display = v->next;
            } else {
                ref->next = v->next;
            }
            nr_arena_item_unparent(v->arenaitem);
            nr_arena_item_unref(v->arenaitem);
            g_free(v);
        } else {
            ref = v;
        }
        v = next;
    }
}

// Adjusters

void SPItem::adjust_pattern (Geom::Affine const &postmul, bool set)
{
    if (style && (style->fill.isPaintserver())) {
        SPObject *server = style->getFillPaintServer();
        if ( SP_IS_PATTERN(server) ) {
            SPPattern *pattern = sp_pattern_clone_if_necessary(this, SP_PATTERN(server), "fill");
            sp_pattern_transform_multiply(pattern, postmul, set);
        }
    }

    if (style && (style->stroke.isPaintserver())) {
        SPObject *server = style->getStrokePaintServer();
        if ( SP_IS_PATTERN(server) ) {
            SPPattern *pattern = sp_pattern_clone_if_necessary(this, SP_PATTERN(server), "stroke");
            sp_pattern_transform_multiply(pattern, postmul, set);
        }
    }
}

void SPItem::adjust_gradient( Geom::Affine const &postmul, bool set )
{
    if ( style && style->fill.isPaintserver() ) {
        SPPaintServer *server = style->getFillPaintServer();
        if ( SP_IS_GRADIENT(server) ) {

            /**
             * \note Bbox units for a gradient are generally a bad idea because
             * with them, you cannot preserve the relative position of the
             * object and its gradient after rotation or skew. So now we
             * convert them to userspace units which are easy to keep in sync
             * just by adding the object's transform to gradientTransform.
             * \todo FIXME: convert back to bbox units after transforming with
             * the item, so as to preserve the original units.
             */
            SPGradient *gradient = sp_gradient_convert_to_userspace( SP_GRADIENT(server), this, "fill" );

            sp_gradient_transform_multiply( gradient, postmul, set );
        }
    }

    if ( style && style->stroke.isPaintserver() ) {
        SPPaintServer *server = style->getStrokePaintServer();
        if ( SP_IS_GRADIENT(server) ) {
            SPGradient *gradient = sp_gradient_convert_to_userspace( SP_GRADIENT(server), this, "stroke");
            sp_gradient_transform_multiply( gradient, postmul, set );
        }
    }
}

void SPItem::adjust_stroke( gdouble ex )
{
    SPStyle *style = this->style;

    if (style && !style->stroke.isNone() && !Geom::are_near(ex, 1.0, Geom::EPSILON)) {
        style->stroke_width.computed *= ex;
        style->stroke_width.set = TRUE;

        if ( style->stroke_dash.n_dash != 0 ) {
            for (int i = 0; i < style->stroke_dash.n_dash; i++) {
                style->stroke_dash.dash[i] *= ex;
            }
            style->stroke_dash.offset *= ex;
        }

        updateRepr();
    }
}

/**
 * Find out the inverse of previous transform of an item (from its repr)
 */
Geom::Affine sp_item_transform_repr (SPItem *item)
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


/**
 * Recursively scale stroke width in \a item and its children by \a expansion.
 */
void SPItem::adjust_stroke_width_recursive(double expansion)
{
    adjust_stroke (expansion);

// A clone's child is the ghost of its original - we must not touch it, skip recursion
    if ( !SP_IS_USE(this) ) {
        for ( SPObject *o = children; o; o = o->getNext() ) {
            if (SP_IS_ITEM(o)) {
                SP_ITEM(o)->adjust_stroke_width_recursive(expansion);
            }
        }
    }
}

/**
 * Recursively adjust rx and ry of rects.
 */
void
sp_item_adjust_rects_recursive(SPItem *item, Geom::Affine advertized_transform)
{
    if (SP_IS_RECT (item)) {
        sp_rect_compensate_rxry (SP_RECT(item), advertized_transform);
    }

    for (SPObject *o = item->children; o != NULL; o = o->next) {
        if (SP_IS_ITEM(o))
            sp_item_adjust_rects_recursive(SP_ITEM(o), advertized_transform);
    }
}

/**
 * Recursively compensate pattern or gradient transform.
 */
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
    if (!(this && (SP_IS_TEXT(this) || SP_IS_USE(this)))) {
        for (SPObject *o = children; o != NULL; o = o->next) {
            if (SP_IS_ITEM(o)) {
// At the level of the transformed item, t_ancestors is identity;
// below it, it is the accmmulated chain of transforms from this level to the top level
                SP_ITEM(o)->adjust_paint_recursive (advertized_transform, t_item * t_ancestors, is_pattern);
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
    if ( SP_IS_LPE_ITEM(this) ) {
        SPLPEItem *lpeitem = SP_LPE_ITEM (this);
        if ( sp_lpe_item_has_path_effect(lpeitem) ) {
            sp_lpe_item_fork_path_effects_if_necessary(lpeitem);

            // now that all LPEs are forked_if_necessary, we can apply the transform
            PathEffectList effect_list =  sp_lpe_item_get_effect_list(lpeitem);
            for (PathEffectList::iterator it = effect_list.begin(); it != effect_list.end(); it++)
            {
                LivePathEffectObject *lpeobj = (*it)->lpeobject;
                if (lpeobj && lpeobj->get_lpe()) {
                    Inkscape::LivePathEffect::Effect * effect = lpeobj->get_lpe();
                    effect->transform_multiply(postmul, set);
                }
            }
        }
    }
}

/**
 * Set a new transform on an object.
 *
 * Compensate for stroke scaling and gradient/pattern fill transform, if
 * necessary. Call the object's set_transform method if transforms are
 * stored optimized. Send _transformed_signal. Invoke _write method so that
 * the repr is updated with the new transform.
 */
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

         // recursively compensate for stroke scaling, depending on user preference
        if (!prefs->getBool("/options/transform/stroke", true)) {
            double const expansion = 1. / advertized_transform.descrim();
            adjust_stroke_width_recursive(expansion);
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
    if ( // run the object's set_transform (i.e. embed transform) only if:
         ((SPItemClass *) G_OBJECT_GET_CLASS(this))->set_transform && // it does have a set_transform method
             !preserve && // user did not chose to preserve all transforms
             !clip_ref->getObject() && // the object does not have a clippath
             !mask_ref->getObject() && // the object does not have a mask
         !(!transform.isTranslation() && style && style->getFilter())
             // the object does not have a filter, or the transform is translation (which is supposed to not affect filters)
        ) {
        transform_attr = ((SPItemClass *) G_OBJECT_GET_CLASS(this))->set_transform(this, transform);
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

gint SPItem::emitEvent(SPEvent &event)
{
    if (((SPItemClass *) G_OBJECT_GET_CLASS(this))->event) {
        return ((SPItemClass *) G_OBJECT_GET_CLASS(this))->event(this, &event);
    }

    return FALSE;
}

/**
 * Sets item private transform (not propagated to repr), without compensating stroke widths,
 * gradients, patterns as sp_item_write_transform does.
 */
void SPItem::set_item_transform(Geom::Affine const &transform_matrix)
{
    if (!matrix_equalp(transform_matrix, transform, NR_EPSILON)) {
        transform = transform_matrix;
        /* The SP_OBJECT_USER_MODIFIED_FLAG_B is used to mark the fact that it's only a
           transformation.  It's apparently not used anywhere else. */
        requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_USER_MODIFIED_FLAG_B);
        sp_item_rm_unsatisfied_cns(*this);
    }
}

void SPItem::convert_item_to_guides() {
    // Use derived method if present ...
    if (((SPItemClass *) G_OBJECT_GET_CLASS(this))->convert_to_guides) {
        (*((SPItemClass *) G_OBJECT_GET_CLASS(this))->convert_to_guides)(this);
    } else {
        // .. otherwise simply place the guides around the item's bounding box

        convert_to_guides();
    }
}


/**
 * \pre \a ancestor really is an ancestor (\>=) of \a object, or NULL.
 *   ("Ancestor (\>=)" here includes as far as \a object itself.)
 */
Geom::Affine
i2anc_affine(SPObject const *object, SPObject const *const ancestor) {
    Geom::Affine ret(Geom::identity());
    g_return_val_if_fail(object != NULL, ret);

    /* stop at first non-renderable ancestor */
    while ( object != ancestor && SP_IS_ITEM(object) ) {
        if (SP_IS_ROOT(object)) {
            ret *= SP_ROOT(object)->c2p;
        } else {
            ret *= SP_ITEM(object)->transform;
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

/**
 * Returns the accumulated transformation of the item and all its ancestors, including root's viewport.
 * \pre (item != NULL) and SP_IS_ITEM(item).
 */
Geom::Affine SPItem::i2doc_affine() const
{
    return i2anc_affine(this, NULL);
}

/**
 * Returns the transformation from item to desktop coords
 */
Geom::Affine SPItem::i2dt_affine() const
{
//    Geom::Affine const ret( i2doc_affine()
//                          * Geom::Scale(1, -1)
//                          * Geom::Translate(0, document->getHeight()) );
    SPDesktop const *desktop = inkscape_active_desktop();
    Geom::Affine const ret( i2doc_affine() * desktop->doc2dt() );
    return ret;
}

void SPItem::set_i2d_affine(Geom::Affine const &i2dt)
{
    Geom::Affine dt2p; /* desktop to item parent transform */
    if (parent) {
        dt2p = static_cast<SPItem *>(parent)->i2dt_affine().inverse();
    } else {
        SPDesktop *dt = inkscape_active_desktop();
        dt2p = dt->dt2doc();
    }

    Geom::Affine const i2p( i2dt * dt2p );
    set_item_transform(i2p);
}


/**
 * should rather be named "sp_item_d2i_affine" to match "sp_item_i2d_affine" (or vice versa)
 */
Geom::Affine SPItem::dt2i_affine() const
{
    /* fixme: Implement the right way (Lauris) */
    return i2dt_affine().inverse();
}

/* Item views */

SPItemView *SPItem::sp_item_view_new_prepend(SPItemView *list, SPItem *item, unsigned flags, unsigned key, NRArenaItem *arenaitem)
{
    g_assert(item != NULL);
    g_assert(SP_IS_ITEM(item));
    g_assert(arenaitem != NULL);
    g_assert(NR_IS_ARENA_ITEM(arenaitem));

    SPItemView *new_view = g_new(SPItemView, 1);

    new_view->next = list;
    new_view->flags = flags;
    new_view->key = key;
    new_view->arenaitem = arenaitem;

    return new_view;
}

SPItemView *SPItem::sp_item_view_list_remove(SPItemView *list, SPItemView *view)
{
    if (view == list) {
        list = list->next;
    } else {
        SPItemView *prev;
        prev = list;
        while (prev->next != view) prev = prev->next;
        prev->next = view->next;
    }

    nr_arena_item_unref(view->arenaitem);
    g_free(view);

    return list;
}

/**
 * Return the arenaitem corresponding to the given item in the display
 * with the given key
 */
NRArenaItem *SPItem::get_arenaitem(unsigned key)
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
        if ( SP_IS_ITEM(iter) ) {
            child = SP_ITEM(iter);
            break;
        }
    }
    return child;
}

void SPItem::convert_to_guides() {
    SPDesktop *dt = inkscape_active_desktop();
    sp_desktop_namedview(dt);

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    int prefs_bbox = prefs->getInt("/tools/bounding_box", 0);
    SPItem::BBoxType bbox_type = (prefs_bbox ==0)?
        SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;

    Geom::OptRect bbox = getBboxDesktop(bbox_type);
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

    sp_guide_pt_pairs_to_guides(dt, pts);
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

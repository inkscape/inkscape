#define __SP_ITEM_C__

/** \file
 * Base class for visual SVG elements
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
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
#include "sp-switch.h"
#include "gradient-chemistry.h"
#include "prefs-utils.h"
#include "conn-avoid-ref.h"
#include "conditions.h"
#include "sp-filter-reference.h"
#include "filter-chemistry.h"
#include "sp-guide.h"

#include "libnr/nr-matrix-div.h"
#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-matrix-scale-ops.h"
#include "libnr/nr-matrix-translate-ops.h"
#include "libnr/nr-scale-translate-ops.h"
#include "libnr/nr-translate-scale-ops.h"
#include "libnr/nr-convert2geom.h"
#include "algorithms/find-last-if.h"
#include "util/reverse-list.h"

#include "xml/repr.h"
#include "extract-uri.h"

#include "live_effects/lpeobject.h"
#include "live_effects/effect.h"
#include "live_effects/lpeobject-reference.h"

#define noSP_ITEM_DEBUG_IDLE

static void sp_item_class_init(SPItemClass *klass);
static void sp_item_init(SPItem *item);

static void sp_item_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_item_release(SPObject *object);
static void sp_item_set(SPObject *object, unsigned key, gchar const *value);
static void sp_item_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_item_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static gchar *sp_item_private_description(SPItem *item);
static void sp_item_private_snappoints(SPItem const *item, SnapPointsIter p);

static SPItemView *sp_item_view_new_prepend(SPItemView *list, SPItem *item, unsigned flags, unsigned key, NRArenaItem *arenaitem);
static SPItemView *sp_item_view_list_remove(SPItemView *list, SPItemView *view);

static SPObjectClass *parent_class;

static void clip_ref_changed(SPObject *old_clip, SPObject *clip, SPItem *item);
static void mask_ref_changed(SPObject *old_clip, SPObject *clip, SPItem *item);

/**
 * Registers SPItem class and returns its type number.
 */
GType
sp_item_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPItemClass),
            NULL, NULL,
            (GClassInitFunc) sp_item_class_init,
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
static void
sp_item_class_init(SPItemClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    parent_class = (SPObjectClass *)g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->build = sp_item_build;
    sp_object_class->release = sp_item_release;
    sp_object_class->set = sp_item_set;
    sp_object_class->update = sp_item_update;
    sp_object_class->write = sp_item_write;

    klass->description = sp_item_private_description;
    klass->snappoints = sp_item_private_snappoints;
}

/**
 * Callback for SPItem object initialization.
 */
static void
sp_item_init(SPItem *item)
{
    item->init();
}

void SPItem::init() {
    this->sensitive = TRUE;

    this->transform_center_x = 0;
    this->transform_center_y = 0;

    this->_is_evaluated = true;
    this->_evaluated_status = StatusUnknown;

    this->transform = NR::identity();

    this->display = NULL;

    this->clip_ref = new SPClipPathReference(this);
		sigc::signal<void, SPObject *, SPObject *> cs1=this->clip_ref->changedSignal();
		sigc::slot2<void,SPObject*, SPObject *> sl1=sigc::bind(sigc::ptr_fun(clip_ref_changed), this);
    _clip_ref_connection = cs1.connect(sl1);

    this->mask_ref = new SPMaskReference(this);
		sigc::signal<void, SPObject *, SPObject *> cs2=this->mask_ref->changedSignal();
		sigc::slot2<void,SPObject*, SPObject *> sl2=sigc::bind(sigc::ptr_fun(mask_ref_changed), this);
    _mask_ref_connection = cs2.connect(sl2);

    this->avoidRef = new SPAvoidRef(this);

    new (&this->_transformed_signal) sigc::signal<void, NR::Matrix const *, SPItem *>();
}

bool SPItem::isVisibleAndUnlocked() const {
    return (!isHidden() && !isLocked());
}

bool SPItem::isVisibleAndUnlocked(unsigned display_key) const {
    return (!isHidden(display_key) && !isLocked());
}

bool SPItem::isLocked() const {
    for (SPObject *o = SP_OBJECT(this); o != NULL; o = SP_OBJECT_PARENT(o)) {
        if (SP_IS_ITEM(o) && !(SP_ITEM(o)->sensitive))
            return true;
    }
    return false;
}

void SPItem::setLocked(bool locked) {
    SP_OBJECT_REPR(this)->setAttribute("sodipodi:insensitive",
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
        SPObject const *const parent = SP_OBJECT_PARENT(this);
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
bool
SPItem::isExplicitlyHidden() const
{
    return (this->style->display.set
	    && this->style->display.value == SP_CSS_DISPLAY_NONE);
}

/**
 * Sets the display CSS property to `hidden' if \a val is true,
 * otherwise makes it unset
 */
void
SPItem::setExplicitlyHidden(bool const val) {
    this->style->display.set = val;
    this->style->display.value = ( val ? SP_CSS_DISPLAY_NONE : SP_CSS_DISPLAY_INLINE );
    this->style->display.computed = this->style->display.value;
    this->updateRepr();
}

/**
 * Sets the transform_center_x and transform_center_y properties to retain the rotation centre
 */
void
SPItem::setCenter(NR::Point object_centre) {
    NR::Maybe<NR::Rect> bbox = getBounds(sp_item_i2d_affine(this));
    if (bbox) {
        transform_center_x = object_centre[NR::X] - bbox->midpoint()[NR::X];
        if (fabs(transform_center_x) < 1e-5) // rounding error
            transform_center_x = 0;
        transform_center_y = object_centre[NR::Y] - bbox->midpoint()[NR::Y];
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

NR::Point SPItem::getCenter() const {
    NR::Maybe<NR::Rect> bbox = getBounds(sp_item_i2d_affine(this));
    if (bbox) {
        return bbox->midpoint() + NR::Point (this->transform_center_x, this->transform_center_y);
    } else {
        return NR::Point (0, 0); // something's wrong!
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
        SP_OBJECT_NEXT(this), NULL, &is_item
    );
    if (topmost) {
        Inkscape::XML::Node *repr=SP_OBJECT_REPR(this);
        sp_repr_parent(repr)->changeOrder(repr, SP_OBJECT_REPR(topmost));
    }
}

void SPItem::raiseOne() {
    SPObject *next_higher=std::find_if<SPObject::SiblingIterator>(
        SP_OBJECT_NEXT(this), NULL, &is_item
    );
    if (next_higher) {
        Inkscape::XML::Node *repr=SP_OBJECT_REPR(this);
        Inkscape::XML::Node *ref=SP_OBJECT_REPR(next_higher);
        sp_repr_parent(repr)->changeOrder(repr, ref);
    }
}

void SPItem::lowerOne() {
    using Inkscape::Util::MutableList;
    using Inkscape::Util::reverse_list;

    MutableList<SPObject &> next_lower=std::find_if(
        reverse_list<SPObject::SiblingIterator>(
            SP_OBJECT_PARENT(this)->firstChild(), this
        ),
        MutableList<SPObject &>(),
        &is_item
    );
    if (next_lower) {
        ++next_lower;
        Inkscape::XML::Node *repr=SP_OBJECT_REPR(this);
        Inkscape::XML::Node *ref=( next_lower ? SP_OBJECT_REPR(&*next_lower) : NULL );
        sp_repr_parent(repr)->changeOrder(repr, ref);
    }
}

void SPItem::lowerToBottom() {
    using Inkscape::Algorithms::find_last_if;
    using Inkscape::Util::MutableList;
    using Inkscape::Util::reverse_list;

    MutableList<SPObject &> bottom=find_last_if(
        reverse_list<SPObject::SiblingIterator>(
            SP_OBJECT_PARENT(this)->firstChild(), this
        ),
        MutableList<SPObject &>(),
        &is_item
    );
    if (bottom) {
        ++bottom;
        Inkscape::XML::Node *repr=SP_OBJECT_REPR(this);
        Inkscape::XML::Node *ref=( bottom ? SP_OBJECT_REPR(&*bottom) : NULL );
        sp_repr_parent(repr)->changeOrder(repr, ref);
    }
}

static void
sp_item_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    sp_object_read_attr(object, "style");
    sp_object_read_attr(object, "transform");
    sp_object_read_attr(object, "clip-path");
    sp_object_read_attr(object, "mask");
    sp_object_read_attr(object, "sodipodi:insensitive");
    sp_object_read_attr(object, "sodipodi:nonprintable");
    sp_object_read_attr(object, "inkscape:transform-center-x");
    sp_object_read_attr(object, "inkscape:transform-center-y");
    sp_object_read_attr(object, "inkscape:connector-avoid");

    if (((SPObjectClass *) (parent_class))->build) {
        (* ((SPObjectClass *) (parent_class))->build)(object, document, repr);
    }
}

static void
sp_item_release(SPObject *object)
{
    SPItem *item = (SPItem *) object;

    item->_clip_ref_connection.disconnect();
    item->_mask_ref_connection.disconnect();

    // Note: do this here before the clip_ref is deleted, since calling
    // sp_document_ensure_up_to_date for triggered routing may reference
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

    if (((SPObjectClass *) (parent_class))->release) {
        ((SPObjectClass *) parent_class)->release(object);
    }

    while (item->display) {
        nr_arena_item_unparent(item->display->arenaitem);
        item->display = sp_item_view_list_remove(item->display, item->display);
    }

    item->_transformed_signal.~signal();
}

static void
sp_item_set(SPObject *object, unsigned key, gchar const *value)
{
    SPItem *item = (SPItem *) object;

    switch (key) {
        case SP_ATTR_TRANSFORM: {
            NR::Matrix t;
            if (value && sp_svg_transform_read(value, &t)) {
                sp_item_set_item_transform(item, t);
            } else {
                sp_item_set_item_transform(item, NR::identity());
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
                if (((SPObjectClass *) (parent_class))->set) {
                    (* ((SPObjectClass *) (parent_class))->set)(object, key, value);
                }
            }
            break;
    }
}

static void
clip_ref_changed(SPObject *old_clip, SPObject *clip, SPItem *item)
{
    if (old_clip) {
        SPItemView *v;
        /* Hide clippath */
        for (v = item->display; v != NULL; v = v->next) {
            sp_clippath_hide(SP_CLIPPATH(old_clip), NR_ARENA_ITEM_GET_KEY(v->arenaitem));
            nr_arena_item_set_clip(v->arenaitem, NULL);
        }
    }
    if (SP_IS_CLIPPATH(clip)) {
        NRRect bbox;
        sp_item_invoke_bbox(item, &bbox, NR::identity(), TRUE);
        for (SPItemView *v = item->display; v != NULL; v = v->next) {
            if (!v->arenaitem->key) {
                NR_ARENA_ITEM_SET_KEY(v->arenaitem, sp_item_display_key_new(3));
            }
            NRArenaItem *ai = sp_clippath_show(SP_CLIPPATH(clip),
                                               NR_ARENA_ITEM_ARENA(v->arenaitem),
                                               NR_ARENA_ITEM_GET_KEY(v->arenaitem));
            nr_arena_item_set_clip(v->arenaitem, ai);
            nr_arena_item_unref(ai);
            sp_clippath_set_bbox(SP_CLIPPATH(clip), NR_ARENA_ITEM_GET_KEY(v->arenaitem), &bbox);
            SP_OBJECT(clip)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }
    }
}

static void
mask_ref_changed(SPObject *old_mask, SPObject *mask, SPItem *item)
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
        sp_item_invoke_bbox(item, &bbox, NR::identity(), TRUE);
        for (SPItemView *v = item->display; v != NULL; v = v->next) {
            if (!v->arenaitem->key) {
                NR_ARENA_ITEM_SET_KEY(v->arenaitem, sp_item_display_key_new(3));
            }
            NRArenaItem *ai = sp_mask_show(SP_MASK(mask),
                                           NR_ARENA_ITEM_ARENA(v->arenaitem),
                                           NR_ARENA_ITEM_GET_KEY(v->arenaitem));
            nr_arena_item_set_mask(v->arenaitem, ai);
            nr_arena_item_unref(ai);
            sp_mask_set_bbox(SP_MASK(mask), NR_ARENA_ITEM_GET_KEY(v->arenaitem), &bbox);
            SP_OBJECT(mask)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }
    }
}

static void
sp_item_update(SPObject *object, SPCtx *ctx, guint flags)
{
    SPItem *item = SP_ITEM(object);

    if (((SPObjectClass *) (parent_class))->update)
        (* ((SPObjectClass *) (parent_class))->update)(object, ctx, flags);

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
            sp_item_invoke_bbox(item, &bbox, NR::identity(), TRUE);
            if (clip_path) {
                for (SPItemView *v = item->display; v != NULL; v = v->next) {
                    sp_clippath_set_bbox(clip_path, NR_ARENA_ITEM_GET_KEY(v->arenaitem), &bbox);
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
        NRRect item_bbox;
        sp_item_invoke_bbox(item, &item_bbox, NR::identity(), TRUE, SPItem::GEOMETRIC_BBOX);
        NR::Maybe<NR::Rect> i_bbox = item_bbox;

        SPItemView *itemview = item->display;
        do {
            if (itemview->arenaitem)
                nr_arena_item_set_item_bbox(itemview->arenaitem, i_bbox);
        } while ( (itemview = itemview->next) );
    }

    // Update libavoid with item geometry (for connector routing).
    if (item->avoidRef)
        item->avoidRef->handleSettingChange();
}

static Inkscape::XML::Node *
sp_item_write(SPObject *const object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPItem *item = SP_ITEM(object);

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

    if (((SPObjectClass *) (parent_class))->write) {
        ((SPObjectClass *) (parent_class))->write(object, xml_doc, repr, flags);
    }

    return repr;
}

NR::Maybe<NR::Rect> SPItem::getBounds(NR::Matrix const &transform,
                                      SPItem::BBoxType type,
                                      unsigned int /*dkey*/) const
{
    NR::Maybe<NR::Rect> r = NR::Nothing();
    sp_item_invoke_bbox_full(this, &r, transform, type, TRUE);
    return r;
}

void
sp_item_invoke_bbox(SPItem const *item, NR::Maybe<NR::Rect> *bbox, NR::Matrix const &transform, unsigned const clear, SPItem::BBoxType type)
{
    sp_item_invoke_bbox_full(item, bbox, transform, type, clear);
}

// DEPRECATED to phase out the use of NRRect in favor of NR::Maybe<NR::Rect>
void
sp_item_invoke_bbox(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const clear, SPItem::BBoxType type)
{
    sp_item_invoke_bbox_full(item, bbox, transform, type, clear);
}

/** Calls \a item's subclass' bounding box method; clips it by the bbox of clippath, if any; and
 * unions the resulting bbox with \a bbox. If \a clear is true, empties \a bbox first. Passes the
 * transform and the flags to the actual bbox methods. Note that many of subclasses (e.g. groups,
 * clones), in turn, call this function in their bbox methods. */
void
sp_item_invoke_bbox_full(SPItem const *item, NR::Maybe<NR::Rect> *bbox, NR::Matrix const &transform, unsigned const flags, unsigned const clear)
{
    g_assert(item != NULL);
    g_assert(SP_IS_ITEM(item));
    g_assert(bbox != NULL);

    if (clear) {
        *bbox = NR::Nothing();
    }

    // TODO: replace NRRect by NR::Rect, for all SPItemClasses, and for SP_CLIPPATH

    NRRect temp_bbox;
    temp_bbox.x0 = temp_bbox.y0 = NR_HUGE;
    temp_bbox.x1 = temp_bbox.y1 = -NR_HUGE;

    // call the subclass method
    if (((SPItemClass *) G_OBJECT_GET_CLASS(item))->bbox) {
        ((SPItemClass *) G_OBJECT_GET_CLASS(item))->bbox(item, &temp_bbox, transform, flags);
    }

    // unless this is geometric bbox, extend by filter area and crop the bbox by clip path, if any
    if ((SPItem::BBoxType) flags != SPItem::GEOMETRIC_BBOX) {
        if (SP_OBJECT_STYLE(item) && SP_OBJECT_STYLE(item)->filter.href) {
            SPObject *filter = SP_OBJECT_STYLE(item)->getFilter();
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
                NR::Matrix i2d = sp_item_i2d_affine (item);
                dx0 *= NR::expansionX(i2d);
                dx1 *= NR::expansionX(i2d);
                dy0 *= NR::expansionY(i2d);
                dy1 *= NR::expansionY(i2d);

                // expand the bbox
                temp_bbox.x0 += dx0;
                temp_bbox.x1 += dx1;
                temp_bbox.y0 += dy0;
                temp_bbox.y1 += dy1;
            }
        }
        if (item->clip_ref->getObject()) {
            NRRect b;
            sp_clippath_get_bbox(SP_CLIPPATH(item->clip_ref->getObject()), &b, transform, flags);
            nr_rect_d_intersect (&temp_bbox, &temp_bbox, &b);
        }
    }

    if (temp_bbox.x0 > temp_bbox.x1 || temp_bbox.y0 > temp_bbox.y1) {
        // Either the bbox hasn't been touched by the SPItemClass' bbox method 
        // (it still has its initial values, see above: x0 = y0 = NR_HUGE and x1 = y1 = -NR_HUGE)
        // or it has explicitely been set to be like this (e.g. in sp_shape_bbox)
        
        // When x0 > x1 or y0 > y1, the bbox is considered to be "nothing", although it has not been 
        // explicitely defined this way for NRRects (as opposed to NR::Maybe<NR::Rect>)
        *bbox = NR::Nothing();
        return;
    }

    // Do not use temp_bbox.upgrade() here, because it uses a test that returns NR::Nothing
    // for any rectangle with zero area. The geometrical bbox of for example a vertical line
    // would therefore be translated into NR::Nothing (see bug https://bugs.launchpad.net/inkscape/+bug/168684)
    NR::Maybe<NR::Rect> temp_bbox_new = NR::Rect(NR::Point(temp_bbox.x0, temp_bbox.y0), NR::Point(temp_bbox.x1, temp_bbox.y1));

    *bbox = NR::union_bounds(*bbox, temp_bbox_new);
}

// DEPRECATED to phase out the use of NRRect in favor of NR::Maybe<NR::Rect>
/** Calls \a item's subclass' bounding box method; clips it by the bbox of clippath, if any; and
 * unions the resulting bbox with \a bbox. If \a clear is true, empties \a bbox first. Passes the
 * transform and the flags to the actual bbox methods. Note that many of subclasses (e.g. groups,
 * clones), in turn, call this function in their bbox methods. */
void
sp_item_invoke_bbox_full(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const flags, unsigned const clear)
{
    g_assert(item != NULL);
    g_assert(SP_IS_ITEM(item));
    g_assert(bbox != NULL);

    if (clear) {
        bbox->x0 = bbox->y0 = 1e18;
        bbox->x1 = bbox->y1 = -1e18;
    }

    NRRect this_bbox;
    this_bbox.x0 = this_bbox.y0 = 1e18;
    this_bbox.x1 = this_bbox.y1 = -1e18;

    // call the subclass method
    if (((SPItemClass *) G_OBJECT_GET_CLASS(item))->bbox) {
        ((SPItemClass *) G_OBJECT_GET_CLASS(item))->bbox(item, &this_bbox, transform, flags);
    }

    // unless this is geometric bbox, crop the bbox by clip path, if any
    if ((SPItem::BBoxType) flags != SPItem::GEOMETRIC_BBOX && item->clip_ref->getObject()) {
        NRRect b;
        sp_clippath_get_bbox(SP_CLIPPATH(item->clip_ref->getObject()), &b, transform, flags);
        nr_rect_d_intersect (&this_bbox, &this_bbox, &b);
    }

    // if non-empty (with some tolerance - ?) union this_bbox with the bbox we've got passed
    if ( fabs(this_bbox.x1-this_bbox.x0) > -0.00001 && fabs(this_bbox.y1-this_bbox.y0) > -0.00001 ) {
        nr_rect_d_union (bbox, bbox, &this_bbox);
    }
}

unsigned sp_item_pos_in_parent(SPItem *item)
{
    g_assert(item != NULL);
    g_assert(SP_IS_ITEM(item));

    SPObject *parent = SP_OBJECT_PARENT(item);
    g_assert(parent != NULL);
    g_assert(SP_IS_OBJECT(parent));

    SPObject *object = SP_OBJECT(item);

    unsigned pos=0;
    for ( SPObject *iter = sp_object_first_child(parent) ; iter ; iter = SP_OBJECT_NEXT(iter)) {
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

void
sp_item_bbox_desktop(SPItem *item, NRRect *bbox, SPItem::BBoxType type)
{
    g_assert(item != NULL);
    g_assert(SP_IS_ITEM(item));
    g_assert(bbox != NULL);

    sp_item_invoke_bbox(item, bbox, sp_item_i2d_affine(item), TRUE, type);
}

NR::Maybe<NR::Rect> sp_item_bbox_desktop(SPItem *item, SPItem::BBoxType type)
{
    NR::Maybe<NR::Rect> rect = NR::Nothing();
    sp_item_invoke_bbox(item, &rect, sp_item_i2d_affine(item), TRUE, type);
    return rect;
}

static void sp_item_private_snappoints(SPItem const *item, SnapPointsIter p)
{
    NR::Maybe<NR::Rect> bbox = item->getBounds(sp_item_i2d_affine(item));
    /* Just the corners of the bounding box suffices given that we don't yet
       support angled guide lines. */

    if (bbox) {
        NR::Point p1, p2;
        p1 = bbox->min();
        p2 = bbox->max();
        *p = p1;
        *p = NR::Point(p1[NR::X], p2[NR::Y]);
        *p = p2;
        *p = NR::Point(p1[NR::Y], p2[NR::X]);
    }
}

void sp_item_snappoints(SPItem const *item, bool includeItemCenter, SnapPointsIter p)
{
    g_assert (item != NULL);
    g_assert (SP_IS_ITEM(item));

    SPItemClass const &item_class = *(SPItemClass const *) G_OBJECT_GET_CLASS(item);
    if (item_class.snappoints) {
        item_class.snappoints(item, p);
    }

    if (includeItemCenter) {
    	*p = item->getCenter();
    }
}

void
sp_item_invoke_print(SPItem *item, SPPrintContext *ctx)
{
    if (!item->isHidden()) {
        if (((SPItemClass *) G_OBJECT_GET_CLASS(item))->print) {
            if (!item->transform.test_identity()
                || SP_OBJECT_STYLE(item)->opacity.value != SP_SCALE24_MAX)
            {
                sp_print_bind(ctx, item->transform, SP_SCALE24_TO_FLOAT(SP_OBJECT_STYLE(item)->opacity.value));
                ((SPItemClass *) G_OBJECT_GET_CLASS(item))->print(item, ctx);
                sp_print_release(ctx);
            } else {
                ((SPItemClass *) G_OBJECT_GET_CLASS(item))->print(item, ctx);
            }
        }
    }
}

static gchar *
sp_item_private_description(SPItem */*item*/)
{
    return g_strdup(_("Object"));
}

/**
 * Returns a string suitable for status bar, formatted in pango markup language.
 *
 * Must be freed by caller.
 */
gchar *
sp_item_description(SPItem *item)
{
    g_assert(item != NULL);
    g_assert(SP_IS_ITEM(item));

    if (((SPItemClass *) G_OBJECT_GET_CLASS(item))->description) {
        gchar *s = ((SPItemClass *) G_OBJECT_GET_CLASS(item))->description(item);
        if (s && item->clip_ref->getObject()) {
            gchar *snew = g_strdup_printf (_("%s; <i>clipped</i>"), s);
            g_free (s);
            s = snew;
        }
        if (s && item->mask_ref->getObject()) {
            gchar *snew = g_strdup_printf (_("%s; <i>masked</i>"), s);
            g_free (s);
            s = snew;
        }
        if (SP_OBJECT_STYLE(item) && SP_OBJECT_STYLE(item)->filter.href) {
            gchar *snew = g_strdup_printf (_("%s; <i>filtered</i>"), s);
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
unsigned
sp_item_display_key_new(unsigned numkeys)
{
    static unsigned dkey = 0;

    dkey += numkeys;

    return dkey - numkeys;
}

NRArenaItem *
sp_item_invoke_show(SPItem *item, NRArena *arena, unsigned key, unsigned flags)
{
    g_assert(item != NULL);
    g_assert(SP_IS_ITEM(item));
    g_assert(arena != NULL);
    g_assert(NR_IS_ARENA(arena));

    NRArenaItem *ai = NULL;
    if (((SPItemClass *) G_OBJECT_GET_CLASS(item))->show) {
        ai = ((SPItemClass *) G_OBJECT_GET_CLASS(item))->show(item, arena, key, flags);
    }

    if (ai != NULL) {
        item->display = sp_item_view_new_prepend(item->display, item, flags, key, ai);
        nr_arena_item_set_transform(ai, item->transform);
        nr_arena_item_set_opacity(ai, SP_SCALE24_TO_FLOAT(SP_OBJECT_STYLE(item)->opacity.value));
        nr_arena_item_set_visible(ai, !item->isHidden());
        nr_arena_item_set_sensitive(ai, item->sensitive);
        if (item->clip_ref->getObject()) {
            SPClipPath *cp = item->clip_ref->getObject();

            if (!item->display->arenaitem->key) {
                NR_ARENA_ITEM_SET_KEY(item->display->arenaitem, sp_item_display_key_new(3));
            }
            int clip_key = NR_ARENA_ITEM_GET_KEY(item->display->arenaitem);

            // Show and set clip
            NRArenaItem *ac = sp_clippath_show(cp, arena, clip_key);
            nr_arena_item_set_clip(ai, ac);
            nr_arena_item_unref(ac);

            // Update bbox, in case the clip uses bbox units
            NRRect bbox;
            sp_item_invoke_bbox(item, &bbox, NR::identity(), TRUE);
            sp_clippath_set_bbox(SP_CLIPPATH(cp), clip_key, &bbox);
            SP_OBJECT(cp)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }
        if (item->mask_ref->getObject()) {
            SPMask *mask = item->mask_ref->getObject();

            if (!item->display->arenaitem->key) {
                NR_ARENA_ITEM_SET_KEY(item->display->arenaitem, sp_item_display_key_new(3));
            }
            int mask_key = NR_ARENA_ITEM_GET_KEY(item->display->arenaitem);

            // Show and set mask
            NRArenaItem *ac = sp_mask_show(mask, arena, mask_key);
            nr_arena_item_set_mask(ai, ac);
            nr_arena_item_unref(ac);

            // Update bbox, in case the mask uses bbox units
            NRRect bbox;
            sp_item_invoke_bbox(item, &bbox, NR::identity(), TRUE);
            sp_mask_set_bbox(SP_MASK(mask), mask_key, &bbox);
            SP_OBJECT(mask)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        }
        NR_ARENA_ITEM_SET_DATA(ai, item);
        NRRect item_bbox;
        sp_item_invoke_bbox(item, &item_bbox, NR::identity(), TRUE, SPItem::GEOMETRIC_BBOX);
        NR::Maybe<NR::Rect> i_bbox = item_bbox;
        nr_arena_item_set_item_bbox(ai, i_bbox);
    }

    return ai;
}

void
sp_item_invoke_hide(SPItem *item, unsigned key)
{
    g_assert(item != NULL);
    g_assert(SP_IS_ITEM(item));

    if (((SPItemClass *) G_OBJECT_GET_CLASS(item))->hide) {
        ((SPItemClass *) G_OBJECT_GET_CLASS(item))->hide(item, key);
    }

    SPItemView *ref = NULL;
    SPItemView *v = item->display;
    while (v != NULL) {
        SPItemView *next = v->next;
        if (v->key == key) {
            if (item->clip_ref->getObject()) {
                sp_clippath_hide(item->clip_ref->getObject(), NR_ARENA_ITEM_GET_KEY(v->arenaitem));
                nr_arena_item_set_clip(v->arenaitem, NULL);
            }
            if (item->mask_ref->getObject()) {
                sp_mask_hide(item->mask_ref->getObject(), NR_ARENA_ITEM_GET_KEY(v->arenaitem));
                nr_arena_item_set_mask(v->arenaitem, NULL);
            }
            if (!ref) {
                item->display = v->next;
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

void
sp_item_adjust_pattern (SPItem *item, NR::Matrix const &postmul, bool set)
{
    SPStyle *style = SP_OBJECT_STYLE (item);

    if (style && (style->fill.isPaintserver())) {
        SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (item);
        if (SP_IS_PATTERN (server)) {
            SPPattern *pattern = sp_pattern_clone_if_necessary (item, SP_PATTERN (server), "fill");
            sp_pattern_transform_multiply (pattern, postmul, set);
        }
    }

    if (style && (style->stroke.isPaintserver())) {
        SPObject *server = SP_OBJECT_STYLE_STROKE_SERVER (item);
        if (SP_IS_PATTERN (server)) {
            SPPattern *pattern = sp_pattern_clone_if_necessary (item, SP_PATTERN (server), "stroke");
            sp_pattern_transform_multiply (pattern, postmul, set);
        }
    }

}

void
sp_item_adjust_gradient (SPItem *item, NR::Matrix const &postmul, bool set)
{
    SPStyle *style = SP_OBJECT_STYLE (item);

    if (style && (style->fill.isPaintserver())) {
        SPObject *server = SP_OBJECT_STYLE_FILL_SERVER(item);
        if (SP_IS_GRADIENT (server)) {

            /**
             * \note Bbox units for a gradient are generally a bad idea because
             * with them, you cannot preserve the relative position of the
             * object and its gradient after rotation or skew. So now we
             * convert them to userspace units which are easy to keep in sync
             * just by adding the object's transform to gradientTransform.
             * \todo FIXME: convert back to bbox units after transforming with
             * the item, so as to preserve the original units.
             */
            SPGradient *gradient = sp_gradient_convert_to_userspace (SP_GRADIENT (server), item, "fill");

            sp_gradient_transform_multiply (gradient, postmul, set);
        }
    }

    if (style && (style->stroke.isPaintserver())) {
        SPObject *server = SP_OBJECT_STYLE_STROKE_SERVER(item);
        if (SP_IS_GRADIENT (server)) {
            SPGradient *gradient = sp_gradient_convert_to_userspace (SP_GRADIENT (server), item, "stroke");
            sp_gradient_transform_multiply (gradient, postmul, set);
        }
    }
}

void
sp_item_adjust_stroke (SPItem *item, gdouble ex)
{
    SPStyle *style = SP_OBJECT_STYLE (item);

    if (style && !style->stroke.isNone() && !NR_DF_TEST_CLOSE (ex, 1.0, NR_EPSILON)) {

        style->stroke_width.computed *= ex;
        style->stroke_width.set = TRUE;

        if (style->stroke_dash.n_dash != 0) {
            int i;
            for (i = 0; i < style->stroke_dash.n_dash; i++) {
                style->stroke_dash.dash[i] *= ex;
            }
            style->stroke_dash.offset *= ex;
        }

        SP_OBJECT(item)->updateRepr();
    }
}

/**
 * Find out the inverse of previous transform of an item (from its repr)
 */
NR::Matrix
sp_item_transform_repr (SPItem *item)
{
    NR::Matrix t_old(NR::identity());
    gchar const *t_attr = SP_OBJECT_REPR(item)->attribute("transform");
    if (t_attr) {
        NR::Matrix t;
        if (sp_svg_transform_read(t_attr, &t)) {
            t_old = t;
        }
    }

    return t_old;
}


/**
 * Recursively scale stroke width in \a item and its children by \a expansion.
 */
void
sp_item_adjust_stroke_width_recursive(SPItem *item, double expansion)
{
    sp_item_adjust_stroke (item, expansion);

// A clone's child is the ghost of its original - we must not touch it, skip recursion
    if (item && SP_IS_USE(item))
        return;

    for (SPObject *o = SP_OBJECT(item)->children; o != NULL; o = o->next) {
        if (SP_IS_ITEM(o))
            sp_item_adjust_stroke_width_recursive(SP_ITEM(o), expansion);
    }
}

/**
 * Recursively adjust rx and ry of rects.
 */
void
sp_item_adjust_rects_recursive(SPItem *item, NR::Matrix advertized_transform)
{
    if (SP_IS_RECT (item)) {
        sp_rect_compensate_rxry (SP_RECT(item), advertized_transform);
    }

    for (SPObject *o = SP_OBJECT(item)->children; o != NULL; o = o->next) {
        if (SP_IS_ITEM(o))
            sp_item_adjust_rects_recursive(SP_ITEM(o), advertized_transform);
    }
}

/**
 * Recursively compensate pattern or gradient transform.
 */
void
sp_item_adjust_paint_recursive (SPItem *item, NR::Matrix advertized_transform, NR::Matrix t_ancestors, bool is_pattern)
{
// _Before_ full pattern/gradient transform: t_paint * t_item * t_ancestors
// _After_ full pattern/gradient transform: t_paint_new * t_item * t_ancestors * advertised_transform
// By equating these two expressions we get t_paint_new = t_paint * paint_delta, where:
    NR::Matrix t_item = sp_item_transform_repr (item);
    NR::Matrix paint_delta = t_item * t_ancestors * advertized_transform * t_ancestors.inverse() * t_item.inverse();

// Within text, we do not fork gradients, and so must not recurse to avoid double compensation;
// also we do not recurse into clones, because a clone's child is the ghost of its original -
// we must not touch it
    if (!(item && (SP_IS_TEXT(item) || SP_IS_USE(item)))) {
        for (SPObject *o = SP_OBJECT(item)->children; o != NULL; o = o->next) {
            if (SP_IS_ITEM(o)) {
// At the level of the transformed item, t_ancestors is identity;
// below it, it is the accmmulated chain of transforms from this level to the top level
                sp_item_adjust_paint_recursive (SP_ITEM(o), advertized_transform, t_item * t_ancestors, is_pattern);
            }
        }
    }

// We recursed into children first, and are now adjusting this object second;
// this is so that adjustments in a tree are done from leaves up to the root,
// and paintservers on leaves inheriting their values from ancestors could adjust themselves properly
// before ancestors themselves are adjusted, probably differently (bug 1286535)

    if (is_pattern)
        sp_item_adjust_pattern (item, paint_delta);
    else
        sp_item_adjust_gradient (item, paint_delta);

}

void
sp_item_adjust_livepatheffect (SPItem *item, NR::Matrix const &postmul, bool set)
{
    if ( !SP_IS_LPE_ITEM(item) )
        return;

    SPLPEItem *lpeitem = SP_LPE_ITEM (item);
    if ( sp_lpe_item_has_path_effect(lpeitem) ) {
        PathEffectList effect_list =  sp_lpe_item_get_effect_list(lpeitem);
        for (PathEffectList::iterator it = effect_list.begin(); it != effect_list.end(); it++)
        {
            // If the path effect is used by 2 or more items, fork it
            // so that each object has its own independent copy of the effect
            LivePathEffectObject *lpeobj = (*it)->lpeobject;
            LivePathEffectObject *new_lpeobj = lpeobj->fork_private_if_necessary();
            if (new_lpeobj != lpeobj) {
                sp_lpe_item_replace_path_effect(lpeitem, lpeobj, new_lpeobj);
            }
        
            if (lpeobj->lpe) {
                Inkscape::LivePathEffect::Effect * effect = lpeobj->lpe;
                effect->transform_multiply(to_2geom(postmul), set);
            }
        }
    }
}

/**
 * A temporary wrapper for the next function accepting the NR::Matrix
 * instead of NR::Matrix
 */
void
sp_item_write_transform(SPItem *item, Inkscape::XML::Node *repr, NR::Matrix const *transform, NR::Matrix const *adv)
{
    if (transform == NULL)
        sp_item_write_transform(item, repr, NR::identity(), adv);
    else
        sp_item_write_transform(item, repr, *transform, adv);
}

/**
 * Set a new transform on an object.
 *
 * Compensate for stroke scaling and gradient/pattern fill transform, if
 * necessary. Call the object's set_transform method if transforms are
 * stored optimized. Send _transformed_signal. Invoke _write method so that
 * the repr is updated with the new transform.
 */
void
sp_item_write_transform(SPItem *item, Inkscape::XML::Node *repr, NR::Matrix const &transform, NR::Matrix const *adv, bool compensate)
{
    g_return_if_fail(item != NULL);
    g_return_if_fail(SP_IS_ITEM(item));
    g_return_if_fail(repr != NULL);

    // calculate the relative transform, if not given by the adv attribute
    NR::Matrix advertized_transform;
    if (adv != NULL) {
        advertized_transform = *adv;
    } else {
        advertized_transform = sp_item_transform_repr (item).inverse() * transform;
    }

    if (compensate) {

         // recursively compensate for stroke scaling, depending on user preference
        if (prefs_get_int_attribute("options.transform", "stroke", 1) == 0) {
            double const expansion = 1. / NR::expansion(advertized_transform);
            sp_item_adjust_stroke_width_recursive(item, expansion);
        }

        // recursively compensate rx/ry of a rect if requested
        if (prefs_get_int_attribute("options.transform", "rectcorners", 1) == 0) {
            sp_item_adjust_rects_recursive(item, advertized_transform);
        }

        // recursively compensate pattern fill if it's not to be transformed
        if (prefs_get_int_attribute("options.transform", "pattern", 1) == 0) {
            sp_item_adjust_paint_recursive (item, advertized_transform.inverse(), NR::identity(), true);
        }
        /// \todo FIXME: add the same else branch as for gradients below, to convert patterns to userSpaceOnUse as well
        /// recursively compensate gradient fill if it's not to be transformed
        if (prefs_get_int_attribute("options.transform", "gradient", 1) == 0) {
            sp_item_adjust_paint_recursive (item, advertized_transform.inverse(), NR::identity(), false);
        } else {
            // this converts the gradient/pattern fill/stroke, if any, to userSpaceOnUse; we need to do
            // it here _before_ the new transform is set, so as to use the pre-transform bbox
            sp_item_adjust_paint_recursive (item, NR::identity(), NR::identity(), false);
        }

    } // endif(compensate)

    gint preserve = prefs_get_int_attribute("options.preservetransform", "value", 0);
    NR::Matrix transform_attr (transform);
    if ( // run the object's set_transform (i.e. embed transform) only if:
         ((SPItemClass *) G_OBJECT_GET_CLASS(item))->set_transform && // it does have a set_transform method
             !preserve && // user did not chose to preserve all transforms
             !item->clip_ref->getObject() && // the object does not have a clippath
             !item->mask_ref->getObject() && // the object does not have a mask
         !(!transform.is_translation() && SP_OBJECT_STYLE(item) && SP_OBJECT_STYLE(item)->getFilter())
             // the object does not have a filter, or the transform is translation (which is supposed to not affect filters)
        ) {
        transform_attr = ((SPItemClass *) G_OBJECT_GET_CLASS(item))->set_transform(item, transform);
    }
    sp_item_set_item_transform(item, transform_attr);

    // Note: updateRepr comes before emitting the transformed signal since
    // it causes clone SPUse's copy of the original object to brought up to
    // date with the original.  Otherwise, sp_use_bbox returns incorrect
    // values if called in code handling the transformed signal.
    SP_OBJECT(item)->updateRepr();

    // send the relative transform with a _transformed_signal
    item->_transformed_signal.emit(&advertized_transform, item);
}

gint
sp_item_event(SPItem *item, SPEvent *event)
{
    g_return_val_if_fail(item != NULL, FALSE);
    g_return_val_if_fail(SP_IS_ITEM(item), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    if (((SPItemClass *) G_OBJECT_GET_CLASS(item))->event)
        return ((SPItemClass *) G_OBJECT_GET_CLASS(item))->event(item, event);

    return FALSE;
}

/**
 * Sets item private transform (not propagated to repr), without compensating stroke widths,
 * gradients, patterns as sp_item_write_transform does.
 */
void
sp_item_set_item_transform(SPItem *item, NR::Matrix const &transform)
{
    g_return_if_fail(item != NULL);
    g_return_if_fail(SP_IS_ITEM(item));

    if (!matrix_equalp(transform, item->transform, NR_EPSILON)) {
        item->transform = transform;
        /* The SP_OBJECT_USER_MODIFIED_FLAG_B is used to mark the fact that it's only a
           transformation.  It's apparently not used anywhere else. */
        item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_USER_MODIFIED_FLAG_B);
        sp_item_rm_unsatisfied_cns(*item);
    }
}

void
sp_item_convert_item_to_guides(SPItem *item) {
    g_return_if_fail(item != NULL);
    g_return_if_fail(SP_IS_ITEM(item));

    /* Use derived method if present ... */
    if (((SPItemClass *) G_OBJECT_GET_CLASS(item))->convert_to_guides) {
        (*((SPItemClass *) G_OBJECT_GET_CLASS(item))->convert_to_guides)(item);
        return;
    }

    /* .. otherwise simply place the guides around the item's bounding box */

    sp_item_convert_to_guides(item);
}


/**
 * \pre \a ancestor really is an ancestor (\>=) of \a object, or NULL.
 *   ("Ancestor (\>=)" here includes as far as \a object itself.)
 */
NR::Matrix
i2anc_affine(SPObject const *object, SPObject const *const ancestor) {
    NR::Matrix ret(NR::identity());
    g_return_val_if_fail(object != NULL, ret);

    /* stop at first non-renderable ancestor */
    while ( object != ancestor && SP_IS_ITEM(object) ) {
        if (SP_IS_ROOT(object)) {
            ret *= SP_ROOT(object)->c2p;
        }
        ret *= SP_ITEM(object)->transform;
        object = SP_OBJECT_PARENT(object);
    }
    return ret;
}

NR::Matrix
i2i_affine(SPObject const *src, SPObject const *dest) {
    g_return_val_if_fail(src != NULL && dest != NULL, NR::identity());
    SPObject const *ancestor = src->nearestCommonAncestor(dest);
    return i2anc_affine(src, ancestor) / i2anc_affine(dest, ancestor);
}

NR::Matrix SPItem::getRelativeTransform(SPObject const *dest) const {
    return i2i_affine(this, dest);
}

/**
 * Returns the accumulated transformation of the item and all its ancestors, including root's viewport.
 * \pre (item != NULL) and SP_IS_ITEM(item).
 */
NR::Matrix sp_item_i2doc_affine(SPItem const *item)
{
    return i2anc_affine(item, NULL);
}

/**
 * Returns the accumulated transformation of the item and all its ancestors, but excluding root's viewport.
 * Used in path operations mostly.
 * \pre (item != NULL) and SP_IS_ITEM(item).
 */
NR::Matrix sp_item_i2root_affine(SPItem const *item)
{
    g_assert(item != NULL);
    g_assert(SP_IS_ITEM(item));

    NR::Matrix ret(NR::identity());
    g_assert(ret.test_identity());
    while ( NULL != SP_OBJECT_PARENT(item) ) {
        ret *= item->transform;
        item = SP_ITEM(SP_OBJECT_PARENT(item));
    }
    g_assert(SP_IS_ROOT(item));

    ret *= item->transform;

    return ret;
}

/* fixme: This is EVIL!!! */

NR::Matrix sp_item_i2d_affine(SPItem const *item)
{
    g_assert(item != NULL);
    g_assert(SP_IS_ITEM(item));

    NR::Matrix const ret( sp_item_i2doc_affine(item)
                          * NR::scale(1, -1)
                          * NR::translate(0, sp_document_height(SP_OBJECT_DOCUMENT(item))) );
    return ret;
}

// same as i2d but with i2root instead of i2doc
NR::Matrix sp_item_i2r_affine(SPItem const *item)
{
    g_assert(item != NULL);
    g_assert(SP_IS_ITEM(item));

    NR::Matrix const ret( sp_item_i2root_affine(item)
                          * NR::scale(1, -1)
                          * NR::translate(0, sp_document_height(SP_OBJECT_DOCUMENT(item))) );
    return ret;
}

/**
 * Converts a matrix \a m into the desktop coords of the \a item.
 * Will become a noop when we eliminate the coordinate flipping.
 */
NR::Matrix matrix_to_desktop(NR::Matrix const m, SPItem const *item)
{
    NR::Matrix const ret(m
                         * NR::translate(0, -sp_document_height(SP_OBJECT_DOCUMENT(item)))
                         * NR::scale(1, -1));
    return ret;
}

/**
 * Converts a matrix \a m from the desktop coords of the \a item.
 * Will become a noop when we eliminate the coordinate flipping.
 */
NR::Matrix matrix_from_desktop(NR::Matrix const m, SPItem const *item)
{
    NR::Matrix const ret(NR::scale(1, -1)
                         * NR::translate(0, sp_document_height(SP_OBJECT_DOCUMENT(item)))
                         * m);
    return ret;
}

void sp_item_set_i2d_affine(SPItem *item, NR::Matrix const &i2dt)
{
    g_return_if_fail( item != NULL );
    g_return_if_fail( SP_IS_ITEM(item) );

    NR::Matrix dt2p; /* desktop to item parent transform */
    if (SP_OBJECT_PARENT(item)) {
        dt2p = sp_item_i2d_affine((SPItem *) SP_OBJECT_PARENT(item)).inverse();
    } else {
        dt2p = ( NR::translate(0, -sp_document_height(SP_OBJECT_DOCUMENT(item)))
                 * NR::scale(1, -1) );
    }

    NR::Matrix const i2p( i2dt * dt2p );
    sp_item_set_item_transform(item, i2p);
}


NR::Matrix
sp_item_dt2i_affine(SPItem const *item)
{
    /* fixme: Implement the right way (Lauris) */
    return sp_item_i2d_affine(item).inverse();
}

/* Item views */

static SPItemView *
sp_item_view_new_prepend(SPItemView *list, SPItem *item, unsigned flags, unsigned key, NRArenaItem *arenaitem)
{
    SPItemView *new_view;

    g_assert(item != NULL);
    g_assert(SP_IS_ITEM(item));
    g_assert(arenaitem != NULL);
    g_assert(NR_IS_ARENA_ITEM(arenaitem));

    new_view = g_new(SPItemView, 1);

    new_view->next = list;
    new_view->flags = flags;
    new_view->key = key;
    new_view->arenaitem = nr_arena_item_ref(arenaitem);

    return new_view;
}

static SPItemView *
sp_item_view_list_remove(SPItemView *list, SPItemView *view)
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
NRArenaItem *
sp_item_get_arenaitem(SPItem *item, unsigned key)
{
    for ( SPItemView *iv = item->display ; iv ; iv = iv->next ) {
        if ( iv->key == key ) {
            return iv->arenaitem;
        }
    }

    return NULL;
}

int
sp_item_repr_compare_position(SPItem *first, SPItem *second)
{
    return sp_repr_compare_position(SP_OBJECT_REPR(first),
                                    SP_OBJECT_REPR(second));
}

SPItem *
sp_item_first_item_child (SPObject *obj)
{
    for ( SPObject *iter = sp_object_first_child(obj) ; iter ; iter = SP_OBJECT_NEXT(iter)) {
        if (SP_IS_ITEM (iter))
            return SP_ITEM (iter);
    }
    return NULL;
}

void
sp_item_convert_to_guides(SPItem *item) {
    SPDesktop *dt = inkscape_active_desktop();
    SPNamedView *nv = sp_desktop_namedview(dt);
    (void)nv;

    int prefs_bbox = prefs_get_int_attribute("tools", "bounding_box", 0);
    SPItem::BBoxType bbox_type = (prefs_bbox ==0)? 
        SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;

    NR::Maybe<NR::Rect> bbox = sp_item_bbox_desktop(item, bbox_type);
    if (!bbox) {
        g_warning ("Cannot determine item's bounding box during conversion to guides.\n");
        return;
    }

    std::list<std::pair<Geom::Point, Geom::Point> > pts;

    NR::Point A((*bbox).min());
    NR::Point C((*bbox).max());
    NR::Point B(A[NR::X], C[NR::Y]);
    NR::Point D(C[NR::X], A[NR::Y]);

    pts.push_back(std::make_pair(A.to_2geom(), B.to_2geom()));
    pts.push_back(std::make_pair(B.to_2geom(), C.to_2geom()));
    pts.push_back(std::make_pair(C.to_2geom(), D.to_2geom()));
    pts.push_back(std::make_pair(D.to_2geom(), A.to_2geom()));

    sp_guide_pt_pairs_to_guides(SP_OBJECT_DOCUMENT(item), pts);
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

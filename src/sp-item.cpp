#define __SP_ITEM_C__

/** \file
 * Base class for visual SVG elements
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2001-2005 authors
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
#include "gradient-chemistry.h"
#include "prefs-utils.h"
#include "conn-avoid-ref.h"

#include "libnr/nr-matrix-div.h"
#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-matrix-scale-ops.h"
#include "libnr/nr-matrix-translate-ops.h"
#include "libnr/nr-scale-translate-ops.h"
#include "libnr/nr-translate-scale-ops.h"
#include "algorithms/find-last-if.h"
#include "util/reverse-list.h"

#include "xml/repr.h"

#define noSP_ITEM_DEBUG_IDLE

static void sp_item_class_init(SPItemClass *klass);
static void sp_item_init(SPItem *item);

static void sp_item_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_item_release(SPObject *object);
static void sp_item_set(SPObject *object, unsigned key, gchar const *value);
static void sp_item_update(SPObject *object, SPCtx *ctx, guint flags);
static Inkscape::XML::Node *sp_item_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

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
    SPObject *object = SP_OBJECT(item);

    item->sensitive = TRUE;

    item->transform_center_x = 0;
    item->transform_center_y = 0;

    item->transform = NR::identity();

    item->display = NULL;

    item->clip_ref = new SPClipPathReference(SP_OBJECT(item));
		{
			sigc::signal<void, SPObject *, SPObject *> cs1=item->clip_ref->changedSignal();
			sigc::slot2<void,SPObject*, SPObject *> sl1=sigc::bind(sigc::ptr_fun(clip_ref_changed), item);
			cs1.connect(sl1);
		}

    item->mask_ref = new SPMaskReference(SP_OBJECT(item));
		sigc::signal<void, SPObject *, SPObject *> cs2=item->mask_ref->changedSignal();
		sigc::slot2<void,SPObject*, SPObject *> sl2=sigc::bind(sigc::ptr_fun(mask_ref_changed), item);
    cs2.connect(sl2);

    if (!object->style) object->style = sp_style_new_from_object(SP_OBJECT(item));

    item->avoidRef = new SPAvoidRef(item);

    new (&item->_transformed_signal) sigc::signal<void, NR::Matrix const *, SPItem *>();
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
    NR::Rect bbox = invokeBbox(sp_item_i2d_affine(this));
    if (!bbox.isEmpty()) {
        transform_center_x = object_centre[NR::X] - bbox.midpoint()[NR::X];
        if (fabs(transform_center_x) < 1e-5) // rounding error
            transform_center_x = 0;
        transform_center_y = object_centre[NR::Y] - bbox.midpoint()[NR::Y];
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

NR::Point SPItem::getCenter() {
    NR::Rect bbox = invokeBbox(sp_item_i2d_affine(this));
    if (!bbox.isEmpty()) {
        return bbox.midpoint() + NR::Point (this->transform_center_x, this->transform_center_y);
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

    if (item->avoidRef) {
        delete item->avoidRef;
        item->avoidRef = NULL;
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
            gchar *uri = Inkscape::parse_css_url(value);
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
            gchar *uri=Inkscape::parse_css_url(value);
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
        case SP_ATTR_STYLE:
            sp_style_read_from_object(object->style, object);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
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
            break;
        case SP_ATTR_TRANSFORM_CENTER_Y:
            if (value) {
                item->transform_center_y = g_strtod(value, NULL);
            } else {
                item->transform_center_y = 0;
            }
            break;
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

        SPClipPath *clip_path = item->clip_ref->getObject();
        SPMask *mask = item->mask_ref->getObject();

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

    // Update libavoid with item geometry (for connector routing).
    item->avoidRef->handleSettingChange();
}

static Inkscape::XML::Node *
sp_item_write(SPObject *const object, Inkscape::XML::Node *repr, guint flags)
{
    SPItem *item = SP_ITEM(object);

    gchar c[256];
    if (sp_svg_transform_write(c, 256, item->transform)) {
        repr->setAttribute("transform", c);
    } else {
        repr->setAttribute("transform", NULL);
    }

    SPObject const *const parent = SP_OBJECT_PARENT(object);
    /** \todo Can someone please document why this is conditional on having
     * a parent? The only parentless thing I can think of is the top-level
     * <svg> element (SPRoot). SPRoot is derived from SPGroup, and can have
     * style.  I haven't looked at callers.
     */
    if (parent) {
        SPStyle const *const obj_style = SP_OBJECT_STYLE(object);
        if (obj_style) {
            gchar *s = sp_style_write_string(obj_style, SP_STYLE_FLAG_IFSET);
            repr->setAttribute("style", ( *s ? s : NULL ));
            g_free(s);
        } else {
            /** \todo I'm not sure what to do in this case.  Bug #1165868
             * suggests that it can arise, but the submitter doesn't know
             * how to do so reliably.  The main two options are either
             * leave repr's style attribute unchanged, or explicitly clear it.
             * Must also consider what to do with property attributes for
             * the element; see below.
             */
            char const *style_str = repr->attribute("style");
            if (!style_str) {
                style_str = "NULL";
            }
            g_warning("Item's style is NULL; repr style attribute is %s", style_str);
        }

        /** \note We treat object->style as authoritative.  Its effects have
         * been written to the style attribute above; any properties that are
         * unset we take to be deliberately unset (e.g. so that clones can
         * override the property).
         *
         * Note that the below has an undesirable consequence of changing the
         * appearance on renderers that lack CSS support (e.g. SVG tiny);
         * possibly we should write property attributes instead of a style
         * attribute.
         */
        sp_style_unset_property_attrs (object);
    }

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

    if (((SPObjectClass *) (parent_class))->write) {
        ((SPObjectClass *) (parent_class))->write(object, repr, flags);
    }

    return repr;
}

NR::Rect SPItem::invokeBbox(NR::Matrix const &transform) const
{
    NRRect r;
    sp_item_invoke_bbox_full(this, &r, transform, 0, TRUE);
    return NR::Rect(r);
}

void
sp_item_invoke_bbox(SPItem const *item, NRRect *bbox, NR::Matrix const &transform, unsigned const clear)
{
    sp_item_invoke_bbox_full(item, bbox, transform, 0, clear);
}

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

    if (((SPItemClass *) G_OBJECT_GET_CLASS(item))->bbox) {
        ((SPItemClass *) G_OBJECT_GET_CLASS(item))->bbox(item, bbox, transform, flags);
    }

    // crop the bbox by clip path, if any
    if (item->clip_ref->getObject()) {
        NRRect b;
        sp_clippath_get_bbox(SP_CLIPPATH(item->clip_ref->getObject()), &b, transform, flags);
        nr_rect_d_intersect (bbox, bbox, &b);
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
sp_item_bbox_desktop(SPItem *item, NRRect *bbox)
{
    g_assert(item != NULL);
    g_assert(SP_IS_ITEM(item));
    g_assert(bbox != NULL);

    sp_item_invoke_bbox(item, bbox, sp_item_i2d_affine(item), TRUE);
}

NR::Rect sp_item_bbox_desktop(SPItem *item)
{
    NRRect ret;
    sp_item_bbox_desktop(item, &ret);
    return NR::Rect(ret);
}

static void sp_item_private_snappoints(SPItem const *item, SnapPointsIter p)
{
    NR::Rect const bbox = item->invokeBbox(sp_item_i2d_affine(item));
    /* Just a pair of opposite corners of the bounding box suffices given that we don't yet
       support angled guide lines. */

    *p = bbox.min();
    *p = bbox.max();
}

void sp_item_snappoints(SPItem const *item, SnapPointsIter p)
{
    g_assert (item != NULL);
    g_assert (SP_IS_ITEM(item));

    SPItemClass const &item_class = *(SPItemClass const *) G_OBJECT_GET_CLASS(item);
    if (item_class.snappoints) {
        item_class.snappoints(item, p);
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
sp_item_private_description(SPItem *item)
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
        return ((SPItemClass *) G_OBJECT_GET_CLASS(item))->description(item);
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
            NRArenaItem *ac;
            if (!item->display->arenaitem->key) NR_ARENA_ITEM_SET_KEY(item->display->arenaitem, sp_item_display_key_new(3));
            ac = sp_clippath_show(item->clip_ref->getObject(), arena, NR_ARENA_ITEM_GET_KEY(item->display->arenaitem));
            nr_arena_item_set_clip(ai, ac);
            nr_arena_item_unref(ac);
        }
        if (item->mask_ref->getObject()) {
            NRArenaItem *ac;
            if (!item->display->arenaitem->key) NR_ARENA_ITEM_SET_KEY(item->display->arenaitem, sp_item_display_key_new(3));
            ac = sp_mask_show(item->mask_ref->getObject(), arena, NR_ARENA_ITEM_GET_KEY(item->display->arenaitem));
            nr_arena_item_set_mask(ai, ac);
            nr_arena_item_unref(ac);
        }
        NR_ARENA_ITEM_SET_DATA(ai, item);
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

    if (style && (style->fill.type == SP_PAINT_TYPE_PAINTSERVER)) {
        SPObject *server = SP_OBJECT_STYLE_FILL_SERVER (item);
        if (SP_IS_PATTERN (server)) {
            SPPattern *pattern = sp_pattern_clone_if_necessary (item, SP_PATTERN (server), "fill");
            sp_pattern_transform_multiply (pattern, postmul, set);
        }
    }

    if (style && (style->stroke.type == SP_PAINT_TYPE_PAINTSERVER)) {
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

    if (style && (style->fill.type == SP_PAINT_TYPE_PAINTSERVER)) {
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

    if (style && (style->stroke.type == SP_PAINT_TYPE_PAINTSERVER)) {
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

    if (style && style->stroke.type != SP_PAINT_TYPE_NONE && !NR_DF_TEST_CLOSE (ex, 1.0, NR_EPSILON)) {

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
// A clone must not touch the style (it's that of its parent!) and has no children, so quit now
    if (item && SP_IS_USE(item))
        return;

// _Before_ full pattern/gradient transform: t_paint * t_item * t_ancestors
// _After_ full pattern/gradient transform: t_paint_new * t_item * t_ancestors * advertised_transform
// By equating these two expressions we get t_paint_new = t_paint * paint_delta, where:
    NR::Matrix t_item = sp_item_transform_repr (item);
    NR::Matrix paint_delta = t_item * t_ancestors * advertized_transform * t_ancestors.inverse() * t_item.inverse();

    if (is_pattern)
        sp_item_adjust_pattern (item, paint_delta);
    else
        sp_item_adjust_gradient (item, paint_delta);

// Within text, we do not fork gradients, and so must not recurse to avoid double compensation
    if (item && SP_IS_TEXT(item))
        return;

    for (SPObject *o = SP_OBJECT(item)->children; o != NULL; o = o->next) {
        if (SP_IS_ITEM(o)) {
// At the level of the transformed item, t_ancestors is identity;
// below it, it is the accmmulated chain of transforms from this level to the top level
            sp_item_adjust_paint_recursive (SP_ITEM(o), advertized_transform, t_item * t_ancestors, is_pattern);
        }
    }
}

/**
 * A temporary wrapper for the next function accepting the NRMatrix
 * instead of NR::Matrix
 */
void
sp_item_write_transform(SPItem *item, Inkscape::XML::Node *repr, NRMatrix const *transform, NR::Matrix const *adv)
{
    if (transform == NULL)
        sp_item_write_transform(item, repr, NR::identity(), adv);
    else
        sp_item_write_transform(item, repr, NR::Matrix (transform), adv);
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
sp_item_write_transform(SPItem *item, Inkscape::XML::Node *repr, NR::Matrix const &transform, NR::Matrix const *adv)
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

    // run the object's set_transform if transforms are stored optimized
    gint preserve = prefs_get_int_attribute("options.preservetransform", "value", 0);
    NR::Matrix transform_attr (transform);
    if (((SPItemClass *) G_OBJECT_GET_CLASS(item))->set_transform && !preserve) {
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

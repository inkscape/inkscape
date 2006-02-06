#define __SP_CLIPPATH_C__

/*
 * SVG <clipPath> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */



#include "display/nr-arena.h"
#include "display/nr-arena-group.h"
#include "xml/repr.h"

#include "enums.h"
#include "attributes.h"
#include "document.h"
#include "sp-item.h"

#include "sp-clippath.h"

struct SPClipPathView {
    SPClipPathView *next;
    unsigned int key;
    NRArenaItem *arenaitem;
    NRRect bbox;
};

static void sp_clippath_class_init(SPClipPathClass *klass);
static void sp_clippath_init(SPClipPath *clippath);

static void sp_clippath_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_clippath_release(SPObject * object);
static void sp_clippath_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_clippath_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
static void sp_clippath_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_clippath_modified(SPObject *object, guint flags);
static Inkscape::XML::Node *sp_clippath_write(SPObject *object, Inkscape::XML::Node *repr, guint flags);

SPClipPathView *sp_clippath_view_new_prepend(SPClipPathView *list, unsigned int key, NRArenaItem *arenaitem);
SPClipPathView *sp_clippath_view_list_remove(SPClipPathView *list, SPClipPathView *view);

static SPObjectGroupClass *parent_class;

GType
sp_clippath_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPClipPathClass),
            NULL, NULL,
            (GClassInitFunc) sp_clippath_class_init,
            NULL, NULL,
            sizeof(SPClipPath),
            16,
            (GInstanceInitFunc) sp_clippath_init,
            NULL,	/* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECTGROUP, "SPClipPath", &info, (GTypeFlags)0);
    }
    return type;
}

static void
sp_clippath_class_init(SPClipPathClass *klass)
{
    GObjectClass *gobject_class;
    SPObjectClass *sp_object_class;

    gobject_class = (GObjectClass *) klass;
    sp_object_class = (SPObjectClass *) klass;

    parent_class = (SPObjectGroupClass*)g_type_class_ref(SP_TYPE_OBJECTGROUP);

    sp_object_class->build = sp_clippath_build;
    sp_object_class->release = sp_clippath_release;
    sp_object_class->set = sp_clippath_set;
    sp_object_class->child_added = sp_clippath_child_added;
    sp_object_class->update = sp_clippath_update;
    sp_object_class->modified = sp_clippath_modified;
    sp_object_class->write = sp_clippath_write;
}

static void
sp_clippath_init(SPClipPath *cp)
{
    cp->clipPathUnits_set = FALSE;
    cp->clipPathUnits = SP_CONTENT_UNITS_USERSPACEONUSE;

    cp->display = NULL;
}

static void
sp_clippath_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    SPClipPath *cp;

    cp = SP_CLIPPATH(object);

    if (((SPObjectClass *) parent_class)->build)
        ((SPObjectClass *) parent_class)->build(object, document, repr);

    sp_object_read_attr(object, "clipPathUnits");

    /* Register ourselves */
    sp_document_add_resource(document, "clipPath", object);
}

static void
sp_clippath_release(SPObject * object)
{
    SPClipPath *cp;

    cp = SP_CLIPPATH(object);

    if (SP_OBJECT_DOCUMENT(object)) {
        /* Unregister ourselves */
        sp_document_remove_resource(SP_OBJECT_DOCUMENT(object), "clipPath", object);
    }

    while (cp->display) {
        /* We simply unref and let item to manage this in handler */
        cp->display = sp_clippath_view_list_remove(cp->display, cp->display);
    }

    if (((SPObjectClass *) (parent_class))->release)
        ((SPObjectClass *) parent_class)->release(object);
}

static void
sp_clippath_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPClipPath *cp;

    cp = SP_CLIPPATH(object);

    switch (key) {
	case SP_ATTR_CLIPPATHUNITS:
            cp->clipPathUnits = SP_CONTENT_UNITS_USERSPACEONUSE;
            cp->clipPathUnits_set = FALSE;
            if (value) {
                if (!strcmp(value, "userSpaceOnUse")) {
                    cp->clipPathUnits_set = TRUE;
                } else if (!strcmp(value, "objectBoundingBox")) {
                    cp->clipPathUnits = SP_CONTENT_UNITS_OBJECTBOUNDINGBOX;
                    cp->clipPathUnits_set = TRUE;
                }
            }
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
	default:
            if (((SPObjectClass *) parent_class)->set)
                ((SPObjectClass *) parent_class)->set(object, key, value);
            break;
    }
}

static void
sp_clippath_child_added(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    SPClipPath *cp;
    SPObject *ochild;

    cp = SP_CLIPPATH(object);

    /* Invoke SPObjectGroup implementation */
    ((SPObjectClass *) (parent_class))->child_added(object, child, ref);

    /* Show new object */
    ochild = SP_OBJECT_DOCUMENT(object)->getObjectByRepr(child);
    if (SP_IS_ITEM(ochild)) {
        SPClipPathView *v;
        for (v = cp->display; v != NULL; v = v->next) {
            NRArenaItem *ac;
            ac = sp_item_invoke_show(SP_ITEM(ochild), NR_ARENA_ITEM_ARENA(v->arenaitem), v->key, SP_ITEM_REFERENCE_FLAGS);
            if (ac) {
                nr_arena_item_add_child(v->arenaitem, ac, NULL);
                nr_arena_item_unref(ac);
            }
        }
    }
}

static void
sp_clippath_update(SPObject *object, SPCtx *ctx, guint flags)
{
    SPObjectGroup *og;
    SPClipPath *cp;
    SPObject *child;
    SPClipPathView *v;
    GSList *l;

    og = SP_OBJECTGROUP(object);
    cp = SP_CLIPPATH(object);

    if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    l = NULL;
    for (child = sp_object_first_child(SP_OBJECT(og)); child != NULL; child = SP_OBJECT_NEXT(child)) {
        g_object_ref(G_OBJECT(child));
        l = g_slist_prepend(l, child);
    }
    l = g_slist_reverse(l);
    while (l) {
        child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);
        if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->updateDisplay(ctx, flags);
        }
        g_object_unref(G_OBJECT(child));
    }

    for (v = cp->display; v != NULL; v = v->next) {
        if (cp->clipPathUnits == SP_CONTENT_UNITS_OBJECTBOUNDINGBOX) {
            NRMatrix t;
            nr_matrix_set_scale(&t, v->bbox.x1 - v->bbox.x0, v->bbox.y1 - v->bbox.y0);
            t.c[4] = v->bbox.x0;
            t.c[5] = v->bbox.y0;
            nr_arena_group_set_child_transform(NR_ARENA_GROUP(v->arenaitem), &t);
        } else {
            nr_arena_group_set_child_transform(NR_ARENA_GROUP(v->arenaitem), NULL);
        }
    }
}

static void
sp_clippath_modified(SPObject *object, guint flags)
{
    SPObjectGroup *og;
    SPClipPath *cp;
    SPObject *child;
    GSList *l;

    og = SP_OBJECTGROUP(object);
    cp = SP_CLIPPATH(object);

    if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    l = NULL;
    for (child = sp_object_first_child(SP_OBJECT(og)); child != NULL; child = SP_OBJECT_NEXT(child)) {
        g_object_ref(G_OBJECT(child));
        l = g_slist_prepend(l, child);
    }
    l = g_slist_reverse(l);
    while (l) {
        child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);
        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }
        g_object_unref(G_OBJECT(child));
    }
}

static Inkscape::XML::Node *
sp_clippath_write(SPObject *object, Inkscape::XML::Node *repr, guint flags)
{
    SPClipPath *cp;

    cp = SP_CLIPPATH(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = sp_repr_new("svg:clipPath");
    }

    if (((SPObjectClass *) (parent_class))->write)
        ((SPObjectClass *) (parent_class))->write(object, repr, flags);

    return repr;
}

NRArenaItem *
sp_clippath_show(SPClipPath *cp, NRArena *arena, unsigned int key)
{
    NRArenaItem *ai, *ac;
    SPObject *child;

    g_return_val_if_fail(cp != NULL, NULL);
    g_return_val_if_fail(SP_IS_CLIPPATH(cp), NULL);
    g_return_val_if_fail(arena != NULL, NULL);
    g_return_val_if_fail(NR_IS_ARENA(arena), NULL);

    ai = NRArenaGroup::create(arena);
    cp->display = sp_clippath_view_new_prepend(cp->display, key, ai);

    for (child = sp_object_first_child(SP_OBJECT(cp)) ; child != NULL; child = SP_OBJECT_NEXT(child)) {
        if (SP_IS_ITEM(child)) {
            ac = sp_item_invoke_show(SP_ITEM(child), arena, key, SP_ITEM_REFERENCE_FLAGS);
            if (ac) {
                /* The order is not important in clippath */
                nr_arena_item_add_child(ai, ac, NULL);
                nr_arena_item_unref(ac);
            }
        }
    }

    if (cp->clipPathUnits == SP_CONTENT_UNITS_OBJECTBOUNDINGBOX) {
        NRMatrix t;
        nr_matrix_set_scale(&t, cp->display->bbox.x1 - cp->display->bbox.x0, cp->display->bbox.y1 - cp->display->bbox.y0);
        t.c[4] = cp->display->bbox.x0;
        t.c[5] = cp->display->bbox.y0;
        nr_arena_group_set_child_transform(NR_ARENA_GROUP(ai), &t);
    }

    return ai;
}

void
sp_clippath_hide(SPClipPath *cp, unsigned int key)
{
    SPClipPathView *v;
    SPObject *child;

    g_return_if_fail(cp != NULL);
    g_return_if_fail(SP_IS_CLIPPATH(cp));

    for (child = sp_object_first_child(SP_OBJECT(cp)) ; child != NULL; child = SP_OBJECT_NEXT(child)) {
        if (SP_IS_ITEM(child)) {
            sp_item_invoke_hide(SP_ITEM(child), key);
        }
    }

    for (v = cp->display; v != NULL; v = v->next) {
        if (v->key == key) {
            /* We simply unref and let item to manage this in handler */
            cp->display = sp_clippath_view_list_remove(cp->display, v);
            return;
        }
    }

    g_assert_not_reached();
}

void
sp_clippath_set_bbox(SPClipPath *cp, unsigned int key, NRRect *bbox)
{
    SPClipPathView *v;

    for (v = cp->display; v != NULL; v = v->next) {
        if (v->key == key) {
            if (!NR_DF_TEST_CLOSE(v->bbox.x0, bbox->x0, NR_EPSILON) ||
                !NR_DF_TEST_CLOSE(v->bbox.y0, bbox->y0, NR_EPSILON) ||
                !NR_DF_TEST_CLOSE(v->bbox.x1, bbox->x1, NR_EPSILON) ||
                !NR_DF_TEST_CLOSE(v->bbox.y1, bbox->y1, NR_EPSILON)) {
                v->bbox = *bbox;
            }
            break;
        }
    }
}

void
sp_clippath_get_bbox(SPClipPath *cp, NRRect *bbox, NR::Matrix const &transform, unsigned const flags)
{
	for (SPObject *o = sp_object_first_child(SP_OBJECT(cp)); o != NULL; o = SP_OBJECT_NEXT(o)) {
		if (SP_IS_ITEM(o)) {
			SPItem *child = SP_ITEM(o);
			sp_item_invoke_bbox_full(child, bbox, transform, flags, FALSE);
		}
	}
}

/* ClipPath views */

SPClipPathView *
sp_clippath_view_new_prepend(SPClipPathView *list, unsigned int key, NRArenaItem *arenaitem)
{
    SPClipPathView *new_path_view;

    new_path_view = g_new(SPClipPathView, 1);

    new_path_view->next = list;
    new_path_view->key = key;
    new_path_view->arenaitem = nr_arena_item_ref(arenaitem);
    new_path_view->bbox.x0 = new_path_view->bbox.x1 = 0.0;
    new_path_view->bbox.y0 = new_path_view->bbox.y1 = 0.0;

    return new_path_view;
}

SPClipPathView *
sp_clippath_view_list_remove(SPClipPathView *list, SPClipPathView *view)
{
    if (view == list) {
        list = list->next;
    } else {
        SPClipPathView *prev;
        prev = list;
        while (prev->next != view) prev = prev->next;
        prev->next = view->next;
    }

    nr_arena_item_unref(view->arenaitem);
    g_free(view);

    return list;
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

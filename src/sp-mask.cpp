#define __SP_MASK_C__

/*
 * SVG <mask> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2003 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <string>

#include "display/nr-arena.h"
#include "display/nr-arena-group.h"
#include "libnr/nr-matrix-ops.h"
#include <xml/repr.h>

#include "enums.h"
#include "attributes.h"
#include "document.h"
#include "document-private.h"
#include "sp-item.h"

#include "sp-mask.h"

struct SPMaskView {
	SPMaskView *next;
	unsigned int key;
	NRArenaItem *arenaitem;
	NRRect bbox;
};

static void sp_mask_class_init (SPMaskClass *klass);
static void sp_mask_init (SPMask *mask);

static void sp_mask_build (SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void sp_mask_release (SPObject * object);
static void sp_mask_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_mask_child_added (SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
static void sp_mask_update (SPObject *object, SPCtx *ctx, guint flags);
static void sp_mask_modified (SPObject *object, guint flags);
static Inkscape::XML::Node *sp_mask_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

SPMaskView *sp_mask_view_new_prepend (SPMaskView *list, unsigned int key, NRArenaItem *arenaitem);
SPMaskView *sp_mask_view_list_remove (SPMaskView *list, SPMaskView *view);

static SPObjectGroupClass *parent_class;

GType
sp_mask_get_type (void)
{
	static GType type = 0;
	if (!type) {
		GTypeInfo info = {
			sizeof (SPMaskClass),
			NULL, NULL,
			(GClassInitFunc) sp_mask_class_init,
			NULL, NULL,
			sizeof (SPMask),
			16,
			(GInstanceInitFunc) sp_mask_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_OBJECTGROUP, "SPMask", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_mask_class_init (SPMaskClass *klass)
{
	parent_class = (SPObjectGroupClass*) g_type_class_ref (SP_TYPE_OBJECTGROUP);

	SPObjectClass *sp_object_class = (SPObjectClass *) klass;
	sp_object_class->build = sp_mask_build;
	sp_object_class->release = sp_mask_release;
	sp_object_class->set = sp_mask_set;
	sp_object_class->child_added = sp_mask_child_added;
	sp_object_class->update = sp_mask_update;
	sp_object_class->modified = sp_mask_modified;
	sp_object_class->write = sp_mask_write;
}

static void
sp_mask_init (SPMask *mask)
{
	mask->maskUnits_set = FALSE;
	mask->maskUnits = SP_CONTENT_UNITS_OBJECTBOUNDINGBOX;

	mask->maskContentUnits_set = FALSE;
	mask->maskContentUnits = SP_CONTENT_UNITS_USERSPACEONUSE;

	mask->display = NULL;
}

static void
sp_mask_build (SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
	if (((SPObjectClass *) parent_class)->build) {
		((SPObjectClass *) parent_class)->build (object, document, repr);
	}

	sp_object_read_attr (object, "maskUnits");
	sp_object_read_attr (object, "maskContentUnits");

	/* Register ourselves */
	sp_document_add_resource (document, "mask", object);
}

static void
sp_mask_release (SPObject * object)
{
	if (SP_OBJECT_DOCUMENT (object)) {
		/* Unregister ourselves */
		sp_document_remove_resource (SP_OBJECT_DOCUMENT (object), "mask", object);
	}

	SPMask *cp = SP_MASK (object);
	while (cp->display) {
		/* We simply unref and let item manage this in handler */
		cp->display = sp_mask_view_list_remove (cp->display, cp->display);
	}

	if (((SPObjectClass *) (parent_class))->release) {
		((SPObjectClass *) parent_class)->release (object);
	}
}

static void
sp_mask_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPMask *mask = SP_MASK (object);

	switch (key) {
	case SP_ATTR_MASKUNITS:
		mask->maskUnits = SP_CONTENT_UNITS_OBJECTBOUNDINGBOX;
		mask->maskUnits_set = FALSE;
		if (value) {
			if (!strcmp (value, "userSpaceOnUse")) {
				mask->maskUnits = SP_CONTENT_UNITS_USERSPACEONUSE;
				mask->maskUnits_set = TRUE;
			} else if (!strcmp (value, "objectBoundingBox")) {
				mask->maskUnits_set = TRUE;
			}
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_MASKCONTENTUNITS:
		mask->maskContentUnits = SP_CONTENT_UNITS_USERSPACEONUSE;
		mask->maskContentUnits_set = FALSE;
		if (value) {
			if (!strcmp (value, "userSpaceOnUse")) {
				mask->maskContentUnits_set = TRUE;
			} else if (!strcmp (value, "objectBoundingBox")) {
				mask->maskContentUnits = SP_CONTENT_UNITS_OBJECTBOUNDINGBOX;
				mask->maskContentUnits_set = TRUE;
			}
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	default:
		if (((SPObjectClass *) parent_class)->set)
			((SPObjectClass *) parent_class)->set (object, key, value);
		break;
	}
}

static void
sp_mask_child_added (SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
	/* Invoke SPObjectGroup implementation */
	((SPObjectClass *) (parent_class))->child_added (object, child, ref);

	/* Show new object */
	SPObject *ochild = SP_OBJECT_DOCUMENT (object)->getObjectByRepr(child);
	if (SP_IS_ITEM (ochild)) {
		SPMask *cp = SP_MASK (object);
		for (SPMaskView *v = cp->display; v != NULL; v = v->next) {
			NRArenaItem *ac = sp_item_invoke_show (SP_ITEM (ochild),
							       NR_ARENA_ITEM_ARENA (v->arenaitem),
							       v->key,
							       SP_ITEM_REFERENCE_FLAGS);
			if (ac) {
				nr_arena_item_add_child (v->arenaitem, ac, NULL);
				nr_arena_item_unref (ac);
			}
		}
	}
}

static void
sp_mask_update (SPObject *object, SPCtx *ctx, guint flags)
{
	if (flags & SP_OBJECT_MODIFIED_FLAG) {
		flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	}
	
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	SPObjectGroup *og = SP_OBJECTGROUP (object);
	GSList *l = NULL;
	for (SPObject *child = sp_object_first_child(SP_OBJECT(og)); child != NULL; child = SP_OBJECT_NEXT(child)) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		SPObject *child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			child->updateDisplay(ctx, flags);
		}
		g_object_unref (G_OBJECT (child));
	}

	SPMask *mask = SP_MASK (object);
	for (SPMaskView *v = mask->display; v != NULL; v = v->next) {
		if (mask->maskContentUnits == SP_CONTENT_UNITS_OBJECTBOUNDINGBOX) {
            NR::Matrix t(NR::scale(v->bbox.x1 - v->bbox.x0, v->bbox.y1 - v->bbox.y0));
			t[4] = v->bbox.x0;
			t[5] = v->bbox.y0;
			nr_arena_group_set_child_transform (NR_ARENA_GROUP (v->arenaitem), &t);
		} else {
			nr_arena_group_set_child_transform (NR_ARENA_GROUP (v->arenaitem), NULL);
		}
	}
}

static void
sp_mask_modified (SPObject *object, guint flags)
{
	if (flags & SP_OBJECT_MODIFIED_FLAG) {
		flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
	}
	
	flags &= SP_OBJECT_MODIFIED_CASCADE;

	SPObjectGroup *og = SP_OBJECTGROUP (object);
	GSList *l = NULL;
	for (SPObject *child = sp_object_first_child(SP_OBJECT(og)); child != NULL; child = SP_OBJECT_NEXT(child)) {
		g_object_ref (G_OBJECT (child));
		l = g_slist_prepend (l, child);
	}
	l = g_slist_reverse (l);
	while (l) {
		SPObject *child = SP_OBJECT (l->data);
		l = g_slist_remove (l, child);
		if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
			child->emitModified(flags);
		}
		g_object_unref (G_OBJECT (child));
	}
}

static Inkscape::XML::Node *
sp_mask_write (SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = xml_doc->createElement("svg:mask");
	}

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, xml_doc, repr, flags);

	return repr;
}

// Create a mask element (using passed elements), add it to <defs>
const gchar *
sp_mask_create (GSList *reprs, SPDocument *document, NR::Matrix const* applyTransform)
{
    Inkscape::XML::Node *defsrepr = SP_OBJECT_REPR (SP_DOCUMENT_DEFS (document));

    Inkscape::XML::Document *xml_doc = sp_document_repr_doc(document);
    Inkscape::XML::Node *repr = xml_doc->createElement("svg:mask");
    repr->setAttribute("maskUnits", "userSpaceOnUse");
    
    defsrepr->appendChild(repr);
    const gchar *mask_id = repr->attribute("id");
    SPObject *mask_object = document->getObjectById(mask_id);
    
    for (GSList *it = reprs; it != NULL; it = it->next) {
        Inkscape::XML::Node *node = (Inkscape::XML::Node *)(it->data);
        SPItem *item = SP_ITEM(mask_object->appendChildRepr(node));
        
        if (NULL != applyTransform) {
            NR::Matrix transform (item->transform);
            transform *= (*applyTransform);
            sp_item_write_transform(item, SP_OBJECT_REPR(item), transform);
        }
    }

    if (repr != defsrepr->lastChild())
        defsrepr->changeOrder(repr, defsrepr->lastChild()); // workaround for bug 989084
    
    Inkscape::GC::release(repr);
    return mask_id;
}

NRArenaItem *
sp_mask_show (SPMask *mask, NRArena *arena, unsigned int key)
{
	g_return_val_if_fail (mask != NULL, NULL);
	g_return_val_if_fail (SP_IS_MASK (mask), NULL);
	g_return_val_if_fail (arena != NULL, NULL);
	g_return_val_if_fail (NR_IS_ARENA (arena), NULL);

	NRArenaItem *ai = NRArenaGroup::create(arena);
	mask->display = sp_mask_view_new_prepend (mask->display, key, ai);

	for (SPObject *child = sp_object_first_child(SP_OBJECT(mask)) ; child != NULL; child = SP_OBJECT_NEXT(child)) {
		if (SP_IS_ITEM (child)) {
			NRArenaItem *ac = sp_item_invoke_show (SP_ITEM (child), arena, key, SP_ITEM_REFERENCE_FLAGS);
			if (ac) {
				/* The order is not important in mask */
				nr_arena_item_add_child (ai, ac, NULL);
				nr_arena_item_unref (ac);
			}
		}
	}

	if (mask->maskContentUnits == SP_CONTENT_UNITS_OBJECTBOUNDINGBOX) {
        NR::Matrix t(NR::scale(mask->display->bbox.x1 - mask->display->bbox.x0, mask->display->bbox.y1 - mask->display->bbox.y0));
		t[4] = mask->display->bbox.x0;
		t[5] = mask->display->bbox.y0;
		nr_arena_group_set_child_transform (NR_ARENA_GROUP (ai), &t);
	}

	return ai;
}

void
sp_mask_hide (SPMask *cp, unsigned int key)
{
	g_return_if_fail (cp != NULL);
	g_return_if_fail (SP_IS_MASK (cp));

	for (SPObject *child = sp_object_first_child(SP_OBJECT(cp)); child != NULL; child = SP_OBJECT_NEXT(child)) {
		if (SP_IS_ITEM (child)) {
			sp_item_invoke_hide (SP_ITEM (child), key);
		}
	}

	for (SPMaskView *v = cp->display; v != NULL; v = v->next) {
		if (v->key == key) {
			/* We simply unref and let item to manage this in handler */
			cp->display = sp_mask_view_list_remove (cp->display, v);
			return;
		}
	}

	g_assert_not_reached ();
}

void
sp_mask_set_bbox (SPMask *mask, unsigned int key, NRRect *bbox)
{
	for (SPMaskView *v = mask->display; v != NULL; v = v->next) {
		if (v->key == key) {
			if (!NR_DF_TEST_CLOSE (v->bbox.x0, bbox->x0, NR_EPSILON) ||
			    !NR_DF_TEST_CLOSE (v->bbox.y0, bbox->y0, NR_EPSILON) ||
			    !NR_DF_TEST_CLOSE (v->bbox.x1, bbox->x1, NR_EPSILON) ||
			    !NR_DF_TEST_CLOSE (v->bbox.y1, bbox->y1, NR_EPSILON)) {
				v->bbox = *bbox;
			}
			break;
		}
	}
}

/* Mask views */

SPMaskView *
sp_mask_view_new_prepend (SPMaskView *list, unsigned int key, NRArenaItem *arenaitem)
{
	SPMaskView *new_mask_view = g_new (SPMaskView, 1);

	new_mask_view->next = list;
	new_mask_view->key = key;
	new_mask_view->arenaitem = nr_arena_item_ref(arenaitem);
	new_mask_view->bbox.x0 = new_mask_view->bbox.x1 = 0.0;
	new_mask_view->bbox.y0 = new_mask_view->bbox.y1 = 0.0;

	return new_mask_view;
}

SPMaskView *
sp_mask_view_list_remove (SPMaskView *list, SPMaskView *view)
{
	if (view == list) {
		list = list->next;
	} else {
		SPMaskView *prev;
		prev = list;
		while (prev->next != view) prev = prev->next;
		prev->next = view->next;
	}

	nr_arena_item_unref (view->arenaitem);
	g_free (view);

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

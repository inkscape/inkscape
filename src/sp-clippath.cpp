/*
 * SVG <clipPath> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2001-2002 authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <string>

#include "display/drawing.h"
#include "display/drawing-group.h"
#include "xml/repr.h"

#include "enums.h"
#include "attributes.h"
#include "document.h"
#include "document-private.h"
#include "sp-item.h"
#include "style.h"

#include <2geom/transforms.h>

#include "sp-clippath.h"

struct SPClipPathView {
    SPClipPathView *next;
    unsigned int key;
    Inkscape::DrawingItem *arenaitem;
    Geom::OptRect bbox;
};

SPClipPathView *sp_clippath_view_new_prepend(SPClipPathView *list, unsigned int key, Inkscape::DrawingItem *arenaitem);
SPClipPathView *sp_clippath_view_list_remove(SPClipPathView *list, SPClipPathView *view);

SPObjectGroupClass * SPClipPathClass::static_parent_class = 0;

GType
SPClipPath::sp_clippath_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPClipPathClass),
            NULL, NULL,
            (GClassInitFunc) SPClipPathClass::sp_clippath_class_init,
            NULL, NULL,
            sizeof(SPClipPath),
            16,
            (GInstanceInitFunc) SPClipPath::init,
            NULL,       /* value_table */
        };
        type = g_type_register_static(SP_TYPE_OBJECTGROUP, "SPClipPath", &info, (GTypeFlags)0);
    }
    return type;
}

void SPClipPathClass::sp_clippath_class_init(SPClipPathClass *klass)
{
    SPObjectClass *sp_object_class = SP_OBJECT_CLASS(klass);
    static_parent_class            = SP_OBJECTGROUP_CLASS(g_type_class_ref(SP_TYPE_OBJECTGROUP));

    sp_object_class->build = SPClipPath::build;
    sp_object_class->release = SPClipPath::release;
    sp_object_class->set = SPClipPath::set;
    sp_object_class->child_added = SPClipPath::childAdded;
    sp_object_class->update = SPClipPath::update;
    sp_object_class->modified = SPClipPath::modified;
    sp_object_class->write = SPClipPath::write;
}

void SPClipPath::init(SPClipPath *cp)
{
    cp->clipPathUnits_set = FALSE;
    cp->clipPathUnits = SP_CONTENT_UNITS_USERSPACEONUSE;

    cp->display = NULL;
}

void SPClipPath::build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) SPClipPathClass::static_parent_class)->build)
        ((SPObjectClass *) SPClipPathClass::static_parent_class)->build(object, document, repr);

    object->readAttr( "style" );
    object->readAttr( "clipPathUnits" );

    /* Register ourselves */
    document->addResource("clipPath", object);
}

void SPClipPath::release(SPObject * object)
{
    if (object->document) {
        // Unregister ourselves
        object->document->removeResource("clipPath", object);
    }

    SPClipPath *cp = SP_CLIPPATH(object);
    while (cp->display) {
        /* We simply unref and let item manage this in handler */
        cp->display = sp_clippath_view_list_remove(cp->display, cp->display);
    }

    if (((SPObjectClass *) (SPClipPathClass::static_parent_class))->release) {
        ((SPObjectClass *) SPClipPathClass::static_parent_class)->release(object);
    }
}

void SPClipPath::set(SPObject *object, unsigned int key, gchar const *value)
{
    SPClipPath *cp = SP_CLIPPATH(object);

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
            if (SP_ATTRIBUTE_IS_CSS(key)) {
                sp_style_read_from_object(object->style, object);
                object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            } else {
                if (((SPObjectClass *) SPClipPathClass::static_parent_class)->set) {
                    ((SPObjectClass *) SPClipPathClass::static_parent_class)->set(object, key, value);
                }
            }
            break;
    }
}

void SPClipPath::childAdded(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    /* Invoke SPObjectGroup implementation */
    ((SPObjectClass *) (SPClipPathClass::static_parent_class))->child_added(object, child, ref);

    /* Show new object */
    SPObject *ochild = object->document->getObjectByRepr(child);
    if (SP_IS_ITEM(ochild)) {
        SPClipPath *cp = SP_CLIPPATH(object);
        for (SPClipPathView *v = cp->display; v != NULL; v = v->next) {
            Inkscape::DrawingItem *ac = SP_ITEM(ochild)->invoke_show(                                                  v->arenaitem->drawing(),
                                                  v->key,
                                                  SP_ITEM_REFERENCE_FLAGS);
            if (ac) {
                v->arenaitem->prependChild(ac);
            }
        }
    }
}

void SPClipPath::update(SPObject *object, SPCtx *ctx, guint flags)
{
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    SPObjectGroup *og = SP_OBJECTGROUP(object);
    GSList *l = NULL;
    for ( SPObject *child = og->firstChild(); child; child = child->getNext()) {
        g_object_ref(G_OBJECT(child));
        l = g_slist_prepend(l, child);
    }
    l = g_slist_reverse(l);
    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);
        if (flags || (child->uflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->updateDisplay(ctx, flags);
        }
        g_object_unref(G_OBJECT(child));
    }

    SPClipPath *cp = SP_CLIPPATH(object);
    for (SPClipPathView *v = cp->display; v != NULL; v = v->next) {
        Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
        if (cp->clipPathUnits == SP_CONTENT_UNITS_OBJECTBOUNDINGBOX && v->bbox) {
            Geom::Affine t = Geom::Scale(v->bbox->dimensions());
            t.setTranslation(v->bbox->min());
            g->setChildTransform(t);
        } else {
            g->setChildTransform(Geom::identity());
        }
    }
}

void SPClipPath::modified(SPObject *object, guint flags)
{
    if (flags & SP_OBJECT_MODIFIED_FLAG) {
        flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    }

    flags &= SP_OBJECT_MODIFIED_CASCADE;

    SPObjectGroup *og = SP_OBJECTGROUP(object);
    GSList *l = NULL;
    for (SPObject *child = og->firstChild(); child; child = child->getNext()) {
        g_object_ref(G_OBJECT(child));
        l = g_slist_prepend(l, child);
    }
    l = g_slist_reverse(l);
    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);
        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }
        g_object_unref(G_OBJECT(child));
    }
}

Inkscape::XML::Node *SPClipPath::write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:clipPath");
    }

    if (((SPObjectClass *) (SPClipPathClass::static_parent_class))->write) {
        ((SPObjectClass *) (SPClipPathClass::static_parent_class))->write(object, xml_doc, repr, flags);
    }

    return repr;
}

Inkscape::DrawingItem *SPClipPath::show(Inkscape::Drawing &drawing, unsigned int key)
{
    Inkscape::DrawingGroup *ai = new Inkscape::DrawingGroup(drawing);
    display = sp_clippath_view_new_prepend(display, key, ai);

    for ( SPObject *child = firstChild() ; child ; child = child->getNext() ) {
        if (SP_IS_ITEM(child)) {
            Inkscape::DrawingItem *ac = SP_ITEM(child)->invoke_show(drawing, key, SP_ITEM_REFERENCE_FLAGS);
            if (ac) {
                /* The order is not important in clippath */
                ai->appendChild(ac);
            }
        }
    }

    if (clipPathUnits == SP_CONTENT_UNITS_OBJECTBOUNDINGBOX && display->bbox) {
        Geom::Affine t = Geom::Scale(display->bbox->dimensions());
        t.setTranslation(display->bbox->min());
        ai->setChildTransform(t);
    }
    ai->setStyle(this->style);

    return ai;
}

void SPClipPath::hide(unsigned int key)
{
    for ( SPObject *child = firstChild() ; child; child = child->getNext() ) {
        if (SP_IS_ITEM(child)) {
            SP_ITEM(child)->invoke_hide(key);
        }
    }

    for (SPClipPathView *v = display; v != NULL; v = v->next) {
        if (v->key == key) {
            /* We simply unref and let item to manage this in handler */
            display = sp_clippath_view_list_remove(display, v);
            return;
        }
    }

    g_assert_not_reached();
}

void SPClipPath::setBBox(unsigned int key, Geom::OptRect const &bbox)
{
    for (SPClipPathView *v = display; v != NULL; v = v->next) {
        if (v->key == key) {
            v->bbox = bbox;
            break;
        }
    }
}

Geom::OptRect SPClipPath::geometricBounds(Geom::Affine const &transform)
{
    SPObject *i = 0;
    Geom::OptRect bbox;
    for (i = firstChild(); i; i = i->getNext()) {
        if (!SP_IS_ITEM(i)) continue;
        Geom::OptRect tmp = SP_ITEM(i)->geometricBounds(Geom::Affine(SP_ITEM(i)->transform) * transform);
        bbox.unionWith(tmp);
    }
    return bbox;
}

/* ClipPath views */

SPClipPathView *
sp_clippath_view_new_prepend(SPClipPathView *list, unsigned int key, Inkscape::DrawingItem *arenaitem)
{
    SPClipPathView *new_path_view = g_new(SPClipPathView, 1);

    new_path_view->next = list;
    new_path_view->key = key;
    new_path_view->arenaitem = arenaitem;
    new_path_view->bbox = Geom::OptRect();

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

    delete view->arenaitem;
    g_free(view);

    return list;
}

// Create a mask element (using passed elements), add it to <defs>
const gchar *SPClipPath::create (GSList *reprs, SPDocument *document, Geom::Affine const* applyTransform)
{
    Inkscape::XML::Node *defsrepr = document->getDefs()->getRepr();

    Inkscape::XML::Document *xml_doc = document->getReprDoc();
    Inkscape::XML::Node *repr = xml_doc->createElement("svg:clipPath");
    repr->setAttribute("clipPathUnits", "userSpaceOnUse");

    defsrepr->appendChild(repr);
    const gchar *id = repr->attribute("id");
    SPObject *clip_path_object = document->getObjectById(id);

    for (GSList *it = reprs; it != NULL; it = it->next) {
        Inkscape::XML::Node *node = (Inkscape::XML::Node *)(it->data);
        SPItem *item = SP_ITEM(clip_path_object->appendChildRepr(node));

        if (NULL != applyTransform) {
            Geom::Affine transform (item->transform);
            transform *= (*applyTransform);
            item->doWriteTransform(item->getRepr(), transform);
        }
    }

    Inkscape::GC::release(repr);
    return id;
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

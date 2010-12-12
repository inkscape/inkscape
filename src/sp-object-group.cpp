/*
 * Abstract base class for non-item groups
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2003 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-object-group.h"
#include "xml/repr.h"
#include "document.h"

SPObjectClass * SPObjectGroupClass::static_parent_class = 0;

GType SPObjectGroup::sp_objectgroup_get_type(void)
{
    static GType objectgroup_type = 0;
    if (!objectgroup_type) {
        GTypeInfo objectgroup_info = {
            sizeof(SPObjectGroupClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) SPObjectGroupClass::sp_objectgroup_class_init,
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof(SPObjectGroup),
            16,     /* n_preallocs */
            (GInstanceInitFunc) init,
            NULL,   /* value_table */
        };
        objectgroup_type = g_type_register_static(SP_TYPE_OBJECT, "SPObjectGroup", &objectgroup_info, (GTypeFlags)0);
    }
    return objectgroup_type;
}

void SPObjectGroupClass::sp_objectgroup_class_init(SPObjectGroupClass *klass)
{
    GObjectClass * object_class = (GObjectClass *) klass;
    SPObjectClass * sp_object_class = (SPObjectClass *) klass;

    static_parent_class = (SPObjectClass *)g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->child_added = SPObjectGroup::childAdded;
    sp_object_class->remove_child = SPObjectGroup::removeChild;
    sp_object_class->order_changed = SPObjectGroup::orderChanged;
    sp_object_class->write = SPObjectGroup::write;
}

void SPObjectGroup::init(SPObjectGroup * /*objectgroup*/)
{
}

void SPObjectGroup::childAdded(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    if (((SPObjectClass *) (SPObjectGroupClass::static_parent_class))->child_added) {
        (* ((SPObjectClass *) (SPObjectGroupClass::static_parent_class))->child_added)(object, child, ref);
    }

    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

void SPObjectGroup::removeChild(SPObject *object, Inkscape::XML::Node *child)
{
    if (((SPObjectClass *) (SPObjectGroupClass::static_parent_class))->remove_child) {
        (* ((SPObjectClass *) (SPObjectGroupClass::static_parent_class))->remove_child)(object, child);
    }

    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

void SPObjectGroup::orderChanged(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *old_ref, Inkscape::XML::Node *new_ref)
{
    if (((SPObjectClass *) (SPObjectGroupClass::static_parent_class))->order_changed) {
        (* ((SPObjectClass *) (SPObjectGroupClass::static_parent_class))->order_changed)(object, child, old_ref, new_ref);
    }

    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

Inkscape::XML::Node *SPObjectGroup::write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SP_OBJECTGROUP(object); // Ensure we have the right type of SPObject

    if (flags & SP_OBJECT_WRITE_BUILD) {
        if (!repr) {
            repr = xml_doc->createElement("svg:g");
        }
        GSList *l = 0;
        for ( SPObject *child = object->firstChild() ; child ; child = child->getNext() ) {
            Inkscape::XML::Node *crepr = child->updateRepr(xml_doc, NULL, flags);
            if (crepr) {
                l = g_slist_prepend(l, crepr);
            }
        }
        while (l) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove(l, l->data);
        }
    } else {
        for ( SPObject *child = object->firstChild() ; child ; child = child->getNext() ) {
            child->updateRepr(flags);
        }
    }

    if (((SPObjectClass *) (SPObjectGroupClass::static_parent_class))->write) {
        ((SPObjectClass *) (SPObjectGroupClass::static_parent_class))->write(object, xml_doc, repr, flags);
    }

    return repr;
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

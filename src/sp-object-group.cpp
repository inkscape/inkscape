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

static void sp_objectgroup_child_added(SPObject            *object,
                                       Inkscape::XML::Node *child,
                                       Inkscape::XML::Node *ref);

static void sp_objectgroup_remove_child(SPObject            *object,
                                        Inkscape::XML::Node *child);

static void sp_objectgroup_order_changed(SPObject            *object,
                                         Inkscape::XML::Node *child,
                                         Inkscape::XML::Node *old_ref,
                                         Inkscape::XML::Node *new_ref);

static Inkscape::XML::Node* sp_objectgroup_write(SPObject                *object,
                                                 Inkscape::XML::Document *doc,
                                                 Inkscape::XML::Node     *repr,
                                                 guint                    flags);

G_DEFINE_TYPE(SPObjectGroup, sp_objectgroup, SP_TYPE_OBJECT);

static void
sp_objectgroup_class_init(SPObjectGroupClass *klass)
{
    SPObjectClass * sp_object_class = SP_OBJECT_CLASS(klass);

    sp_object_class->child_added   = sp_objectgroup_child_added;
    sp_object_class->remove_child  = sp_objectgroup_remove_child;
    sp_object_class->order_changed = sp_objectgroup_order_changed;
    sp_object_class->write         = sp_objectgroup_write;
}

static void
sp_objectgroup_init(SPObjectGroup * /*objectgroup*/)
{
}

static void
sp_objectgroup_child_added(SPObject            *object,
                           Inkscape::XML::Node *child,
                           Inkscape::XML::Node *ref)
{
    if ((SP_OBJECT_CLASS(sp_objectgroup_parent_class))->child_added) {
        (* (SP_OBJECT_CLASS(sp_objectgroup_parent_class))->child_added)(object, child, ref);
    }

    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_objectgroup_remove_child(SPObject            *object,
                            Inkscape::XML::Node *child)
{
    if ((SP_OBJECT_CLASS(sp_objectgroup_parent_class))->remove_child) {
        (* (SP_OBJECT_CLASS(sp_objectgroup_parent_class))->remove_child)(object, child);
    }

    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static void
sp_objectgroup_order_changed(SPObject            *object,
                             Inkscape::XML::Node *child,
                             Inkscape::XML::Node *old_ref,
                             Inkscape::XML::Node *new_ref)
{
    if ((SP_OBJECT_CLASS(sp_objectgroup_parent_class))->order_changed) {
        (* (SP_OBJECT_CLASS(sp_objectgroup_parent_class))->order_changed)(object, child, old_ref, new_ref);
    }

    object->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

static Inkscape::XML::Node*
sp_objectgroup_write(SPObject                *object,
                     Inkscape::XML::Document *xml_doc,
                     Inkscape::XML::Node     *repr,
                     guint                    flags)
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
            repr->addChild(static_cast<Inkscape::XML::Node *>(l->data), NULL);
            Inkscape::GC::release(static_cast<Inkscape::XML::Node *>(l->data));
            l = g_slist_remove(l, l->data);
        }
    } else {
        for ( SPObject *child = object->firstChild() ; child ; child = child->getNext() ) {
            child->updateRepr(flags);
        }
    }

    if ((SP_OBJECT_CLASS(sp_objectgroup_parent_class))->write) {
        (SP_OBJECT_CLASS(sp_objectgroup_parent_class))->write(object, xml_doc, repr, flags);
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

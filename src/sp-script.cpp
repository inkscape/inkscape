/*
 * SVG <script> implementation
 *
 * Authors:
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2008 authors
 *
 * Released under GNU GPL version 2 or later, read the file 'COPYING' for more information
 */

#include "sp-script.h"
#include "attributes.h"
#include <cstring>
#include "document.h"

static void sp_script_release(SPObject *object);
static void sp_script_update(SPObject *object, SPCtx *ctx, guint flags);
static void sp_script_modified(SPObject *object, guint flags);
static void sp_script_set(SPObject *object, unsigned int key, gchar const *value);
static void sp_script_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static Inkscape::XML::Node *sp_script_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

G_DEFINE_TYPE(SPScript, sp_script, SP_TYPE_OBJECT);

static void sp_script_class_init(SPScriptClass *sc)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) sc;

    sp_object_class->build = sp_script_build;
    sp_object_class->release = sp_script_release;
    sp_object_class->update = sp_script_update;
    sp_object_class->modified = sp_script_modified;
    sp_object_class->write = sp_script_write;
    sp_object_class->set = sp_script_set;
}

static void sp_script_init(SPScript */*script*/)
{

}


/**
 * Reads the Inkscape::XML::Node, and initializes SPScript variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
static void
sp_script_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
    if (((SPObjectClass *) sp_script_parent_class)->build) {
        ((SPObjectClass *) sp_script_parent_class)->build(object, document, repr);
    }

    //Read values of key attributes from XML nodes into object.
    object->readAttr( "xlink:href" );

    document->addResource("script", object);
}

static void sp_script_release(SPObject *object)
{
    if (object->document) {
        // Unregister ourselves
        object->document->removeResource("script", object);
    }

    if (((SPObjectClass *) sp_script_parent_class)->release) {
        ((SPObjectClass *) sp_script_parent_class)->release(object);
    }
}

static void sp_script_update(SPObject */*object*/, SPCtx */*ctx*/, guint /*flags*/)
{
}

static void sp_script_modified(SPObject */*object*/, guint /*flags*/)
{
}

static void
sp_script_set(SPObject *object, unsigned int key, gchar const *value)
{
    SPScript *scr = SP_SCRIPT(object);

    switch (key) {
	case SP_ATTR_XLINK_HREF:
            if (scr->xlinkhref) g_free(scr->xlinkhref);
            scr->xlinkhref = g_strdup(value);
            object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
	default:
            if (((SPObjectClass *) sp_script_parent_class)->set)
                ((SPObjectClass *) sp_script_parent_class)->set(object, key, value);
            break;
    }
}

static Inkscape::XML::Node *sp_script_write(SPObject */*object*/, Inkscape::XML::Document */*xml_doc*/, Inkscape::XML::Node *repr, guint /*flags*/)
{
/*
TODO:
 code copied from sp-defs
 decide what to do here!

    if (flags & SP_OBJECT_WRITE_BUILD) {

        if (!repr) {
            repr = xml_doc->createElement("svg:script");
        }

        GSList *l = NULL;
        for ( SPObject *child = object->firstChild() ; child; child = child->getNext() ) {
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
        for ( SPObject *child = object->firstChild() ; child; child = child->getNext() ) {
            child->updateRepr(flags);
        }
    }

    if (((SPObjectClass *) (parent_class))->write) {
        (* ((SPObjectClass *) (parent_class))->write)(object, xml_doc, repr, flags);
    }
*/
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :

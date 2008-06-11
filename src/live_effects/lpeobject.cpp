#define INKSCAPE_LIVEPATHEFFECT_OBJECT_CPP

/*
 * Copyright (C) Johan Engelen 2007-2008 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/lpeobject.h"

#include "live_effects/effect.h"

#include "xml/repr.h"
#include "xml/node-event-vector.h"
#include "sp-object.h"
#include "attributes.h"
#include "document.h"
#include "document-private.h"

#include <glibmm/i18n.h>

//#define LIVEPATHEFFECT_VERBOSE

static void livepatheffect_class_init(LivePathEffectObjectClass *klass);
static void livepatheffect_init(LivePathEffectObject *stop);

static void livepatheffect_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
static void livepatheffect_release(SPObject *object);

static void livepatheffect_set(SPObject *object, unsigned key, gchar const *value);
static Inkscape::XML::Node *livepatheffect_write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

static void livepatheffect_on_repr_attr_changed (Inkscape::XML::Node * repr, const gchar *key, const gchar *oldval, const gchar *newval, bool is_interactive, void * data);

static SPObjectClass *livepatheffect_parent_class;
/**
 * Registers the LivePathEffect class with Gdk and returns its type number.
 */
GType
livepatheffect_get_type ()
{
    static GType livepatheffect_type = 0;

    if (!livepatheffect_type) {
        GTypeInfo livepatheffect_info = {
            sizeof (LivePathEffectObjectClass),
            NULL, NULL,
            (GClassInitFunc) livepatheffect_class_init,
            NULL, NULL,
            sizeof (LivePathEffectObject),
            16,
            (GInstanceInitFunc) livepatheffect_init,
            NULL,
        };
        livepatheffect_type = g_type_register_static (SP_TYPE_OBJECT, "LivePathEffectObject", &livepatheffect_info, (GTypeFlags)0);
    }
    return livepatheffect_type;
}

static Inkscape::XML::NodeEventVector const livepatheffect_repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    livepatheffect_on_repr_attr_changed,
    NULL, /* content_changed */
    NULL  /* order_changed */
};


/**
 * Callback to initialize livepatheffect vtable.
 */
static void
livepatheffect_class_init(LivePathEffectObjectClass *klass)
{
    SPObjectClass *sp_object_class = (SPObjectClass *) klass;

    livepatheffect_parent_class = (SPObjectClass *) g_type_class_ref(SP_TYPE_OBJECT);

    sp_object_class->build = livepatheffect_build;
    sp_object_class->release = livepatheffect_release;

    sp_object_class->set = livepatheffect_set;
    sp_object_class->write = livepatheffect_write;
}

/**
 * Callback to initialize livepatheffect object.
 */
static void
livepatheffect_init(LivePathEffectObject *lpeobj)
{
#ifdef LIVEPATHEFFECT_VERBOSE
    g_message("Init livepatheffectobject");
#endif
    lpeobj->effecttype = Inkscape::LivePathEffect::INVALID_LPE;
    lpeobj->lpe = NULL;

    lpeobj->effecttype_set = false;
}

/**
 * Virtual build: set livepatheffect attributes from its associated XML node.
 */
static void
livepatheffect_build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr)
{
#ifdef LIVEPATHEFFECT_VERBOSE
    g_message("Build livepatheffect");
#endif
    g_assert(object != NULL);
    g_assert(SP_IS_OBJECT(object));

    if (((SPObjectClass *) livepatheffect_parent_class)->build)
        (* ((SPObjectClass *) livepatheffect_parent_class)->build)(object, document, repr);

    sp_object_read_attr(object, "effect");

    if (repr) {
        repr->addListener (&livepatheffect_repr_events, object);
    }

    /* Register ourselves, is this necessary? */
//    sp_document_add_resource(document, "path-effect", object);
}

/**
 * Virtual release of livepatheffect members before destruction.
 */
static void
livepatheffect_release(SPObject *object)
{
#ifdef LIVEPATHEFFECT_VERBOSE
    g_print("Releasing livepatheffect");
#endif

    LivePathEffectObject *lpeobj = LIVEPATHEFFECT(object);

    SP_OBJECT_REPR(object)->removeListenerByData(object);


/*
    if (SP_OBJECT_DOCUMENT(object)) {
        // Unregister ourselves
        sp_document_remove_resource(SP_OBJECT_DOCUMENT(object), "livepatheffect", SP_OBJECT(object));
    }

    if (gradient->ref) {
        gradient->modified_connection.disconnect();
        gradient->ref->detach();
        delete gradient->ref;
        gradient->ref = NULL;
    }

    gradient->modified_connection.~connection();

*/

    if (lpeobj->lpe) {
        delete lpeobj->lpe;
        lpeobj->lpe = NULL;
    }
    lpeobj->effecttype = Inkscape::LivePathEffect::INVALID_LPE;

    if (((SPObjectClass *) livepatheffect_parent_class)->release)
        ((SPObjectClass *) livepatheffect_parent_class)->release(object);
}

/**
 * Virtual set: set attribute to value.
 */
static void
livepatheffect_set(SPObject *object, unsigned key, gchar const *value)
{
    LivePathEffectObject *lpeobj = LIVEPATHEFFECT(object);
#ifdef LIVEPATHEFFECT_VERBOSE
    g_print("Set livepatheffect");
#endif
    switch (key) {
        case SP_PROP_PATH_EFFECT:
            if (lpeobj->lpe) {
                delete lpeobj->lpe;
                lpeobj->lpe = NULL;
            }

            if ( value && Inkscape::LivePathEffect::LPETypeConverter.is_valid_key(value) ) {
                lpeobj->effecttype = Inkscape::LivePathEffect::LPETypeConverter.get_id_from_key(value);
                lpeobj->lpe = Inkscape::LivePathEffect::Effect::New(lpeobj->effecttype, lpeobj);
                lpeobj->effecttype_set = true;
            } else {
                lpeobj->effecttype = Inkscape::LivePathEffect::INVALID_LPE;
                lpeobj->effecttype_set = false;
            }
            object->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
    }

    if (((SPObjectClass *) livepatheffect_parent_class)->set) {
        ((SPObjectClass *) livepatheffect_parent_class)->set(object, key, value);
    }
}

/**
 * Virtual write: write object attributes to repr.
 */
static Inkscape::XML::Node *
livepatheffect_write(SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
#ifdef LIVEPATHEFFECT_VERBOSE
    g_print("Write livepatheffect");
#endif

    LivePathEffectObject *lpeobj = LIVEPATHEFFECT(object);

    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("inkscape:path-effect");
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || lpeobj->effecttype_set)
        repr->setAttribute("effect", Inkscape::LivePathEffect::LPETypeConverter.get_key(lpeobj->effecttype).c_str());

//    lpeobj->lpe->write(repr); something like this.

    lpeobj->lpe->writeParamsToSVG();

    if (((SPObjectClass *) livepatheffect_parent_class)->write)
        (* ((SPObjectClass *) livepatheffect_parent_class)->write)(object, xml_doc, repr, flags);

    return repr;
}

static void
livepatheffect_on_repr_attr_changed ( Inkscape::XML::Node * /*repr*/,
                                      const gchar *key,
                                      const gchar */*oldval*/,
                                      const gchar *newval,
                                      bool /*is_interactive*/,
                                      void * data )
{
#ifdef LIVEPATHEFFECT_VERBOSE
    g_print("livepatheffect_on_repr_attr_changed");
#endif

    if (!data)
        return;

    LivePathEffectObject *lpeobj = (LivePathEffectObject*) data;
    if (!lpeobj->lpe)
        return;

    lpeobj->lpe->setParameter(key, newval);

    lpeobj->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * If this has other users, create a new private duplicate and return it
 * returns 'this' when no forking was necessary (and therefore no duplicate was made)
 */
LivePathEffectObject *
LivePathEffectObject::fork_private_if_necessary(unsigned int nr_of_allowed_users)
{
    if (SP_OBJECT_HREFCOUNT(this) > nr_of_allowed_users) {
        SPDocument *doc = SP_OBJECT_DOCUMENT(this);
        Inkscape::XML::Document *xml_doc = sp_document_repr_doc(doc);

        Inkscape::XML::Node *repr = SP_OBJECT_REPR (this)->duplicate(xml_doc);
        SP_OBJECT_REPR (SP_DOCUMENT_DEFS (doc))->addChild(repr, NULL);
        LivePathEffectObject *lpeobj_new = (LivePathEffectObject *) doc->getObjectByRepr(repr);
        Inkscape::GC::release(repr);
        return lpeobj_new;
    }
    return this;
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
